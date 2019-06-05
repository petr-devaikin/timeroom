//
//  rgbdFrame.cpp
//  timeroom
//
//  Created by Petr Devaikin on 03.05.19.
//

#include "rgbdFrame.hpp"

rgbdFrame::rgbdFrame(int width, int height) {
    depthImage.setUseTexture(false);
    depthImage.allocate(width, height);
    //depthImage.clear();
    depthImage.invert(); // to make it "white" - all pixels are far away
    
    irImage.setUseTexture(false);
    //colorImage.clear();
    irImage.allocate(width, height);
}


rgbdFrame::rgbdFrame(rs2::video_frame videoFrame, rs2::depth_frame depthFrame, float minDepthValue, float maxDepthValue) {
    //timestamp = videoFrame.get_timestamp();
    width = videoFrame.get_width();
    height = videoFrame.get_height();
    
    // allocate images
    irImage.setUseTexture(false);
    irImage.allocate(depthFrame.get_width(), depthFrame.get_height());
    
    depthImage.setUseTexture(false);
    depthImage.allocate(depthFrame.get_width(), depthFrame.get_height());
    
    
    ofxCvShortImage depthRawImage; // temporal image before scaling
    depthRawImage.setUseTexture(false);
    depthRawImage.allocate(depthFrame.get_width(), depthFrame.get_height());
    
    // copy data
    memcpy(irImage.getPixels().getData(), videoFrame.get_data(), width * height * 3);
    memcpy(depthRawImage.getShortPixelsRef().getData(), depthFrame.get_data(), width * height * 2);
    
    irImage.flagImageChanged();
    depthRawImage.flagImageChanged();
    
    // rescale image
    float newMinPoints = minDepthValue * 65535. / (maxDepthValue - minDepthValue);
    float newMaxPoints = minDepthValue + 65535. * 65535. / (maxDepthValue - minDepthValue);
    
    depthRawImage.convertToRange(-newMinPoints, newMaxPoints);
    depthImage = depthRawImage;
}
