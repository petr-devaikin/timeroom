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
    
<<<<<<< HEAD
=======
    ofTexture currentImage;
    ofTexture currentDepthImage;
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    
    rs2::align * align_to_color;
    
>>>>>>> parent of 6a71f49... working!
    ofShader maskShader;
    ofShader maxShader;
    
    ofFbo resultFbo;
    ofFbo resultDepthFbo;
<<<<<<< HEAD
    ofFbo tempFbo;
    ofFbo tempDepthFbo;
    
=======
>>>>>>> parent of 6a71f49... working!
    
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
