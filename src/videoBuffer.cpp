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
    
    // init recorder
    rgbRecorder.setVideoCodec("mpeg4");
    rgbRecorder.setVideoBitrate("800k");
    //vidRecorder.setAudioCodec("mp3");
    //vidRecorder.setAudioBitrate("192k");
    rgbRecorder.
    ofAddListener(rgbRecorder.outputFileCompleteEvent, this, &videoFragment::recordingComplete);
    
    depthRecorder.setVideoCodec("mpeg4");
    depthRecorder.setVideoBitrate("800k");
    //vidRecorder.setAudioCodec("mp3");
    //vidRecorder.setAudioBitrate("192k");
    depthRecorder.
    ofAddListener(depthRecorder.outputFileCompleteEvent, this, &videoFragment::recordingComplete);
    
    cout << "New fragment created" << filename << "\n";
}

void videoFragment::recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args){
    cout << "The recoded video file is now complete." << "\n";
}

void videoFragment::addFrame(rgbdFrame frame) {
    // add new frame to the fragment
    rgbRecorder.addFrame(frame.colorImage.getPixels());
    depthRecorder.addFrame(frame.depthImageColored.getPixels());
    
    if (rgbRecorder.hasVideoError()) {
        ofLogWarning("RGB The video recorder failed to write some frames!");
    }
    
    if (rgbRecorder.hasAudioError()) {
        ofLogWarning("RGB The video recorder failed to write some audio samples!");
    }
    
    if (depthRecorder.hasVideoError()) {
        ofLogWarning("Depth The video recorder failed to write some frames!");
    }
    
    if (depthRecorder.hasAudioError()) {
        ofLogWarning("Depth The video recorder failed to write some audio samples!");
    }
}

rgbdFrame videoFragment::getFrame(float time) {
    // add new frame to the fragment
    
    return rgbdFrame(1, 1); // <----- CHANGE !!!!!
}

void videoFragment::loadFromDisk() {
    // ...
    
    state = inMemory;
    
    cout << "Fragment loaded " << filename << "\n";
}

void videoFragment::saveOnDisk() {
    if (state == recording) {
        ofRemoveListener(recorder.outputFileCompleteEvent, this, &videoFragment::recordingComplete);
        recorder.close();
    }
    
    clearMemory();
    
    state = onDisk;
    
    cout << "Fragment saved " << filename << "\n";
}

void videoFragment::clearMemory() {
    // remove video from memory
}

void videoFragment::removeFromDisk() {
    // remove file from disk
    
    cout << "Fragment removed " << filename << "\n";
}

// Video Buffer

videoBuffer::videoBuffer(int videoWidth, int videoHeight, float fragmentLength) {
    this->fragmentLength = fragmentLength;
    this->videoWidth = videoWidth;
    this->videoHeight = videoHeight;
}

videoFragment videoBuffer::addNewFragment(float startTimestamp) {
    fileCounter++;
    
    char buff[100];
    snprintf(buff, sizeof(buff), "fragments/%05d", fileCounter);
    std::string fragmentFileName = buff;
    
    videoFragment fragment = videoFragment(startTimestamp, fragmentFileName);
    fragments.push_back(fragment);
    
    return fragment;
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
