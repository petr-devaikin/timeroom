//
//  ghostMaker.hpp
//  timeroom
//
//  Created by Petr Devaikin on 05.07.19.
//

#ifndef ghostMaker_hpp
#define ghostMaker_hpp

#include "cameraFilter.hpp"
#include "videoBuffer.hpp"
#include <ofxRealSense2.hpp>
#include "ofxGui.h"

class ghostMaker : public cameraFilter {
private:
    videoBuffer * buffer;
    
    list<float> ghostTimestamps;
    float lastGhostGeneratedTimestamp = 0;
    
    ofShader maskShader;
    ofShader maxShader;
    
    float timer;
    
    ofFbo resultFbo;
    ofFbo resultDepthFbo;
    
    ofFbo tempFbo;
    ofFbo tempDepthFbo;
    
    void removeOldGhosts();
    void addNewGhost();
    void updateGhosts();
    
    void mergeImages();
    
    // GUI
    void initGui();
    ofxPanel gui;
    ofxFloatSlider maxGhostLifetime;
    ofxFloatSlider ghostGenerationInterval;
public:
    ghostMaker(ofxRealSense2 * realSense);
    ~ghostMaker();
    
    // constructor: buffer(1280, 720, 30)
    
    void update(rgbdFrame * newFrame) override;
    void draw() override;
    void drawGui(float x, float y) override;
    
    void setMaxGhostLifetime(float lifetime);
    void setGhostGenerationInterfal(float interval);
};

#endif /* ghostMaker_hpp */
