#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "rgbdFrame.hpp"

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    const float MAX_DISTANCE = 4.;
    int maxDelay = 1000;
    
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
    ofxCvColorImage pastImage;
    ofxCvGrayscaleImage pastDepthImage;
    
    vector<rgbdFrame*> frameBuffer;
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    
    rs2::align * align_to_color;
    
    ofShader maskShader;
    ofFbo mergedImage;
public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
};
