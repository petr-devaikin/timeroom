//
//  rgbdFrame.cpp
//  timeroom
//
//  Created by Petr Devaikin on 03.05.19.
//

#include "rgbdFrame.hpp"

rgbdFrame::rgbdFrame(rs2::video_frame videoFrame, rs2::depth_frame depthFrame) {
    timestamp = videoFrame.get_timestamp();
    width = videoFrame.get_width();
    height = videoFrame.get_height();
    
    depthData = new unsigned short[width * height];
    rgbData = new unsigned char[3 * width * height];
    
    // copy data
    memcpy(depthData, depthFrame.get_data(), width * height * 2);
    memcpy(rgbData, videoFrame.get_data(), width * height * 3);
}

rgbdFrame::~rgbdFrame() {
    delete [] depthData;
    delete [] rgbData;
}
