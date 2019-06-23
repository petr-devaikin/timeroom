#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    initCamera();
    
    resultFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    cout << "2\n";
    resultDepthFbo.allocate(cameraWidth, cameraHeight, GL_LUMINANCE);
    cout << "3\n";
    
    currentImage.allocate(cameraWidth, cameraHeight, GL_LUMINANCE);
    currentDepthImage.allocate(cameraWidth, cameraHeight, GL_LUMINANCE);
    
    maskShader.load("shadersGL3/mask");
    maxShader.load("shadersGL3/max");
    
    timer = 0;
    
    initGui();
}

void ofApp::initGui() {
    showGui = true;
    
    gui.setup();
    gui.add(maxGhostLifeTime.setup("ghost lifetime", 4, 0.1, 20));
    gui.add(ghostGenerationInterval.setup("ghost interval", 1, 0.1, 2));
    
    gui.add(minDistance.setup("min distance", 1, 0, 10));
    gui.add(maxDistance.setup("max distance", 8, 0, 10));
    gui.add(resultScale.setup("scale", 1, 0.4, 3));
    gui.add(resultShift.setup("shift", ofVec2f(0, 0), ofVec2f(-ofGetWindowWidth() / 2, -ofGetWindowHeight() / 2), ofVec2f(ofGetWindowWidth() / 2, ofGetWindowHeight() / 2)));
    
    gui.loadFromFile("settings.xml");
}

bool ofApp::initCamera() {
    ofLogNotice("Looking for RealSense");
    
    if (realSense.setup()) {
        ofLogNotice("RealSense found");
        cameraFound = true;
        
        realSense.start();
        
        return true;
    }
    else {
        ofLogError("RealSense not found");
        cameraFound = false;
        
        return false;
    }
}

bool ofApp::getNewFrames() {
    float currentTime = ofGetElapsedTimef();
    
    if (realSense.hasNewFrames()) {rgbdFrame * newFrame = new rgbdFrame(currentTime, realSense.getColorPixels(), realSense.getDepthPixels(), minDistance / realSense.getDepthScale(), maxDistance / realSense.getDepthScale());
        
        buffer.addFrame(newFrame);  // add to the buffer
        
<<<<<<< HEAD
        return true;
=======
        // update current view
        currentImage.loadData(newFrame->irPixels);
        currentDepthImage.loadData(newFrame->depthPixels);
    }
    
    ofTexture resultTexture;
    ofTexture resultDepthTexture;
    ofTexture newLayerTexture;
    ofTexture newLayerDepthTexture;
    
    resultTexture = currentImage;
    resultDepthTexture = currentDepthImage;
    
    // past pictures
    for (float g : ghostTimestamps) {
        rgbdFrame * pastFrame = buffer.getFrame(g);
        newLayerTexture.loadData(pastFrame->irPixels);
        newLayerDepthTexture.loadData(pastFrame->depthPixels);
        
        resultFbo.begin();
        maskShader.begin();
        maskShader.setUniformTexture("tex1", resultTexture, 2);
        maskShader.setUniformTexture("tex1Depth", resultDepthTexture, 3);
        maskShader.setUniformTexture("tex0Depth", newLayerDepthTexture, 1);
        newLayerTexture.draw(0, 0);
        maskShader.end();
        resultFbo.end();
        
        resultDepthFbo.begin();
        maxShader.begin();
        maskShader.setUniformTexture("tex1", resultDepthTexture, 1);
        newLayerTexture.draw(0, 0);
        maxShader.end();
        resultDepthFbo.end();
        
        resultTexture = resultFbo.getTexture();
        resultDepthTexture = resultDepthFbo.getTexture();
        
        break;
>>>>>>> parent of 6a71f49... working!
    }
    else
        return false;
}

void ofApp::removeOldGhosts() {
    while (ghostTimestamps.size() && timer - ghostTimestamps.front() > maxGhostLifeTime * 2)
        ghostTimestamps.pop_front();
}

void ofApp::addNewGhost() {
    while (lastGhostGeneratedTimestamp + ghostGenerationInterval <= timer) {
        lastGhostGeneratedTimestamp += ghostGenerationInterval;
        if (timer - lastGhostGeneratedTimestamp <= 2 * maxGhostLifeTime)
            ghostTimestamps.push_back(lastGhostGeneratedTimestamp);
    }
}

void ofApp::updateGhosts() {
    if (ghostTimestamps.size()) {
        auto p = ghostTimestamps.begin();
        while (p != ghostTimestamps.end()) {
            *p -= timeDelta;
            p++;
        }
        
        removeOldGhosts();
    }
    
    addNewGhost();
}

void ofApp::mergeImages() {
    // update current view
    ofTexture newLayerTexture;
    ofTexture newLayerDepthTexture;
    
    rgbdFrame * newFrame = buffer.getFrame(timer);
    
    newLayerTexture.loadData(newFrame->colorPixels);
    newLayerDepthTexture.loadData(newFrame->depthPixels);
    
    // init fbo's with current pictures
    resultFbo.begin();
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
        rgbdFrame * pastFrame = buffer.getFrame(g);
        newLayerTexture.loadData(pastFrame->colorPixels);
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

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    realSense.update();
    
    float newTimer = ofGetElapsedTimef();
    timeDelta = newTimer - timer;
    timer = newTimer;
    
    updateGhosts();
    if (getNewFrames())
        mergeImages();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0);
    
    if (!cameraFound) {
        ofSetColor(255);
        ofDrawBitmapString("Camera not found", 10, 20);
        return;
    }
    
    // latest frame
<<<<<<< HEAD
    float dx = ofGetWindowWidth() / 2 + resultShift->x - cameraWidth * resultScale / 2;
    float dy = ofGetWindowHeight() / 2 + resultShift->y - cameraHeight * resultScale / 2;
    float scaledWidth = cameraWidth * resultScale;
    float scaledHeight = cameraHeight * resultScale;
=======
    resultFbo.draw(0, 0);
>>>>>>> parent of 6a71f49... working!
    
    resultFbo.draw(dx, dy, scaledWidth, scaledHeight);
    
    if (showGui) gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'g') showGui = !showGui;
}

void ofApp::exit(){
    if (cameraFound) {
        realSense.stop();
    }
    gui.saveToFile("settings.xml");
}
