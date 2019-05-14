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
    
    // setup timer
    timer = ofGetElapsedTimef();
    
    // allocate buffer
    buffer = new frameBuffer(90 * MAX_BUFFER_LENGTH); // 90 - max fps
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
        //depth_sensor.set_option(RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE, 0.0f);
        
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
        buffer->addFrame(new rgbdFrame(rgbFrame, depthFrame, MAX_DISTANCE / cameraDepthScale)); // add to the buffer
    }
    
    // update images to show
    currentImage = buffer->getLatestFrame()->colorImage;
    currentDepthImage = buffer->getLatestFrame()->colorImage;
    currentDepthImage.updateTexture();
        
    // past pictures
    for (int i = 0; i < REPEAT_NUMBER; i++) {
        rgbdFrame * frame = buffer->getFrameWithDelay(currentDelays[i]);
        pastImages[i] = frame->colorImage;
        pastImages[i].updateTexture();
        pastDepthImages[i] = frame->depthImage;
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
    maskShader.setUniformTexture("background2", pastImages[2].getTexture(), 6);
    maskShader.setUniformTexture("background2Depth", pastDepthImages[2].getTexture(), 7);
    
    currentImage.draw(0, 0);
    
    maskShader.end();

    mergedImage.end();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (!cameraFound) return;
    
    // update timer
    float currentTime = ofGetElapsedTimef();
    float timeDifference = currentTime - timer;
    timer = currentTime;
    
    // update current delay time
    for (int i = 0; i < REPEAT_NUMBER; i++) {
        if (!makeDelays) {
             // substract to 0
            if (currentDelays[i] > 0) {
                currentDelays[i] -= DELAY_SUBTRACTION_SPEED * timeDifference;
                if (currentDelays[i] < 0) currentDelays[i] = 0;
            }
        }
        else {
            // increase to desired value
            if (currentDelays[i] < desiredDelays[i]) {
                currentDelays[i] += DELAY_SUBTRACTION_SPEED * timeDifference;
                if (currentDelays[i] > desiredDelays[i]) currentDelays[i] = desiredDelays[i];
            }
        }
    }
    
    
    // update frames
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
    
    mergedImage.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        makeDelays = !makeDelays;
    }
}

void ofApp::exit(){
    if (cameraFound) {
        pipe.stop();
        delete align_to_color;
    }
    
    delete buffer;
}
