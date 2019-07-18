//
//  cameraFilter.hpp
//  timeroom
//
//  Created by Petr Devaikin on 05.07.19.
//

#ifndef cameraFilter_hpp
#define cameraFilter_hpp

#include <ofxRealSense2.hpp>
#include "ofxGui.h"
#include "ofMain.h"

class cameraFilter {
protected:
    ofxRealSense2 * realSense;
    ofxPanel gui;
public:
    cameraFilter(ofxRealSense2 * realSense);
    virtual ~cameraFilter();
    
    virtual void update() = 0;
    virtual void draw() = 0;
    void drawGui(float x, float y);
};

#endif /* cameraFilter_hpp */
