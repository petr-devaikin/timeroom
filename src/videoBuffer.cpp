//
//  videoBuffer.cpp
//  timeroom
//
//  Created by Petr Devaikin on 27.05.19.
//

#include "videoBuffer.hpp"

videoFragment::videoFragment(float startTimestamp, string filename, int width, int height) {
    this->startTimestamp = startTimestamp;
    this->filename = filename;
    this->width = width;
    this->height = height;
    state = preparedForRecording;
    
    // init recorder
    initRecorders();
    
    cout << "New fragment created " << filename << "\n";
}

videoFragment::~videoFragment() {
    ofRemoveListener(rgbRecorder.outputFileCompleteEvent, this, &videoFragment::recordingComplete);
    
    ofRemoveListener(depthRecorder.outputFileCompleteEvent, this, &videoFragment::recordingComplete);
}

void videoFragment::initRecorders() {
    rgbRecorder.setVideoCodec("mpeg4");
    rgbRecorder.setVideoBitrate("800k");
    rgbRecorder.setAudioCodec("mp3");
    rgbRecorder.setAudioBitrate("192k");
    ofAddListener(rgbRecorder.outputFileCompleteEvent, this, &videoFragment::recordingComplete);
    
    /*
    depthRecorder.setVideoCodec("mpeg4");
    depthRecorder.setVideoBitrate("800k");
    depthRecorder.setAudioCodec("mp3");
    depthRecorder.setAudioBitrate("192k");
    ofAddListener(depthRecorder.outputFileCompleteEvent, this, &videoFragment::recordingComplete);
     */
    
    rgbRecorder.setup(filename + "_rgb.mov", width, height, 15);
    //depthRecorder.setup(filename + "_depth.mov", width, height, 30);
}

void videoFragment::stopRecording() {
    rgbRecorder.close();
    //depthRecorder.close();
}

void videoFragment::recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args){
    cout << "The recoded video file is now complete." << "\n";
}

void videoFragment::addFrame(rgbdFrame frame) {
    // add new frame to the fragment
    if (state == preparedForRecording) {
        rgbRecorder.start();
        //depthRecorder.start();
        state = recording;
    }
    
    if (!rgbRecorder.addFrame(frame.colorImage.getPixels())) {
        ofLogWarning("RGB frame was not added!");
    }
    //if (!depthRecorder.addFrame(frame.depthImageColored.getPixels())) {
    //    ofLogWarning("Depth frame was not added!");
   // }
    
    if (rgbRecorder.hasVideoError()) {
        ofLogWarning("RGB The video recorder failed to write some frames!");
    }
    
    /*
    if (rgbRecorder.hasAudioError()) {
        ofLogWarning("RGB The video recorder failed to write some audio samples!");
    }
    */
    
    //if (depthRecorder.hasVideoError()) {
    //    ofLogWarning("Depth The video recorder failed to write some frames!");
    //}
    
    /*
    if (depthRecorder.hasAudioError()) {
        ofLogWarning("Depth The video recorder failed to write some audio samples!");
    }
     */
}

rgbdFrame videoFragment::getFrame(float position) {
    // need to check if player is initialized
    
    rgbPlayer.setPosition(position);
    depthPlayer.setPosition(position);
    
    return rgbdFrame(rgbPlayer.getPixels(), depthPlayer.getPixels());
}

void videoFragment::loadFromDisk() {
    rgbPlayer.load(filename + "_rgb.mov");
    depthPlayer.load(filename + "_depth.mov");
    
    state = inMemory;
    
    cout << "Fragment loaded " << filename << "\n";
}

void videoFragment::saveOnDisk() {
    if (state == recording) {
        stopRecording();
    }
    
    state = onDisk;
    
    cout << "Fragment saved " << filename << "\n";
}

/*
void videoFragment::clearMemory() {
    // remove video from memory
}
*/

void videoFragment::removeFromDisk() {
    // remove file from disk
    
    if (state == recording) {
        cout << "Removing file in recording state " << filename << "\n";
        stopRecording();
        state = stopping;
    }
    else if (state == onDisk)
        cout << "Fragment removed " << filename << "\n";
    else
        cout << "Nothing to remove for fragment " << filename << "\n";
}

bool videoFragment::isStopped() {
    cout << "rgb queue " << rgbRecorder.getVideoQueueSize() << "\n";
    cout << "depth queue " << depthRecorder.getVideoQueueSize() << "\n";
    return !rgbRecorder.isRecording() && !depthRecorder.isRecording();
}

// Video Buffer

videoBuffer::videoBuffer(int videoWidth, int videoHeight, float fragmentLength) {
    this->fragmentLength = fragmentLength;
    this->videoWidth = videoWidth;
    this->videoHeight = videoHeight;
}

videoBuffer::~videoBuffer() {
    for (int i = 0; i < fragments.size(); i++) {
        fragments[i]->removeFromDisk();
    }
    
    while (fragments.size()) {
        if (fragments[0]->isStopped()) {
            delete fragments[0];
            fragments.erase(fragments.begin());
        }
    }
}

videoFragment * videoBuffer::addNewFragment(float startTimestamp) {
    fileCounter++;
    
    char buff[100];
    snprintf(buff, sizeof(buff), "fragments/%05d", fileCounter);
    std::string fragmentFileName = buff;
    
    videoFragment * fragment = new videoFragment(startTimestamp, fragmentFileName, videoWidth, videoHeight);
    fragments.push_back(fragment);
    
    return fragment;
}

void videoBuffer::addFrame(rgbdFrame frame) {
    float currentTime = ofGetElapsedTimef();
    
    if (fragments.size() == 0) {
        // no recorded fragments yet
        videoFragment * fragment = addNewFragment(currentTime);
        fragment->addFrame(frame);
    }
    else {
        // get last fragment
        videoFragment * currentFragment = fragments[fragments.size() - 1];
        // check if length is already exceeded
        if (currentTime - currentFragment->startTimestamp >= fragmentLength) {
            currentFragment->saveOnDisk();
            
            videoFragment * fragment = addNewFragment(currentFragment->startTimestamp + fragmentLength);
            fragment->addFrame(frame);
        }
        else {
            currentFragment->addFrame(frame);
        }
    }
    
    //cout << "Add frame to " << fragments[fragments.size() - 1]->filename << "\n";
}

rgbdFrame videoBuffer::getFrame(float timestamp) {
    return rgbdFrame(videoWidth, videoHeight);
    if (fragments.size() == 0 || timestamp < fragments[0]->startTimestamp) {
        if (fragments.size() > 1 && fragments[0]->state == onDisk)
            fragments[0]->loadFromDisk();
        return rgbdFrame(videoWidth, videoHeight);
    }
    
    // remove old fragments
    while (fragments.size() > 1 && fragments[1]->startTimestamp > timestamp) {
        fragments[0]->removeFromDisk();
        delete fragments[0];
        fragments.erase(fragments.begin());
    }
    
    // add
    if (fragments.size()) {
        videoFragment * requestedFragment = fragments[0];
        if (requestedFragment->state == onDisk) requestedFragment->loadFromDisk();
        
        // check if requested frame has not been recorded yet
        if (timestamp < requestedFragment->startTimestamp)
            return rgbdFrame(videoWidth, videoHeight);
        else
            return requestedFragment->getFrame((timestamp - requestedFragment->startTimestamp) / fragmentLength);
    }
    else
        return rgbdFrame(videoWidth, videoHeight);
}
