//
//  frameBuffer.cpp
//  timeroom
//
//  Created by Petr Devaikin on 14.05.19.
//

#include "frameBuffer.hpp"

frameBuffer::frameBuffer(int capacity) {
    this->capacity = capacity;
    frames = new rgbdFrame*[capacity];
}

frameBuffer::~frameBuffer() {
    for (int i = 0; i < allocatedFrames; i++) {
        delete frames[i];
    }
    
    delete [] frames;
}

void frameBuffer::addFrame(rgbdFrame * newFrame) {
    if (allocatedFrames < capacity) {
        frames[allocatedFrames] = newFrame;
        allocatedFrames++;
    }
    else {
        // shift frames
        delete frames[0];
        for (int i = 0; i < capacity - 1; i++) {
            frames[i] = frames[i+1];
        }
        
        // add new frame
        frames[capacity - 1] = newFrame;
    }
}

rgbdFrame * frameBuffer::getLatestFrame() {
    return frames[allocatedFrames - 1];
}

rgbdFrame * frameBuffer::getFrameWithDelay(float delay) {
    float latestTime = frames[allocatedFrames - 1]->timestamp;
    
    int frameIndex = allocatedFrames - 2;
    while (frameIndex >= 0 && latestTime - frames[frameIndex]->timestamp < delay) {
        frameIndex--;
    }
    
    frameIndex++;
    return frames[frameIndex];
}
