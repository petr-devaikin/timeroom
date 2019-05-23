#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "rgbdFrame.hpp"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    
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
    
    ofxCvShortImage depthImage;
    ofxCvGrayscaleImage scaledDepthImage;
    ofxCvGrayscaleImage processedImage;
    
    ofShader outlineShader;
    ofShader fadeOutShader;
    ofFbo resultFbo;
    ofFbo tempFbo;
    
    void drawLevel(float depth);
    
    float timer;
    float lastFadeOutTime;
    
    float currentPosition;
    double cameraMinDepth;
    double cameraMaxDepth;
    
    float minDepth;
    float maxDepth;
    
    ofxFloatSlider minDepthThreshold;
    ofxFloatSlider maxDepthThreshold;
    ofxFloatSlider depthStep;
    ofxFloatSlider travelPeriod;
    ofxFloatSlider fadeOutPeriod;
    ofxPanel gui;
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
