//
//  cubinator.cpp
//  timeroom
//
//  Created by Petr Devaikin on 17.07.19.
//

#include "cubinator.hpp"
#include "rgbdFrame.hpp"

cubinator::cubinator(ofxRealSense2 * realSense, int pixelSize) : cameraFilter(realSense) {
    this->pixelSize = pixelSize;
    
    rowNumber = floor(realSense->getWidth() / pixelSize);
    columnNumber = floor(realSense->getHeight() / pixelSize);
    
    boxes = new ofBoxPrimitive * [columnNumber];
    for (int x = 0; x < columnNumber; x++) {
        boxes[x] = new ofBoxPrimitive[rowNumber];
        for (int y = 0; y < rowNumber; y++) {
            boxes[x][y].set(pixelSize, pixelSize, pixelSize);
            boxes[x][y].setPosition(x * pixelSize, y * pixelSize, 0);
        }
    }
}

cubinator::~cubinator() {
    for (int i = 0; i < columnNumber; i++) {
        delete [] boxes[i];
    }
    delete [] boxes;
    
    gui.saveToFile("ghostMaker.xml");
}

void cubinator::initGui() {
    cameraFilter::initGui();
    
    gui.add(minDistance.setup("min distance", 1, 0, 10));
    gui.add(maxDistance.setup("max distance", 8, 0, 10));
    gui.loadFromFile("cubinator.xml");
}

void cubinator::update() {
    if (realSense->hasNewFrames()) {
        rgbdFrame newFrame = rgbdFrame(realSense->getColorPixels(), realSense->getDepthPixels(), minDistance / realSense->getDepthScale(), maxDistance / realSense->getDepthScale());
        updateBoxPosition(newFrame.depthPixels);
        updateBoxColor(newFrame.videoPixels);
    }
}

void cubinator::draw() {
    for (int x = 0; x < columnNumber; x++) {
        for (int y = 0; y < rowNumber; y++) {
            boxes[x][y] ...
        }
    }
}
