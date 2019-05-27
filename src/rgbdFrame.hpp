//
//  rgbdFrame.hpp
//  timeroom
//
//  Created by Petr Devaikin on 03.05.19.
//

#ifndef rgbdFrame_hpp
#define rgbdFrame_hpp

#include "../librealsense2/rs.hpp"
#include "ofxOpenCv.h"
#include "ofMain.h"

class rgbdFrame {
private:
    int width;
    int height;
public:
    rgbdFrame(int width, int height);
    rgbdFrame(rs2::video_frame videoFrame, rs2::depth_frame depthFrame, float maxDepthValue);
    rgbdFrame(ofPixels rgbPixels, ofPixels depthColoredPixels);
    
    ofxCvGrayscaleImage depthImage;
    ofxCvColorImage colorImage;
    ofxCvColorImage depthImageColored; // same depth picture but with rgb channels (still black and white)
    //double timestamp;
};

#endif /* rgbdFrame_hpp */
