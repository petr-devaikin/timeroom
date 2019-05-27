//
//  videoBuffer.cpp
//  timeroom
//
//  Created by Petr Devaikin on 27.05.19.
//

#include "videoBuffer.hpp"

videoFragment::videoFragment(float startTimestamp, string filename) {
    this->startTimestamp = startTimestamp;
    this->filename = filename;
    state = recording;
}

void videoFragment::addFrame(rgbdFrame frame) {
    // add new frame to the fragment
}

rgbdFrame videoFragment::getFrame(float time) {
    // add new frame to the fragment
    
    return rgbdFrame(1, 1); // <----- CHANGE !!!!!
}

void videoFragment::loadFromDisk() {
    // ...
    
    state = inMemory;
}

void videoFragment::saveOnDisk() {
    // ...
    
    clearMemory();
}

void videoFragment::clearMemory() {
    // remove video from memory
    
    state = inMemory;
}

void videoFragment::removeFromDisk() {
    // remove file from disk
}

// Video Buffer

videoBuffer::videoBuffer(int videoWidth, int videoHeight, float fragmentLength) {
    this->fragmentLength = fragmentLength;
    this->videoWidth = videoWidth;
    this->videoHeight = videoHeight;
}

videoFragment videoBuffer::addNewFragment(float startTimestamp) {
    char buff[100];
    snprintf(buff, sizeof(buff), "fragments/%05d", fileCounter++);
    std::string fragmentFileName = buff;
    
    return videoFragment(startTimestamp, fragmentFileName);
}

void videoBuffer::addFrame(rgbdFrame frame) {
    float currentTime = ofGetElapsedTimef();
    
    if (fragments.size() == 0) {
        // no recorded fragments yet
        videoFragment fragment = addNewFragment(currentTime);
        fragment.addFrame(frame);
    }
    else {
        // get last fragment
        videoFragment currentFragment = fragments[fragments.size() - 1];
        // check if length is already exceeded
        if (currentTime - currentFragment.startTimestamp + fragmentLength >= fragmentLength) {
            currentFragment.saveOnDisk();
            
            videoFragment fragment = addNewFragment(currentFragment.startTimestamp + fragmentLength);
            fragment.addFrame(frame);
        }
    }
}

rgbdFrame videoBuffer::getFrame(float timestamp) {
    while (fragments.size() > 1 && fragments[1].startTimestamp >= timestamp) {
        fragments[0].removeFromDisk();
        fragments.erase(fragments.begin());
    }
    
    videoFragment requestedFragment = fragments[0];
    if (requestedFragment.state == onDisk) requestedFragment.loadFromDisk();
    
    // check if requested frame has not been recorded yet
    if (timestamp < requestedFragment.startTimestamp)
        return rgbdFrame(videoWidth, videoHeight);
    else
        return requestedFragment.getFrame(timestamp - requestedFragment.startTimestamp);
}
