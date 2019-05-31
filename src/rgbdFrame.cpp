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
    
    colorImage.setUseTexture(false);
    //colorImage.clear();
    colorImage.allocate(width, height);
    
    depthImageColored.setUseTexture(false);
    depthImageColored.allocate(width, height);
    //depthImageColored.clear();
    depthImageColored.invert(); // to make it "white" - all pixels are far away
}

rgbdFrame::rgbdFrame(ofPixels rgbPixels, ofPixels depthColoredPixels) {
    depthImage.setUseTexture(false);
    depthImage.allocate(width, height);
    
    colorImage.setUseTexture(false);
    colorImage.allocate(width, height);
    
    depthImageColored.setUseTexture(false);
    depthImageColored.allocate(width, height);
    
    // copy data
    colorImage.setFromPixels(rgbPixels);
    depthImageColored.setFromPixels(depthColoredPixels);
    depthImage = depthColoredPixels;
}

rgbdFrame::rgbdFrame(rs2::video_frame videoFrame, rs2::depth_frame depthFrame, float maxDepthValue) {
    //timestamp = videoFrame.get_timestamp();
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
    depthImageColored = depthImage;
}
