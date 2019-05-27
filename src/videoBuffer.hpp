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

enum fragmentState { recording, onDisk, inMemory };

class videoFragment {
private:
    ofxVideoRecorder rgbRecorder;
    ofxVideoRecorder depthRecorder;
    void recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args);
public:
    videoFragment(float startTimestamp, string filename);
    
    float startTimestamp;
    fragmentState state;
    string filename;
    
    void addFrame(rgbdFrame frame);
    rgbdFrame getFrame(float time); // from startTimestamp
    
    void loadFromDisk();
    void saveOnDisk();
    void removeFromDisk();
    void clearMemory();
};

class videoBuffer {
private:
    float fragmentLength;
    int videoWidth, videoHeight;
    int fileCounter = 0;
    
    vector<videoFragment> fragments;
    
    videoFragment addNewFragment(float startTimestamp);
public:
    videoBuffer(int videoWidth, int videoHeight, float fragmentLengts = 10000);
    ~videoBuffer();
    
    void addFrame(rgbdFrame frame);
    rgbdFrame getFrame(float timestamp);
};

#endif /* videoBuffer_hpp */
