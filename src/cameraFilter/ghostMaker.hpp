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

class ghostMaker : public cameraFilter {
private:
    float maxGhostLifeTime;
    float ghostGenerationInterval;
    
    videoBuffer * buffer;
    
    list<float> ghostTimestamps;
    float lastGhostGeneratedTimestamp = 0;
    
    ofShader maskShader;
    ofShader maxShader;
    
    ofFbo resultFbo;
    ofFbo resultDepthFbo;
    
    ofFbo tempFbo;
    ofFbo tempDepthFbo;
    
    void removeOldGhosts();
    void addNewGhost();
    void updateGhosts();
    
    void mergeImages();
public:
    ghostMaker(ofxRealSense2 realSense, float maxGhostLifetime, float ghostGenerationInterval) : cameraFilter(realSense) { };
    ~ghostMaker();
    
    // constructor: buffer(1280, 720, 30)
    
    void update() override;
    void draw() override;
    
    void setMaxGhostLifetime(float lifetime);
    void setGhostGenerationInterfal(float interval);
};

#endif /* ghostMaker_hpp */
