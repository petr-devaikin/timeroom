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

enum fragmentState { preparedForRecording, recording, onDisk, inMemory, stopping };

class videoFragment {
private:
    ofxVideoRecorder rgbRecorder;
    ofxVideoRecorder depthRecorder;
    ofVideoPlayer rgbPlayer;
    ofVideoPlayer depthPlayer;
    
    int width, height;
    
    void initRecorders();
    void stopRecording();
public:
    videoFragment(float startTimestamp, string filename, int width, int height);
    ~videoFragment();
    
    float startTimestamp;
    fragmentState state;
    string filename;
    
    void addFrame(rgbdFrame frame);
    rgbdFrame getFrame(float position); // in percentage
    
    void loadFromDisk();
    void saveOnDisk();
    void removeFromDisk();
    //void clearMemory();
    
    bool isStopped();
    
    void recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args);
};

class videoBuffer {
private:
    float fragmentLength;
    int videoWidth, videoHeight;
    int fileCounter = 0;
    
    vector<videoFragment *> fragments;
    
    videoFragment * addNewFragment(float startTimestamp);
public:
    videoBuffer(int videoWidth, int videoHeight, float fragmentLengts = 5);
    ~videoBuffer();
    
    void addFrame(rgbdFrame frame);
    rgbdFrame getFrame(float timestamp);
};

#endif /* videoBuffer_hpp */
