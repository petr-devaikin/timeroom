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
    float currentTime = ofGetElapsedTimef();
    
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
        
        rgbdFrame newFrame(rgbFrame, depthFrame, MAX_DISTANCE / cameraDepthScale);
        
        buffer.addFrame(newFrame);  // add to the buffer
        
        // update current view
        currentImage = newFrame.colorImage;
        currentDepthImage = newFrame.depthImage;
        currentDepthImage.updateTexture();
    }
    
    // past pictures
    for (int i = 0; i < REPEAT_NUMBER; i++) {
        rgbdFrame pastFrame = buffer.getFrame(currentTime - delays[i]);
        pastImages[i] = pastFrame.colorImage;
        pastImages[i].updateTexture();
        pastDepthImages[i] = pastFrame.depthImage;
        pastDepthImages[i].updateTexture();
    }
    
    return;
    // merge images
    mergedImage.begin();
    
    maskShader.begin();
    maskShader.setUniformTexture("tex0Depth", currentDepthImage.getTexture(), 1);
    
    maskShader.setUniformTexture("background0", pastImages[0].getTexture(), 2);
    maskShader.setUniformTexture("background0Depth", pastDepthImages[0].getTexture(), 3);
    //maskShader.setUniformTexture("background1", pastImages[1].getTexture(), 4);
    //maskShader.setUniformTexture("background1Depth", pastDepthImages[1].getTexture(), 5);
    
    currentImage.draw(0, 0);
    
    maskShader.end();

    mergedImage.end();
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
    
    // latest frame
    currentImage.draw(0, 0, cameraWidth / 2, cameraHeight / 2);
    currentDepthImage.draw(0, cameraHeight / 2, cameraWidth / 2, cameraHeight / 2);
    
    // frame from the past
    pastImages[0].draw(cameraWidth / 2, 0, cameraWidth / 2, cameraHeight / 2);
    pastDepthImages[0].draw(cameraWidth / 2, cameraHeight / 2, cameraWidth / 2, cameraHeight / 2);
    
    
    // diff mask
    //maskRGBImage.draw(0, cameraHeight / 2, cameraWidth / 2, cameraHeight / 2);
    
    // merged
    //mergedImage.draw(0, 0);
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
