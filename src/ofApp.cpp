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

//--------------------------------------------------------------
void ofApp::update(){
    if (!realSense.isConnected()) return;
    
    realSense.update();
    filter->update();
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
    
    filter->draw();
    
    ofPopMatrix();
    
    if (showGui) {
        gui.draw();
        filter->drawGui(200, 0);
    }
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
