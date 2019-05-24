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
    
    tempImage1.allocate(cameraWidth, cameraHeight);
    tempImage2.allocate(cameraWidth, cameraHeight);
    
    resultFbo.allocate(cameraWidth, cameraHeight, GL_RGBA);
    sliceFbo.allocate(cameraWidth, cameraHeight, GL_RGBA);
    
    // clear result
    
    resultFbo.begin();
    ofClear(0);
    resultFbo.end();
    
    // GUI
    gui.setup();
    gui.add(minDepthThreshold.setup("min depth", 1, 0.5, 8));
    gui.add(maxDepthThreshold.setup("max depth", 3, 1, 8));
    gui.add(depthStep.setup("depth step", 0.04, 0.01, 0.2));
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
        depthImage.setROI(cameraWidth / 8, cameraHeight / 8, cameraWidth * 6 / 8, cameraHeight * 6 / 8);
        
        
        depthImage.resetROI();
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

void ofApp::makeSlice(float minDepth, float maxDepth) {
    float maxDepthPoints = 255 * (maxDepth - minDepthThreshold) / (maxDepthThreshold - minDepthThreshold);
    float minDepthPoints = 255 * (minDepth - minDepthThreshold) / (maxDepthThreshold - minDepthThreshold);
    
    tempImage1 = scaledDepthImage;
    tempImage1.threshold(minDepthPoints);
    tempImage2 = scaledDepthImage;
    tempImage2.threshold(maxDepthPoints);
    
    tempImage1 -= tempImage2;
    
    sliceFbo.begin();;
    outlineShader.begin();
    tempImage1.draw(0, 0);
    outlineShader.end();
    sliceFbo.end();
}

void ofApp::drawLevel(float minDepth, float maxDepth) {
    // draw lines at depth level = depth
    makeSlice(minDepth, maxDepth);
    
    resultFbo.begin();
    
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    
    sliceFbo.draw(0, 0);
    
    resultFbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    updateFrames();
    
    // clean image
    resultFbo.begin();
    ofClear(255, 255, 255, 255);
    resultFbo.end();
    
    // update currentPosition;
    float currentPosition = minDepthThreshold;
    while (currentPosition < maxDepthThreshold) {
        // draw lines at currentPosition
        drawLevel(currentPosition, currentPosition + depthStep);
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
    
    resultFbo.draw(0, 0);
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        string filename = "captured/" + ofGetTimestampString();
        
        ofDirectory dir(filename);
        dir.create();
        
        ofPixels pixels;
        pixels.allocate(cameraWidth, cameraHeight, GL_RGBA);
        resultFbo.readToPixels(pixels);
        
        ofSaveImage(pixels, filename + ".jpg", OF_IMAGE_QUALITY_BEST);
        
        float currentPosition = minDepthThreshold;
        int i = 0;
        while (currentPosition < maxDepthThreshold) {
            // draw lines at currentPosition
            drawLevel(currentPosition, currentPosition + depthStep);
            currentPosition += depthStep;
            
            char buff[100];
            snprintf(buff, sizeof(buff), "%03d", i);
            string buffAsStdStr = buff;
            
            makeSlice(currentPosition, currentPosition + depthStep);
            sliceFbo.readToPixels(pixels);
            ofSaveImage(pixels, filename + "/" + buffAsStdStr + ".jpg", OF_IMAGE_QUALITY_BEST);
            
            i++;
        }
    }
}

void ofApp::exit(){
    if (cameraFound) {
        pipe.stop();
    }
}
