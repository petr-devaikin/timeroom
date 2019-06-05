//
//  rgbdFrame.cpp
//  timeroom
//
//  Created by Petr Devaikin on 03.05.19.
//

#include "rgbdFrame.hpp"

rgbdFrame::rgbdFrame(int width, int height) {
    this->width = width;
    this->height = height;
    
    depthPixels.allocate(width, height, OF_IMAGE_GRAYSCALE);
    memset(depthPixels.getData(), 255, width * height);
    
    irPixels.allocate(width, height, OF_IMAGE_GRAYSCALE);
    memset(irPixels.getData(), 0, width * height);
}


rgbdFrame::rgbdFrame(float timestamp, rs2::video_frame videoFrame, rs2::depth_frame depthFrame, float minDepthValue, float maxDepthValue) {
    this->timestamp = timestamp;
    width = videoFrame.get_width();
    height = videoFrame.get_height();
    
    // allocate images
    
    // ir pixels
    irPixels.allocate(width, height, OF_IMAGE_GRAYSCALE);
    memcpy(irPixels.getData(), videoFrame.get_data(), width * height);
    
    // depth pixels
    ofxCvShortImage depthRawImage; // temporal image before scaling
    depthRawImage.setUseTexture(false);
    depthRawImage.allocate(width, height);
    
    ofxCvGrayscaleImage depthConvertedImage;
    depthConvertedImage.setUseTexture(false);
    depthConvertedImage.allocate(width, height);
    
    memcpy(depthRawImage.getShortPixelsRef().getData(), depthFrame.get_data(), width * height * 2);
    
    depthRawImage.flagImageChanged();
    
    // rescale image
    float newMinPoints = minDepthValue * 65535. / (maxDepthValue - minDepthValue);
    float newMaxPoints = minDepthValue + 65535. * 65535. / (maxDepthValue - minDepthValue);
    
    depthRawImage.convertToRange(-newMinPoints, newMaxPoints);
    depthConvertedImage = depthRawImage;
    
    depthPixels.allocate(width, height, OF_IMAGE_GRAYSCALE);
    memcpy(depthPixels.getData(), depthConvertedImage.getPixels().getData(), width * height);
}
