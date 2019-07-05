#pragma once

#include "ofMain.h"
#include "ofxRealSense2.hpp"
#include "ofxOpenCv.h"
#include "videoBuffer.hpp"
#include "ofxGui.h"
#include "cameraFilter/cameraFilter.hpp"

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    
    bool getNewFrames(); // get new frames from camera if available;
    
    // Camera stuff
    ofxRealSense2 realSense;
    bool initCamera();
    // End of Camera stuff
    
    float timer;
    float timeDelta;
    
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
