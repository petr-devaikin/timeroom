//
//  videoBuffer.cpp
//  timeroom
//
//  Created by Petr Devaikin on 27.05.19.
//

#include "videoBuffer.hpp"


videoBuffer::videoBuffer(int videoWidth, int videoHeight, float maxLength) {
    this->maxLength = maxLength;
    this->videoWidth = videoWidth;
    this->videoHeight = videoHeight;
    
    addedFrameCount = 0;
    totalFrameCount = maxLength * MAX_FPS;
    
    frames = new rgbdFrame * [totalFrameCount];
    emptyFrame = new rgbdFrame(videoWidth, videoHeight);
}

videoBuffer::~videoBuffer() {
    for (int i = 0; i < addedFrameCount; i++) {
        delete frames[i];
    }
    
    delete[] frames;
    
    delete emptyFrame;
}

void videoBuffer::addFrame(rgbdFrame * frame) {
    float currentTime = ofGetElapsedTimef();
    
    // remove old frames
    int framesToRemove = 0;
    while (framesToRemove > addedFrameCount && currentTime - frames[framesToRemove]->timestamp > maxLength)
        framesToRemove++;
    
    framesToRemove--;
    
    if (framesToRemove > 0) {
        // remove old frames
        for (int i = 0; i < framesToRemove; i++) {
            delete frames[i];
        }
        
        // shift not removed frames to the beginning
        addedFrameCount -= framesToRemove;
        for (int i = 0; i < addedFrameCount; i++) {
            frames[i] = frames[i + framesToRemove];
        }
    }
    
    // add new frame
    if (addedFrameCount < totalFrameCount) {
        frames[addedFrameCount] = frame;
        addedFrameCount++;
    }
    else
        ofLogWarning("Buffer overloaded");
}

rgbdFrame * videoBuffer::getFrame(float timestamp) {
    if (addedFrameCount == 0) {
        ofLogWarning("Trying to get a frame while no frames added");
        return emptyFrame;
    }
    
    rgbdFrame * oldestFrame = frames[0];
    rgbdFrame * newestFrame = frames[addedFrameCount - 1];
    
    if (timestamp < oldestFrame->timestamp) {
        ofLogWarning("Trying to get a too old frame");
        return emptyFrame;
    }
    else if (timestamp >= newestFrame->timestamp) {
        return newestFrame;
    }
    else {
        int position = floor((timestamp - oldestFrame->timestamp) / (newestFrame->timestamp - oldestFrame->timestamp));
        return frames[position];
    }
}
