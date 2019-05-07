//
//  rgbdFrame.cpp
//  timeroom
//
//  Created by Petr Devaikin on 03.05.19.
//

#include "rgbdFrame.hpp"

rgbdFrame::rgbdFrame(rs2::video_frame videoFrame, rs2::depth_frame depthFrame, float maxDepthValue) {
    timestamp = videoFrame.get_timestamp();
    width = videoFrame.get_width();
    height = videoFrame.get_height();
    
    // allocate images
    colorImage.setUseTexture(false);
    colorImage.allocate(depthFrame.get_width(), depthFrame.get_height());
    
    depthImage.setUseTexture(false);
    depthImage.allocate(depthFrame.get_width(), depthFrame.get_height());
    
    ofxCvShortImage depthRawImage; // temporal image before scaling
    depthRawImage.setUseTexture(false);
    depthRawImage.allocate(depthFrame.get_width(), depthFrame.get_height());
    
    // copy data
    memcpy(colorImage.getPixels().getData(), videoFrame.get_data(), width * height * 3);
    memcpy(depthRawImage.getShortPixelsRef().getData(), depthFrame.get_data(), width * height * 2);
    
    colorImage.flagImageChanged();
    depthRawImage.flagImageChanged();
    
    // rescale image
    depthRawImage.convertToRange(0, 65535. * (65535. / maxDepthValue));
    depthImage = depthRawImage;
}
