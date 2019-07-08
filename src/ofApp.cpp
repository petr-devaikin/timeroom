#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    initCamera();
    initGui();
    
    filter = new ghostMaker(&realSense);
}

void ofApp::initGui() {
    showGui = true;
    
    gui.setup();
    
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
        realSense.start();
        return true;
    }
    else {
        return false;
    }
}

bool ofApp::getNewFrames() {
    float currentTime = ofGetElapsedTimef();
    
    if (realSense.hasNewFrames()) {
        rgbdFrame * newFrame = new rgbdFrame(currentTime, realSense.getColorPixels(), realSense.getDepthPixels(), minDistance / realSense.getDepthScale(), maxDistance / realSense.getDepthScale());
        
        
        buffer.addFrame(newFrame);  // add to the buffer
        
        return true;
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
    
    rgbdFrame * newFrame = buffer.getLastFrame();
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
        rgbdFrame * pastFrame = buffer.getFrame(g);
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

//--------------------------------------------------------------
void ofApp::update(){
    if (!realSense.isConnected()) return;
    
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
    
    if (!realSense.isConnected()) {
        ofSetColor(255);
        ofDrawBitmapString("Camera not found", 10, 20);
        return;
    }
    
    // draw result
    ofPushMatrix();
    
    ofTranslate(ofGetWindowWidth() / 2 + resultShift->x, ofGetWindowHeight() / 2 + resultShift->y);
    ofScale(resultScale);
    ofTranslate(-cameraWidth / 2, -cameraHeight / 2);
    
    resultFbo.draw(0, 0, cameraWidth, cameraHeight);
    
    ofPopMatrix();
    
    if (showGui) gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'g') showGui = !showGui;
}

void ofApp::exit(){
    if (realSense.isConnected()) {
        realSense.stop();
    }
    gui.saveToFile("settings.xml");
}
