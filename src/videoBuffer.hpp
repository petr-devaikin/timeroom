//
//  videoBuffer.hpp
//  timeroom
//
//  Created by Petr Devaikin on 27.05.19.
//

#ifndef videoBuffer_hpp
#define videoBuffer_hpp

#include "rgbdFrame.hpp"
#include "ofMain.h"
#include "ofxVideoRecorder.h"

#define MAX_FPS 60

class videoBuffer {
private:
    float maxLength;
    int videoWidth, videoHeight;
    
    list<rgbdFrame *> frames;
    rgbdFrame * emptyFrame;
public:
    videoBuffer(int videoWidth, int videoHeight, float maxLength);
    ~videoBuffer();
    
    void addFrame(rgbdFrame * frame);
    rgbdFrame * getFrame(float timestamp);
};

#endif /* videoBuffer_hpp */
