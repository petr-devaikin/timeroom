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
#include "rgbdFrame.hpp"

class ghostMaker : public cameraFilter {
private:
    videoBuffer * buffer;
    
    list<float> ghostTimestamps;
    float lastGhostGeneratedTimestamp = 0;
    
    ofShader maskShader;
    ofShader maxShader;
    
    float timer;
    float timeDelta;
    
    ofFbo resultFbo;
    ofFbo resultDepthFbo;
    
    ofFbo tempFbo;
    ofFbo tempDepthFbo;
    
    void updateGhosts();
    void removeOldGhosts();
    void addNewGhost();
    
    void mergeImages();
    
    // GUI
    ofxFloatSlider maxGhostLifetime;
    ofxFloatSlider ghostGenerationInterval;
    ofxFloatSlider minDistance;
    ofxFloatSlider maxDistance;
protected:
    void initGui() override;
public:
    ghostMaker(ofxRealSense2 * realSense);
    ~ghostMaker();
    
    // constructor: buffer(1280, 720, 30)
    
    void update() override;
    void draw() override;
    
    void setMaxGhostLifetime(float lifetime);
    void setGhostGenerationInterfal(float interval);
};

#endif /* ghostMaker_hpp */
