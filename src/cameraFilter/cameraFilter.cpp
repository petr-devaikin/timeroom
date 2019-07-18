//
//  cameraFilter.cpp
//  timeroom
//
//  Created by Petr Devaikin on 05.07.19.
//

#include "cameraFilter.hpp"

cameraFilter::cameraFilter(ofxRealSense2 * realSense) {
    this->realSense = realSense;
    gui.setup();
}

cameraFilter::~cameraFilter() {
}

void cameraFilter::drawGui(float x, float y) {
    gui.setPosition(x, y);
    gui.draw();
}
