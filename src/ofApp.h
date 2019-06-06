#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "videoBuffer.hpp"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    
    videoBuffer buffer;
    
    list<float> ghostTimestamps;
    float lastGhostGeneratedTimestamp = 0;
    
    void removeOldGhosts();
    void addNewGhost();
    void updateGhosts();
    
    void mergeImages();
    
    // Camera stuff
    bool initCamera();
    bool updateFrames(); // get new frames from camera if available;
    bool cameraFound;
    float cameraDepthScale; // meters per depth 1
    rs2::pipeline pipe;
    rs2::frameset frames;
    // End of Camera stuff
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    
    rs2::align * align_to_color;
    
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
    ofApp() : buffer(1280, 720, 60) {};
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    
    bool showGui;
};
