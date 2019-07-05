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
    
    videoPixels.allocate(width, height, OF_IMAGE_GRAYSCALE);
    memset(videoPixels.getData(), 0, width * height);
}


rgbdFrame::rgbdFrame(float timestamp, ofPixels &videoPixels, ofShortPixels& depthPixels, float minDepthValue, float maxDepthValue) {
    this->timestamp = timestamp;
    width = videoPixels.getWidth();
    height = videoPixels.getHeight();
    
    // allocate images
    
    // ir pixels
    this->videoPixels.allocate(width, height, videoPixels.getNumChannels());
    this->videoPixels = videoPixels;
    
    // depth pixels
    ofxCvShortImage depthRawImage; // temporal image before scaling
    depthRawImage.setUseTexture(false);
    depthRawImage.allocate(width, height);
    
    ofxCvGrayscaleImage depthConvertedImage;
    depthConvertedImage.setUseTexture(false);
    depthConvertedImage.allocate(width, height);
    
    depthRawImage.getShortPixelsRef() = depthPixels;
    depthRawImage.flagImageChanged();
    
    // rescale image
    float newMinPoints = minDepthValue * 65535. / (maxDepthValue - minDepthValue);
    float newMaxPoints = minDepthValue + 65535. * 65535. / (maxDepthValue - minDepthValue);
    
    depthRawImage.convertToRange(-newMinPoints, newMaxPoints);
    depthConvertedImage = depthRawImage;
    
    this->depthPixels.allocate(width, height, 1);
    this->depthPixels = depthConvertedImage.getPixels();
}
