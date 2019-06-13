#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    initCamera();
    
    // shaders
    outlineShader.load("shadersGL3/outline");
    fadeOutShader.load("shadersGL3/fadeout");
    
    
    depthImage.setUseTexture(false);
    depthImage.allocate(cameraWidth, cameraHeight);
    
    //scaledDepthImage.setUseTexture(false);
    scaledDepthImage.allocate(cameraWidth, cameraHeight);
    
    lastSlice.allocate(cameraWidth, cameraHeight);
    tempImage1.allocate(cameraWidth, cameraHeight);
    
    resultFbo.allocate(cameraWidth, cameraHeight, GL_RGBA);
    
    // clear result
    
    resultFbo.begin();
    ofClear(0);
    resultFbo.end();
    
    // GUI
    gui.setup();
    gui.add(minDepthThreshold.setup("min depth", 1, 0.5, 8));
    gui.add(maxDepthThreshold.setup("max depth", 3, 1, 8));
    gui.add(depthStep.setup("depth step", 0.04, 0.01, 0.5));
    gui.add(minPolygonSize.setup("min polygon size", 10, 0, 50));
    gui.add(polylineTolerance.setup("polygon tolerance", 0.3, 0, 100));
    gui.loadFromFile("settings.xml");
}

bool ofApp::initCamera() {
    ofLogNotice("Looking for RealSense");
    if (realSense.setup()) {
        ofLogNotice("RealSense connected");
        cameraFound = true;
        realSense.start();
        ofLogNotice("RealSense started");
        return true;
    }
    else {
        ofLogNotice("RealSense not found");
        return false;
    }
}

void ofApp::updateImages() {
    if (realSense.hasNewFrames()) {
        depthImage.getShortPixelsRef() = realSense.getDepthPixels();
        
        // scale depth of image
        
        float minDepthPoints = minDepthThreshold / realSense.getDepthScale();
        float maxDepthPoints = maxDepthThreshold / realSense.getDepthScale();
        
        float newMinPoints = minDepthPoints * 65535. / (maxDepthPoints - minDepthPoints);
        float newMaxPoints = minDepthPoints + 65535. * 65535. / (maxDepthPoints - minDepthPoints);
        
        depthImage.convertToRange(-newMinPoints, newMaxPoints);
        
        // transform to greyscale
        scaledDepthImage = depthImage;
    }
}

ofxCvGrayscaleImage& ofApp::makeSlice(float minDepth, float maxDepth) {
    float maxDepthPoints = 255 * (maxDepth - minDepthThreshold) / (maxDepthThreshold - minDepthThreshold);
    float minDepthPoints = 255 * (minDepth - minDepthThreshold) / (maxDepthThreshold - minDepthThreshold);
    
    lastSlice = scaledDepthImage;
    lastSlice.threshold(minDepthPoints);
    if (subtractPrevious) {
        tempImage1 = scaledDepthImage;
        tempImage1.threshold(maxDepthPoints);
        lastSlice -= tempImage1;
    }
    
    return lastSlice;
}

ofPath ofApp::calculatePolygon(ofxCvGrayscaleImage& slice) {
    contourFinder.findContours(slice, minPolygonSize, cameraWidth * cameraHeight, 1000, true);
    
    ofPath result;
    for (ofxCvBlob b : contourFinder.blobs) {
        result.moveTo(b.pts[0]);
        for (int i = 1; i < b.nPts; i++) result.lineTo(b.pts[i]);
        result.close();
    }
    result.simplify(polylineTolerance);
    return result;
}

void ofApp::drawLevel(float minDepth, float maxDepth, float position) {
    // draw lines at depth level = depth
    resultFbo.begin();
    
    //ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    if (simplify) {
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofSetColor(255);
        ofPath path = calculatePolygon(makeSlice(minDepth, maxDepth));
        path.setColor(ofColor(255 * position, 255 * (1 - position), 0, 200));
        //path.setStrokeWidth(1);
        //path.setFilled(false);
        path.draw();
    }
    else {
        cout << position << "\n";
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(255 * position, 255 * (1 - position), 0);
        makeSlice(minDepth, maxDepth).draw(0, 0);
    }
    
    resultFbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    realSense.update();
    updateImages();
    
    // clean image
    resultFbo.begin();
    ofClear(0);
    resultFbo.end();
    
    // iterate through slices
    float currentPosition = minDepthThreshold;
    float stepNumber = floor((maxDepthThreshold - minDepthThreshold) / depthStep);
    int i = 0;
    while (currentPosition < maxDepthThreshold) {
        // draw lines at currentPosition
        drawLevel(currentPosition, currentPosition + depthStep, i++ / stepNumber);
        currentPosition += depthStep;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0);
    
    if (!cameraFound) {
        ofSetColor(255);
        ofDrawBitmapString("Camera not found", 10, 20);
        return;
    }
    
    scaledDepthImage.draw(0, 0);
    //resultFbo.draw(0, 0, cameraWidth, cameraHeight);
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        string filename = "captured/" + ofGetTimestampString();
        
        ofDirectory dir(filename);
        dir.create();
        
        ofPixels pixels;
        pixels.allocate(cameraWidth, cameraHeight, GL_RGB);
        
        // save scaled depth
        ofSaveImage(scaledDepthImage.getPixels(), filename + "_depth_scaled.png");
        
        // save result
        resultFbo.readToPixels(pixels);
        ofSaveImage(pixels, filename + "_result.png");
        
        // save individual slices
        float currentPosition = minDepthThreshold;
        int i = 0;
        while (currentPosition < maxDepthThreshold) {
            // draw lines at currentPosition
            currentPosition += depthStep;
            
            char buff[100];
            snprintf(buff, sizeof(buff), "%03d", i);
            string buffAsStdStr = buff;
            
            ofSaveImage(makeSlice(currentPosition, currentPosition + depthStep).getPixels(), filename + "/" + buffAsStdStr + ".png");
            
            i++;
        }
    }
}

void ofApp::exit(){
    if (cameraFound) {
        realSense.stop();
    }
    
    gui.saveToFile("settings.xml");
}
