#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    if (initCamera()) {
        frames = pipe.wait_for_frames();
    }
    
    convertedToSaveDepthImage.allocate(cameraWidth, cameraHeight);
    currentDepthPreviewImage.allocate(cameraWidth, cameraHeight);
    stripedImage.allocate(cameraWidth, cameraHeight);
    
    currentDepthImage.setUseTexture(false);
    currentDepthImage.allocate(cameraWidth, cameraHeight);
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
        depth_sensor.set_option(RS2_OPTION_VISUAL_PRESET, RS2_RS400_VISUAL_PRESET_HIGH_ACCURACY);
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
        memcpy(currentDepthImage.getShortPixelsRef().getData(), depthFrame.get_data(), cameraWidth * cameraHeight * 2);
        
        // prepare preview
        currentDepthPreviewImage = currentDepthImage;
        
        // prepare converted to save and striped versions
        for (int i = 0; i < cameraWidth; i++)
            for (int j = 0; j < cameraHeight; j++) {
                float distance = depthFrame.get_distance(i, j);
                if (distance > maxDistance) distance = maxDistance;
                if (((int) round((distance - stripeShift) / stripeStep)) % 2)
                    stripedImage.getPixels().setColor(i, j, ofColor::white);
                else
                    stripedImage.getPixels().setColor(i, j, ofColor::black);
                
                convertedToSaveDepthImage.getPixels().getData()[3 * (i + j * cameraWidth)] = ((unsigned char * ) depthFrame.get_data())[2 * (i + j * cameraWidth)];
                convertedToSaveDepthImage.getPixels().getData()[3 * (i + j * cameraWidth) + 1] = ((unsigned char * ) depthFrame.get_data())[2 * (i + j * cameraWidth) + 1];
                convertedToSaveDepthImage.getPixels().getData()[3 * (i + j * cameraWidth) + 2] = 0;
            }
        
        stripedImage.flagImageChanged();
        convertedToSaveDepthImage.flagImageChanged();
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
    
    //currentDepthPreviewImage.draw(0, 0, cameraWidth / 2, cameraHeight / 2);
    stripedImage.draw(0, 0);
    //convertedToSaveDepthImage.draw(0, cameraHeight / 2, cameraWidth / 2, cameraHeight / 2);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

void ofApp::exit(){
    if (cameraFound) {
        pipe.stop();
    }
}
