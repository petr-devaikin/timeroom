//
//  cameraFilter.cpp
//  timeroom
//
//  Created by Petr Devaikin on 05.07.19.
//

#include "cameraFilter.hpp"

cameraFilter::cameraFilter(ofxRealSense2 * realSense) {
    this->realSense = realSense;
}

void cameraFilter::drawGui(float x, float y) {
    ofPushMatrix();
    ofTranslate(x, y);
    gui.draw();
    ofPopMatrix();
}

void cameraFilter::initGui() {
    gui.setup();
}
