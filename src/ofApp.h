#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "rgbdFrame.hpp"

#define REPEAT_NUMBER 2

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    const float STRIPE_STEP = 0.1; // in meters
    
    float delays[REPEAT_NUMBER] = {5000, 10000};
    int delayFrames[REPEAT_NUMBER] = {0, 0};
    const int maxNotUsedFrames = 5; // to remove not used frame not one by one
    
    // Camera stuff
    bool initCamera();
    void updateFrames(); // get new frames from camera if available;
    bool cameraFound;
    float cameraDepthScale; // meters per depth 1
    rs2::pipeline pipe;
    rs2::frameset frames;
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    // End of Camera stuff
    
    ofxCvShortImage currentDepthImage;
    ofxCvGrayscaleImage currentDepthPreviewImage;
    ofxCvGrayscaleImage stripedImage;
    ofxCvColorImage convertedToSaveDepthImage;
public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    
    float maxDistance = 2;
    float stripeStep = 0.025; // stripe depth in meters
    float stripeShift = 0; // stripe shift in meters
};
