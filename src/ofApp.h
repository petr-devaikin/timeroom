#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "rgbdFrame.hpp"

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    int maxDelay = 1000;
    bool cameraFound;
    
    rs2::pipeline pipe;
    rs2::frameset frames;
    
    ofxCvColorImage currentImage;
    ofxCvShortImage currentDepthImage;
    ofxCvColorImage pastImage;
    ofxCvShortImage pastDepthImage;
    
    vector<rgbdFrame*> frameBuffer;
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    
    rs2::align * align_to_color;
    rs2::colorizer colorizer;
public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    
    void initCamera();
    void updateFrames(); // get new frames from camera if available;
};
