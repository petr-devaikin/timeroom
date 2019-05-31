#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "videoBuffer.hpp"

#define REPEAT_NUMBER 2

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    const float MAX_DISTANCE = 4.;
    
    videoBuffer buffer;
    
    float delays[REPEAT_NUMBER] = {13000};
    const int maxNotUsedFrames = 5; // to remove not used frame not one by one
    
    // Camera stuff
    bool initCamera();
    void updateFrames(); // get new frames from camera if available;
    bool cameraFound;
    float cameraDepthScale; // meters per depth 1
    rs2::pipeline pipe;
    rs2::frameset frames;
    // End of Camera stuff
    
    ofxCvColorImage currentImage;
    ofxCvGrayscaleImage currentDepthImage;
    ofxCvColorImage pastImages[REPEAT_NUMBER];
    ofxCvGrayscaleImage pastDepthImages[REPEAT_NUMBER];
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    
    rs2::align * align_to_color;
    
    ofShader maskShader;
    ofFbo mergedImage;
    ofFbo tempFbo;
public:
    ofApp() : buffer(1280, 720, 5000) {};
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
};
