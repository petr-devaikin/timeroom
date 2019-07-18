//
//  cubinator.cpp
//  timeroom
//
//  Created by Petr Devaikin on 17.07.19.
//

#include "cubinator.hpp"
#include "rgbdFrame.hpp"

cubinator::cubinator(ofxRealSense2 * realSense) : cameraFilter(realSense) {
    // GUI
    initGui();
    
    // generate cubes
    generateBoxes();
    
    // Init result fbo
    result.allocate(realSense->getWidth(), realSense->getHeight(), GL_RGBA);
    result.begin();
    ofClear(100);
    result.end();
    
    // Setup Scene
    setupScene();
    
}

cubinator::~cubinator() {
    deleteBoxes();
    
    pixelSize.removeListener(this, &cubinator::regenerateCubes);
    cameraMarginHor.removeListener(this, &cubinator::regenerateCubes);
    cameraMarginVert.removeListener(this, &cubinator::regenerateCubes);
    
    gui.saveToFile("cubinator.xml");
}

void cubinator::setupScene() {
    camera.enableOrtho();
    
    directionalLight.setDirectional();
    directionalLight.setDiffuseColor(ofColor(255, 255, 255));
    directionalLight.rotateDeg(160, glm::vec3(0, 1, 0));
    directionalLight.rotateDeg(-15, glm::vec3(1, 0, 0));
}

void cubinator::updateScene() {
    camera.setPosition(cameraPosition->x, cameraPosition->y, cameraPosition->z);
    camera.setOrientation(glm::vec3(0, 0, 0));
    camera.rotateDeg(cameraRotation->x, 1, 0, 0);
    camera.rotateDeg(cameraRotation->y, 0, 1, 0);
}

void cubinator::initGui() {
    gui.add(minDistance.setup("min distance", 1, 0, 10));
    gui.add(maxDistance.setup("max distance", 8, 0, 10));
    gui.add(cameraMarginHor.setup("camera margin hor", 0, 0, 500));
    gui.add(cameraMarginVert.setup("camera margin vert", 0, 0, 500));
    gui.add(pixelSize.setup("pixel size", 40, 1, 500));
    gui.add(cubeSize.setup("cube size", 20, 1, 100));
    gui.add(cubePerMeter.setup("cube per depth meter", 10, 0, 100));
    
    gui.add(cameraPosition.setup("camera position", ofVec3f(0, 0, 0), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));
    gui.add(cameraRotation.setup("camera rotation", ofVec2f(0, 0), ofVec2f(-180, -180), ofVec2f(180, 180)));
    gui.loadFromFile("cubinator.xml");
    
    cameraMarginHor.addListener(this, &cubinator::regenerateCubes);
    cameraMarginVert.addListener(this, &cubinator::regenerateCubes);
    pixelSize.addListener(this, &cubinator::regenerateCubes);
}

void cubinator::update() {
    if (realSense->hasNewFrames()) {
        rgbdFrame newFrame = rgbdFrame(realSense->getColorPixels(), realSense->getDepthPixels(), minDistance / realSense->getDepthScale(), maxDistance / realSense->getDepthScale());
        updateBoxPosition(newFrame.depthPixels);
        updateBoxColor(newFrame.videoPixels);
    }
    
    updateScene();
    
    result.begin();
    ofEnableDepthTest();
    
    ofSetSmoothLighting(true);
    ofEnableLighting();
    
    ofClear(0);
    //ofSetColor(20, 30, 40);
    //ofDrawRectangle(0, 0, realSense->getWidth(), realSense->getHeight());
    ofSetColor(255);
    
    camera.begin();
    directionalLight.enable();

    for (int x = 0; x < columnNumber; x++) {
        for (int y = 0; y < rowNumber; y++) {
            boxes[x][y].material.begin();
            boxes[x][y].primitive.draw();
            boxes[x][y].material.end();
        }
    }
    
    directionalLight.disable();
    camera.end();
    
    ofDisableDepthTest();
    result.end();
}

void cubinator::draw() {
    result.draw(0, 0);
}

void cubinator::generateBoxes() {
    columnNumber = floor((realSense->getWidth() - 2 * cameraMarginHor) / pixelSize);
    rowNumber = floor((realSense->getHeight() - 2 * cameraMarginVert) / pixelSize);
    
    if (columnNumber > 0 && rowNumber > 0) {
        boxes = new coloredBox * [columnNumber];
        for (int x = 0; x < columnNumber; x++) {
            boxes[x] = new coloredBox[rowNumber];
        }
    }
}

void cubinator::deleteBoxes() {
    if (columnNumber > 0 && rowNumber > 0) {
        for (int i = 0; i < columnNumber; i++) {
            delete [] boxes[i];
        }
    
        delete [] boxes;
    }
}

void cubinator::regenerateCubes(int &p) {
    deleteBoxes();
    generateBoxes();
}

void cubinator::rescaleImage(ofImage &img) {
    ofRectangle cropRectangle = ofRectangle(0, 0, pixelSize * columnNumber, pixelSize * rowNumber);
    cropRectangle.alignTo(glm::vec2(img.getWidth() / 2, img.getHeight() / 2));
    img.crop(cropRectangle.x, cropRectangle.y, cropRectangle.width, cropRectangle.height);
    img.resize(columnNumber, rowNumber);
}

void cubinator::updateBoxColor(ofPixels &colorPixels) {
    ofImage img;
    img.setFromPixels(colorPixels);
    rescaleImage(img);
    for (int x = 0; x < columnNumber; x++) {
        for (int y = 0; y < rowNumber; y++) {
            boxes[x][y].material.setDiffuseColor(img.getColor(x, y));
        }
    }
}

void cubinator::updateBoxPosition(ofPixels &depthPixels) {
    ofImage img;
    img.setFromPixels(depthPixels);
    rescaleImage(img);
    
    for (int x = 0; x < columnNumber; x++) {
        for (int y = 0; y < rowNumber; y++) {
            float depthMeters = img.getPixels()[x + y * columnNumber] * (maxDistance - minDistance) / 255.0;
            float depth = floor(depthMeters * cubePerMeter) * cubeSize;
            boxes[x][y].primitive.setPosition(-x * cubeSize + cubeSize / 2, -y * cubeSize + cubeSize / 2, -depth + cubeSize / 2);
            boxes[x][y].primitive.set(cubeSize, cubeSize, cubeSize);
        }
    }
}
