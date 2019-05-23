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
    
    processedImage.allocate(cameraWidth, cameraHeight);
    
    resultFbo.allocate(cameraWidth, cameraHeight, GL_RGBA);
    tempFbo.allocate(cameraWidth, cameraHeight, GL_RGBA);
    
    // clear result
    
    resultFbo.begin();
    ofClear(0);
    resultFbo.end();
    
    // GUI
    gui.setup();
    gui.add(minDepth.setup("min depth", 1, 1, 8));
    gui.add(maxDepth.setup("max depth", 4, 1, 8));
    gui.add(depthStep.setup("depth step", 0.1, 0.05, 0.5));
    gui.add(travelPeriod.setup("travel period", 1, 0.1, 5));
    gui.add(fadeOutPeriod.setup("fade out period", 1, 0.1, 5));
    
    currentPosition = minDepth;
    
    // init timer
    timer = ofGetElapsedTimef();
    lastFadeOutTime = timer;
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
        
        // scale image
        float minDepthPoints = minDepth / cameraDepthScale;
        float maxDepthPoints = maxDepth / cameraDepthScale;
        
        float newMinPoints = minDepthPoints * 65535. / (maxDepthPoints - minDepthPoints);
        float newMaxPoints = minDepthPoints + 65535. * 65535. / (maxDepthPoints - minDepthPoints);
        
        depthImage.convertToRange(-newMinPoints, newMaxPoints);
        
        // transform to greyscale
        scaledDepthImage = depthImage;
    }
}

void ofApp::drawLevel(float depth) {
    // draw lines at depth level = depth
    float depthPoints = 255 * (depth - minDepth) / (maxDepth - minDepth);
    
    processedImage = scaledDepthImage;
    processedImage.threshold(depthPoints);
    
    tempFbo.begin();
    resultFbo.draw(0, 0);
    tempFbo.end();
    
    resultFbo.begin();
    
    outlineShader.begin();
    outlineShader.setUniformTexture("backgroundTex", resultFbo.getTexture(), 1);
    
    processedImage.draw(0, 0);
    
    outlineShader.end();
    
    resultFbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    float lastTime = timer;
    timer = ofGetElapsedTimef();
    float timeDiff = timer - lastTime;
    
    // fade out prev pic;
    float periodPerFadeOutBit = fadeOutPeriod / 255;
    float fadeOutValue = (timer - lastFadeOutTime) / periodPerFadeOutBit;
    int fadeOutValueFloor = floor(fadeOutValue);
    
    cout << fadeOutValue << " " << fadeOutValueFloor << "\n";
    
    if (fadeOutValueFloor > 0) {
        tempFbo.begin();
        resultFbo.draw(0, 0);
        tempFbo.end();
    
        resultFbo.begin();
        fadeOutShader.begin();
        fadeOutShader.setUniform1f("difference", fadeOutValueFloor / 255.);
        tempFbo.draw(0, 0);
        fadeOutShader.end();
        resultFbo.end();
        
        lastFadeOutTime += fadeOutValueFloor * periodPerFadeOutBit;
    }
    
    updateFrames();
    
    // update currentPosition;
    float lastPosition = currentPosition;
    
    float step = (maxDepth - minDepth) / travelPeriod * timeDiff;
    
    float newPosition = currentPosition + step;
    if (newPosition <= maxDepth) {
        while (currentPosition <= newPosition) {
            // draw lines at currentPosition
            drawLevel(currentPosition);
            
            currentPosition += step;
        }
    }
    else {
        while (currentPosition < maxDepth) {
            // draw lines at currentPosition
            drawLevel(currentPosition);
            
            currentPosition += step;
        }
        
        newPosition = newPosition - maxDepth + minDepth;
        currentPosition = minDepth;
        while (currentPosition < newPosition) {
            // draw lines at currentPosition
            drawLevel(currentPosition);
            
            currentPosition += step;
        }
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

}

void ofApp::exit(){
    if (cameraFound) {
        pipe.stop();
    }
}
