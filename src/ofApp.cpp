#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if (initCamera()) {
        frames = pipe.wait_for_frames();
        align_to_color = new rs2::align(RS2_STREAM_COLOR);
    }
    
    resultFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    resultDepthFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    tempFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    tempDepthFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    
    maskShader.load("shadersGL3/mask");
    maxShader.load("shadersGL3/max");
    
    timer = 0;
}

bool ofApp::initCamera() {
    cout << "Looking for RealSense\n";
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, cameraWidth, cameraHeight);
    cfg.enable_stream(RS2_STREAM_DEPTH, cameraWidth, cameraHeight);
    cfg.enable_stream(RS2_STREAM_INFRARED, cameraWidth, cameraHeight, RS2_FORMAT_Y8);
    
    rs2::context ctx;
    auto device_list = ctx.query_devices();
    
    if (device_list.size() > 0) {
        rs2::pipeline_profile profile = pipe.start(cfg);
        auto depth_sensor = profile.get_device().first<rs2::depth_sensor>();
        
        // depth sensor settings
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
        
        rgbdFrame * newFrame = new rgbdFrame(currentTime, rgbFrame, depthFrame, MIN_DISTANCE / cameraDepthScale, MAX_DISTANCE / cameraDepthScale);
        
        buffer.addFrame(newFrame);  // add to the buffer
        
        // update current view
        ofTexture newLayerTexture;
        ofTexture newLayerDepthTexture;
        
        newLayerTexture.loadData(newFrame->colorPixels);
        newLayerDepthTexture.loadData(newFrame->depthPixels);
        
        // init fbo's with current pictures
        resultFbo.begin();
        newLayerTexture.draw(0, 0);
        resultFbo.end();
        resultDepthFbo.begin();
        newLayerDepthTexture.draw(0, 0);
        resultDepthFbo.end();
    
        // past pictures
        for (float g : ghostTimestamps) {
            // update temp Fbo's
            tempFbo.begin();
            resultFbo.draw(0, 0);
            tempFbo.end();
            tempDepthFbo.begin();
            resultDepthFbo.draw(0, 0);
            tempDepthFbo.end();
            
            // get past pictures
            rgbdFrame * pastFrame = buffer.getFrame(g);
            newLayerTexture.loadData(pastFrame->colorPixels);
            newLayerDepthTexture.loadData(pastFrame->depthPixels);
            
            // update result
            resultFbo.begin();
            maskShader.begin();
            maskShader.setUniformTexture("tex1", tempFbo.getTexture(), 2);
            maskShader.setUniformTexture("tex1Depth", tempDepthFbo.getTexture(), 3);
            maskShader.setUniformTexture("tex0Depth", newLayerDepthTexture, 1);
            newLayerTexture.draw(0, 0);
            maskShader.end();
            resultFbo.end();
            
            // update resulting depth
            resultDepthFbo.begin();
            maxShader.begin();
            maxShader.setUniformTexture("tex1", tempDepthFbo.getTexture(), 1);
            newLayerDepthTexture.draw(0, 0);
            maxShader.end();
            resultDepthFbo.end();
        }
    }
}

void ofApp::removeOldGhosts() {
    while (ghostTimestamps.size() && timer - ghostTimestamps.front() > maxGhostLifeTime * 2)
        ghostTimestamps.pop_front();
}

void ofApp::addNewGhost() {
    while (lastGhostGeneratedTimestamp + ghostGenerationInterval <= timer) {
        lastGhostGeneratedTimestamp += ghostGenerationInterval;
        if (timer - lastGhostGeneratedTimestamp <= 2 * maxGhostLifeTime)
            ghostTimestamps.push_back(lastGhostGeneratedTimestamp);
    }
}

void ofApp::updateGhosts() {
    if (ghostTimestamps.size()) {
        auto p = ghostTimestamps.begin();
        while (p != ghostTimestamps.end()) {
            *p -= timeDelta;
            p++;
        }
        
        removeOldGhosts();
    }
    
    addNewGhost();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    float newTimer = ofGetElapsedTimef();
    timeDelta = newTimer - timer;
    timer = newTimer;
    
    updateGhosts();
    
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
    resultFbo.draw(0, 0, cameraWidth / 2, cameraHeight / 2);
    resultDepthFbo.draw(cameraWidth / 2, 0, cameraWidth / 2, cameraHeight / 2);
    
    
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
