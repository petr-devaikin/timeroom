//
//  rgbdFrame.hpp
//  timeroom
//
//  Created by Petr Devaikin on 03.05.19.
//

#ifndef rgbdFrame_hpp
#define rgbdFrame_hpp

#include "ofxOpenCv.h"
#include "ofMain.h"

class rgbdFrame {
private:
    int width;
    int height;
public:
    rgbdFrame(int width, int height);
    rgbdFrame(float timestamp, ofPixels& videoPixels, ofShortPixels& depthPixels, float minDepthValue, float maxDepthValue);
    
    ofShortPixels depthPixels;
    ofPixels colorPixels;
    
    double timestamp;
};

#endif /* rgbdFrame_hpp */
