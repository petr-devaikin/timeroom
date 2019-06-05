#pragma once

#include "ofMain.h"
#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "videoBuffer.hpp"

class ofApp : public ofBaseApp{
private:
    const int cameraWidth = 1280;
    const int cameraHeight = 720;
    const float MIN_DISTANCE = 1.;
    const float MAX_DISTANCE = 4.;
    
    videoBuffer buffer;
    
    list<float> ghostTimestamps;
    float maxGhostLifeTime = 4;
    float ghostGenerationInterval = 1;
    float lastGhostGeneratedTimestamp = 0;
    
    void removeOldGhosts();
    void addNewGhost();
    
    void updateGhosts();
    
    // Camera stuff
    bool initCamera();
    void updateFrames(); // get new frames from camera if available;
    bool cameraFound;
    float cameraDepthScale; // meters per depth 1
    rs2::pipeline pipe;
    rs2::frameset frames;
    // End of Camera stuff
    
    ofTexture currentImage;
    ofTexture currentDepthImage;
    
    rs2::temporal_filter temp_filter;
    rs2::hole_filling_filter hole_filter;
    
    rs2::align * align_to_color;
    
    ofShader maskShader;
    ofShader maxShader;
    ofFbo resultFbo;
    ofFbo resultDepthFbo;
    
    float timer;
    float timeDelta;
public:
    ofApp() : buffer(1280, 720, 15) {};
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
};
