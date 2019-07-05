//
//  cameraFilter.hpp
//  timeroom
//
//  Created by Petr Devaikin on 05.07.19.
//

#ifndef cameraFilter_hpp
#define cameraFilter_hpp

#include <ofxRealSense2.hpp>

class cameraFilter {
protected:
    ofxRealSense2 * realSense;
public:
    cameraFilter(ofxRealSense2 * realSense);
    virtual void update() = 0;
    virtual void draw() = 0;
};

#endif /* cameraFilter_hpp */
