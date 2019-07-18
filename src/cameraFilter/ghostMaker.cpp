//
//  ghostMaker.cpp
//  timeroom
//
//  Created by Petr Devaikin on 05.07.19.
//

#include "ghostMaker.hpp"

ghostMaker::ghostMaker(ofxRealSense2 * realSense) : cameraFilter(realSense) {
    // init buffer
    float cameraWidth = realSense->getWidth();
    float cameraHeight = realSense->getHeight();
    
    buffer = new videoBuffer(cameraWidth, cameraHeight, maxGhostLifetime * 2.1);
    
    // load shaders
    maskShader.load("shadersGL3/mask");
    maxShader.load("shadersGL3/max");
    
    // init FBOs
    resultFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    resultDepthFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    
    tempFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    tempDepthFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    
    // init timer
    timer = ofGetElapsedTimef();
    
    // GUI
    initGui();
}

ghostMaker::~ghostMaker() {
    delete buffer;
    gui.saveToFile("ghostMaker.xml");
}

void ghostMaker::initGui() {
    gui.add(minDistance.setup("min distance", 1, 0, 10));
    gui.add(maxDistance.setup("max distance", 8, 0, 10));
    gui.add(maxGhostLifetime.setup("ghost lifetime", 4, 0.1, 20));
    gui.add(ghostGenerationInterval.setup("ghost interval", 1, 0.1, 2));
    gui.loadFromFile("ghostMaker.xml");
}

void ghostMaker::updateGhosts() {
    auto p = ghostTimestamps.begin();
    while (p != ghostTimestamps.end()) {
        *p -= timeDelta;
        p++;
    }
}

void ghostMaker::removeOldGhosts() {
    while (ghostTimestamps.size() && timer - ghostTimestamps.front() > maxGhostLifetime * 2)
        ghostTimestamps.pop_front();
}

void ghostMaker::addNewGhost() {
    while (lastGhostGeneratedTimestamp + ghostGenerationInterval <= timer) {
        lastGhostGeneratedTimestamp += ghostGenerationInterval;
        if (timer - lastGhostGeneratedTimestamp <= 2 * maxGhostLifetime)
            ghostTimestamps.push_back(lastGhostGeneratedTimestamp);
    }
}

void ghostMaker::mergeImages() {
    // update current view
    ofTexture newLayerTexture;
    ofTexture newLayerDepthTexture;
    
    rgbdFrame * newFrame = buffer->getLastFrame();
    newLayerTexture.loadData(newFrame->videoPixels);
    newLayerDepthTexture.loadData(newFrame->depthPixels);
    
    // init fbo's with current pictures
    resultFbo.begin();
    ofSetColor(255, 255, 255);
    newLayerTexture.draw(0, 0);
    resultFbo.end();
    resultDepthFbo.begin();
    newLayerDepthTexture.draw(0, 0);
    resultDepthFbo.end();
    
    // past pictures
    for (float g : ghostTimestamps) {
        // update temp Fbo's
        tempFbo.begin();
        resultFbo.draw(0, 0);
        tempFbo.end();
        tempDepthFbo.begin();
        resultDepthFbo.draw(0, 0);
        tempDepthFbo.end();
        
        // get past pictures
        rgbdFrame * pastFrame = buffer->getFrame(g);
        newLayerTexture.loadData(pastFrame->videoPixels);
        newLayerDepthTexture.loadData(pastFrame->depthPixels);
        
        // update result
        resultFbo.begin();
        maskShader.begin();
        maskShader.setUniformTexture("tex1", tempFbo.getTexture(), 2);
        maskShader.setUniformTexture("tex1Depth", tempDepthFbo.getTexture(), 3);
        maskShader.setUniformTexture("tex0Depth", newLayerDepthTexture, 1);
        newLayerTexture.draw(0, 0);
        maskShader.end();
        resultFbo.end();
        
        // update resulting depth
        resultDepthFbo.begin();
        maxShader.begin();
        maxShader.setUniformTexture("tex1", tempDepthFbo.getTexture(), 1);
        newLayerDepthTexture.draw(0, 0);
        maxShader.end();
        resultDepthFbo.end();
    }
}

void ghostMaker::update() {
    float currentTime = ofGetElapsedTimef();
    timeDelta = currentTime - timer;
    timer = currentTime;
    
    // add new frames
    if (realSense->hasNewFrames()) {
        rgbdFrame * newFrame = new rgbdFrame(realSense->getColorPixels(), realSense->getDepthPixels(), minDistance / realSense->getDepthScale(), maxDistance / realSense->getDepthScale());
        buffer->addFrame(newFrame);
    }
    
    if (ghostTimestamps.size()) {
        // update current ghosts
        updateGhosts();
        removeOldGhosts();
    }
    
    addNewGhost();
    
    mergeImages();
}

void ghostMaker::draw() {
    resultFbo.draw(0, 0);
}
