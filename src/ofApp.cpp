#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if (initCamera()) {
        frames = pipe.wait_for_frames();
        align_to_color = new rs2::align(RS2_STREAM_COLOR);
    }
    
    currentImage.allocate(cameraWidth, cameraHeight);
    currentDepthImage.allocate(cameraWidth, cameraHeight);
    
    for (int i = 0; i < REPEAT_NUMBER; i++) {
        pastImages[i].allocate(cameraWidth, cameraHeight);
        pastDepthImages[i].allocate(cameraWidth, cameraHeight);
    }
    
    mergedImage.allocate(cameraWidth, cameraHeight, GL_RGBA);
    tempFbo.allocate(cameraWidth, cameraHeight, GL_RGBA);
    
    maskShader.load("shadersGL3/mask");
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
        depth_sensor.set_option(RS2_OPTION_VISUAL_PRESET, RS2_RS400_VISUAL_PRESET_DEFAULT);
        depth_sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, 0.f);
        depth_sensor.set_option(RS2_OPTION_EXPOSURE, 30000);
        
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
        frames = align_to_color->process(frames);
        
        // !!! need to process all the frames in frameset, not only one
        
        // process last frame
        rs2::depth_frame depthFrame = frames.get_depth_frame();
        rs2::video_frame rgbFrame = frames.get_color_frame();
        frameBuffer.push_back(rgbdFrame(rgbFrame, depthFrame, MAX_DISTANCE / cameraDepthScale));    // add to the buffer
    }
    
    // update delayFrames
    for (int i = 0; i < REPEAT_NUMBER; i++) {
        // increase number of frames until the delay is not too big
        while (delayFrames[i] + 1 < frameBuffer.size() && frameBuffer[frameBuffer.size() - 1].timestamp - frameBuffer[delayFrames[i]].timestamp > delays[i])
            delayFrames[i] += 1;
    }
    
    // remove not used frames
    while (delayFrames[REPEAT_NUMBER - 1] > maxNotUsedFrames) {
        // now remove elements in vector
        frameBuffer.erase(frameBuffer.begin(), frameBuffer.begin() + maxNotUsedFrames);
        
        // adjust frame counters
        for (int i = 0; i < REPEAT_NUMBER; i++)
            delayFrames[i] -= maxNotUsedFrames;
    }
    
    // update images to show
    if (frameBuffer.size() > 0) {
        // current pictures
        currentImage = frameBuffer[frameBuffer.size() - 1].colorImage;
        currentDepthImage = frameBuffer[frameBuffer.size() - 1].depthImage;
        currentDepthImage.updateTexture();
        
        // past pictures
        for (int i = 0; i < REPEAT_NUMBER; i++) {
            pastImages[i] = frameBuffer[delayFrames[i]].colorImage;
            pastImages[i].updateTexture();
            pastDepthImages[i] = frameBuffer[delayFrames[i]].depthImage;
            pastDepthImages[i].updateTexture();
        }
        
        
        // merge images
        mergedImage.begin();
        
        maskShader.begin();
        maskShader.setUniformTexture("tex0Depth", currentDepthImage.getTexture(), 1);
        
        maskShader.setUniformTexture("background0", pastImages[0].getTexture(), 2);
        maskShader.setUniformTexture("background0Depth", pastDepthImages[0].getTexture(), 3);
        maskShader.setUniformTexture("background1", pastImages[1].getTexture(), 4);
        maskShader.setUniformTexture("background1Depth", pastDepthImages[1].getTexture(), 5);
        
        currentImage.draw(0, 0);
        
        maskShader.end();

        mergedImage.end();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    updateFrames();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0);
    
    if (!cameraFound) {
        ofSetColor(255);
        ofDrawBitmapString("Camera not found", 10, 20);
        return;
    }
    
    if (frameBuffer.size() > 0) {
        
        // latest frame
        //currentImage.draw(0, 0, cameraWidth / 2, cameraHeight / 2);
        
        // frame from the past
        //pastImage.draw(cameraWidth / 2, 0, cameraWidth / 2, cameraHeight / 2);
        
        // diff mask
        //maskRGBImage.draw(0, cameraHeight / 2, cameraWidth / 2, cameraHeight / 2);
        
        // merged
        mergedImage.draw(0, 0);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

void ofApp::exit(){
    if (cameraFound) {
        pipe.stop();
        delete align_to_color;
    }
}
