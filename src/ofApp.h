#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "rgbdFrame.hpp"
#include "frameBuffer.hpp"

#define REPEAT_NUMBER 3

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    const float MAX_DISTANCE = 4.;
    
    const int MAX_BUFFER_LENGTH = 2; // in seconds
    
    float timer;
    
    float desiredDelays[REPEAT_NUMBER] = {461.5, 2 * 461.5, 3 * 461.5};
    float currentDelays[REPEAT_NUMBER] = {0, 0, 0};
    const float DELAY_SUBTRACTION_SPEED = 4 * 1000; // how fast current delays will become 0 in seconds
    
    bool makeDelays = false;
    
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
    
    frameBuffer * buffer;
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    
    rs2::align * align_to_color;
    
    ofShader maskShader;
    ofFbo mergedImage;
    ofFbo tempFbo;
public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
};
