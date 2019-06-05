#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if (initCamera()) {
        frames = pipe.wait_for_frames();
        align_to_color = new rs2::align(RS2_STREAM_INFRARED);
    }
    
    resultFbo.allocate(cameraWidth, cameraHeight, GL_RGB);
    cout << "2\n";
    resultDepthFbo.allocate(cameraWidth, cameraHeight, GL_LUMINANCE);
    cout << "3\n";
    
    currentImage.allocate(cameraWidth, cameraHeight, GL_LUMINANCE);
    currentDepthImage.allocate(cameraWidth, cameraHeight, GL_LUMINANCE);
    
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
        //frames = align_to_color->process(frames);
        
        // !!! need to process all the frames in frameset, not only one
        
        // process last frame
        rs2::depth_frame depthFrame = frames.get_depth_frame();
        rs2::video_frame rgbFrame = frames.get_infrared_frame();
        
        rgbdFrame * newFrame = new rgbdFrame(currentTime, rgbFrame, depthFrame, MIN_DISTANCE / cameraDepthScale, MAX_DISTANCE / cameraDepthScale);
        
        buffer.addFrame(newFrame);  // add to the buffer
        
        // update current view
        currentImage.loadData(newFrame->irPixels);
        currentDepthImage.loadData(newFrame->depthPixels);
    }
    
    ofTexture resultTexture;
    ofTexture resultDepthTexture;
    ofTexture newLayerTexture;
    ofTexture newLayerDepthTexture;
    
    resultTexture = currentImage;
    resultDepthTexture = currentDepthImage;
    
    // past pictures
    for (float g : ghostTimestamps) {
        rgbdFrame * pastFrame = buffer.getFrame(g);
        newLayerTexture.loadData(pastFrame->irPixels);
        newLayerDepthTexture.loadData(pastFrame->depthPixels);
        
        resultFbo.begin();
        maskShader.begin();
        maskShader.setUniformTexture("tex1", resultTexture, 2);
        maskShader.setUniformTexture("tex1Depth", resultDepthTexture, 3);
        maskShader.setUniformTexture("tex0Depth", newLayerDepthTexture, 1);
        newLayerTexture.draw(0, 0);
        maskShader.end();
        resultFbo.end();
        
        resultDepthFbo.begin();
        maxShader.begin();
        maskShader.setUniformTexture("tex1", resultDepthTexture, 1);
        newLayerTexture.draw(0, 0);
        maxShader.end();
        resultDepthFbo.end();
        
        resultTexture = resultFbo.getTexture();
        resultDepthTexture = resultDepthFbo.getTexture();
        
        break;
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
    resultFbo.draw(0, 0);
    
    
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
