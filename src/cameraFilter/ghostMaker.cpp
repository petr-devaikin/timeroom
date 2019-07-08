//
//  ghostMaker.cpp
//  timeroom
//
//  Created by Petr Devaikin on 05.07.19.
//

#include "ghostMaker.hpp"

ghostMaker::ghostMaker(ofxRealSense2 * realSense) : cameraFilter(realSense) {
    // GUI
    initGui();
    
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
}

ghostMaker::~ghostMaker() {
    delete buffer;
    gui.saveToFile("ghostMaker.xml");
}

void ghostMaker::initGui() {
    gui.setup();
    gui.add(maxGhostLifetime.setup("ghost lifetime", 4, 0.1, 20));
    gui.add(ghostGenerationInterval.setup("ghost interval", 1, 0.1, 2));
    gui.loadFromFile("ghostMaker.xml");
}
