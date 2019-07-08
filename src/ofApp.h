#pragma once

#include "ofMain.h"
#include "ofxRealSense2.hpp"
#include "ofxOpenCv.h"
#include "videoBuffer.hpp"
#include "ofxGui.h"
#include "cameraFilter/cameraFilter.hpp"
#include "cameraFilter/ghostMaker.hpp"

class ofApp : public ofBaseApp{
private:
    
    bool getNewFrames(); // get new frames from camera if available;
    
    // Camera stuff
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    
    ofxRealSense2 realSense;
    bool initCamera();
    
    cameraFilter * filter;
    // End of Camera stuff
    
    // GUI
    void initGui();
    ofxPanel gui;
    
    ofxFloatSlider minDistance;
    ofxFloatSlider maxDistance;
    ofxFloatSlider resultScale;
    ofxVec2Slider resultShift;
public:
    ofApp() {};
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    
    bool showGui;
};
