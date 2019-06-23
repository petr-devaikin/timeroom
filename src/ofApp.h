#pragma once

#include "ofMain.h"
#include "ofxRealSense2.hpp"
#include "ofxOpenCv.h"
#include "videoBuffer.hpp"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    
    videoBuffer buffer;
    bool getNewFrames(); // get new frames from camera if available;
    
    list<float> ghostTimestamps;
    float lastGhostGeneratedTimestamp = 0;
    
    void removeOldGhosts();
    void addNewGhost();
    void updateGhosts();
    
    void mergeImages();
    
    // Camera stuff
    ofxRealSense2 realSense;
    bool initCamera();
    bool cameraFound;
    // End of Camera stuff
    
    ofShader maskShader;
    ofShader maxShader;
    
    ofFbo resultFbo;
    ofFbo resultDepthFbo;
    ofFbo tempFbo;
    ofFbo tempDepthFbo;
    
    
    float timer;
    float timeDelta;
    
    // GUI
    void initGui();
    ofxPanel gui;
    
    ofxFloatSlider minDistance;
    ofxFloatSlider maxDistance;
    ofxFloatSlider resultScale;
    ofxVec2Slider resultShift;
    
    ofxFloatSlider maxGhostLifeTime;
    ofxFloatSlider ghostGenerationInterval;
public:
    ofApp() : buffer(1280, 720, 30) {};
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    
    bool showGui;
};
