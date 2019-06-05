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
    
    emptyFrame = new rgbdFrame(videoWidth, videoHeight);
}

videoBuffer::~videoBuffer() {
    for (rgbdFrame * f : frames) {
        delete f;
    }
    
    delete emptyFrame;
}

void videoBuffer::addFrame(rgbdFrame * frame) {
    float currentTime = ofGetElapsedTimef();
    
    // remove old frames
    int framesToRemove = 0;
    while (frames.size() > 1 && currentTime - frames.front()->timestamp > maxLength) {
        delete frames.front();
        frames.pop_front();
    }
    
    // add new frame
    frames.push_back(frame);
}

rgbdFrame * videoBuffer::getFrame(float timestamp) {
    if (frames.size() == 0) {
        //ofLogWarning("Trying to get a frame while no frames added");
        return emptyFrame;
    }
    
    rgbdFrame * oldestFrame = frames.front();
    rgbdFrame * newestFrame = frames.back();
    
    if (timestamp < oldestFrame->timestamp) {
        ofLogWarning("Trying to get too old frame");
        return emptyFrame;
    }
    else if (timestamp >= newestFrame->timestamp) {
        return newestFrame;
    }
    else {
        unsigned int position = floor(frames.size() * (timestamp - oldestFrame->timestamp) / (newestFrame->timestamp - oldestFrame->timestamp));
        return * next(frames.begin(), position);
    }
}
