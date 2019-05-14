//
//  frameBuffer.hpp
//  timeroom
//
//  Created by Petr Devaikin on 14.05.19.
//

#ifndef frameBuffer_hpp
#define frameBuffer_hpp

#include "rgbdFrame.hpp"

class frameBuffer {
    int capacity;
    int allocatedFrames = 0;
    rgbdFrame ** frames;
public:
    frameBuffer(int capacity);
    ~frameBuffer();
    
    void addFrame(rgbdFrame * newFrame);
    
    rgbdFrame * getFrameWithDelay(float delay);
    rgbdFrame * getLatestFrame();
};

#endif /* frameBuffer_hpp */
