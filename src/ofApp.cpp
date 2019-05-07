#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if (initCamera()) {
        frames = pipe.wait_for_frames();
        align_to_color = new rs2::align(RS2_STREAM_COLOR);
    }
    
    currentImage.allocate(cameraWidth, cameraHeight);
    pastImage.allocate(cameraWidth, cameraHeight);
    
    currentDepthImage.allocate(cameraWidth, cameraHeight);
    pastDepthImage.allocate(cameraWidth, cameraHeight);
    
    mergedImage.allocate(cameraWidth, cameraHeight, GL_RGBA);
    
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
        frameBuffer.push_back(new rgbdFrame(rgbFrame, depthFrame, MAX_DISTANCE / cameraDepthScale));    // add to the buffer
    }
    
    // !!! change while to faster stuff
    while (frameBuffer.size() > 0 && frameBuffer[frameBuffer.size() - 1]->timestamp - frameBuffer[0]->timestamp > maxDelay) {
        delete frameBuffer[0];
        frameBuffer.erase(frameBuffer.begin());
    }
    
    // update images to show
    if (frameBuffer.size() > 0) {
        // current pictures
        currentImage = frameBuffer[frameBuffer.size() - 1]->colorImage;
        currentDepthImage = frameBuffer[frameBuffer.size() - 1]->depthImage;
        currentDepthImage.updateTexture();
        
        // past pictures
        pastImage = frameBuffer[0]->colorImage;
        pastImage.updateTexture();
        pastDepthImage = frameBuffer[0]->depthImage;
        pastDepthImage.updateTexture();
        
        
        // merge images
        mergedImage.begin();
        
        maskShader.begin();
        maskShader.setUniformTexture("tex0Depth", currentDepthImage.getTexture(), 1);
        maskShader.setUniformTexture("background", pastImage.getTexture(), 2);
        maskShader.setUniformTexture("backgroundDepth", pastDepthImage.getTexture(), 3);
        
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
