#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if (initCamera()) {
        frames = pipe.wait_for_frames();
    }
    
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
    cout << "Looking for RealSense\n";
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH, cameraWidth, cameraHeight);
    cfg.enable_stream(RS2_STREAM_COLOR, cameraWidth, cameraHeight);
    rs2::context ctx;
    auto device_list = ctx.query_devices();
    
    if (device_list.size() > 0) {
        rs2::pipeline_profile profile = pipe.start(cfg);
        auto depth_sensor = profile.get_device().first<rs2::depth_sensor>();
        
        // depth sensor settings
        // !!!! set more precise
        depth_sensor.set_option(RS2_OPTION_VISUAL_PRESET, RS2_RS400_VISUAL_PRESET_DEFAULT);
        //depth_sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 0.f);
        //depth_sensor.set_option(RS2_OPTION_EXPOSURE, 30000);
        
        cameraDepthScale = depth_sensor.get_depth_scale();
        
        cout << "RealSense found!\n";
        
        cameraFound = true;
        return true;
    }
    else {
        cout << "Camera not found!\n";
        
        cameraFound = false;
        return false;
    }
}

void ofApp::updateFrames() {
    rs2::frameset newFrames;
    if (pipe.poll_for_frames(&newFrames)) {
        frames = newFrames;
        
        frames = temp_filter.process(frames);
        frames = hole_filter.process(frames);
        
        // !!! need to process all the frames in frameset, not only one
        
        // process last frame
        rs2::depth_frame depthFrame = frames.get_depth_frame();
        memcpy(depthImage.getShortPixelsRef().getData(), depthFrame.get_data(), cameraWidth * cameraHeight * 2);
        
        // calc range
        //depthImage.setROI(cameraWidth / 8, cameraHeight / 8, cameraWidth * 6 / 8, cameraHeight * 6 / 8);
        
        
        //depthImage.resetROI();
        //
        
        // scale depth of image
        
        float minDepthPoints = minDepthThreshold / cameraDepthScale;
        float maxDepthPoints = maxDepthThreshold / cameraDepthScale;
        
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
    tempImage1 = scaledDepthImage;
    tempImage1.threshold(maxDepthPoints);
    
    lastSlice -= tempImage1;
    
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
        ofSetColor(255);
        ofPath path = calculatePolygon(makeSlice(minDepth, maxDepth));
        path.setColor(ofColor(255 * position, 255 * (1 - position), 0, 200));
        //path.setStrokeWidth(1);
        //path.setFilled(false);
        path.draw();
    }
    else {
        makeSlice(minDepth, maxDepth).draw(0, 0);
    }
    
    resultFbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    updateFrames();
    
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
    
    resultFbo.draw(0, 0, cameraWidth, cameraHeight);
    
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
        pipe.stop();
    }
    
    gui.saveToFile("settings.xml");
}
