#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    initCamera();
    
    currentImage.allocate(cameraWidth, cameraHeight);
    pastImage.allocate(cameraWidth, cameraHeight);
    
    currentDepthImage.setUseTexture(false);
    currentDepthImage.allocate(cameraWidth, cameraHeight);
    pastDepthImage.setUseTexture(false);
    pastDepthImage.allocate(cameraWidth, cameraHeight);
}

void ofApp::initCamera() {
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
        
        frames = pipe.wait_for_frames();
        cout << "RealSense found!\n";
        
        align_to_color = new rs2::align(RS2_STREAM_COLOR);
        
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
        frameBuffer.push_back(new rgbdFrame(rgbFrame, depthFrame));    // add to the buffer
    }
    
    // !!! change while to faster stuff
    while (frameBuffer.size() > 0 && frameBuffer[frameBuffer.size() - 1]->timestamp - frameBuffer[0]->timestamp > maxDelay) {
        delete frameBuffer[0];
        frameBuffer.erase(frameBuffer.begin());
    }
    
    // update images to show
    if (frameBuffer.size() > 0) {
        // rgb data
        memcpy((currentImage.getPixels().getData()), frameBuffer[frameBuffer.size() - 1]->rgbData, 3 * cameraWidth * cameraHeight);
        currentImage.flagImageChanged();
        
        memcpy((pastImage.getPixels().getData()), frameBuffer[0]->rgbData, 3 * cameraWidth * cameraHeight);
        pastImage.flagImageChanged();
        
        // depth data
        memcpy((currentDepthImage.getPixels().getData()), frameBuffer[frameBuffer.size() - 1]->depthData, 2 * cameraWidth * cameraHeight);
        memcpy((pastDepthImage.getPixels().getData()), frameBuffer[0]->depthData, 2 * cameraWidth * cameraHeight);
        
        // calculate mask for past!
        currentDepthImage -= pastDepthImage;
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
        currentImage.draw(0, 0, cameraWidth / 2, cameraHeight / 2);
        
        // frame from the past
        pastImage.draw(0, cameraHeight / 2, cameraWidth / 2, cameraHeight / 2);
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
