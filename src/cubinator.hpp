//
//  cubinator.hpp
//  timeroom
//
//  Created by Petr Devaikin on 17.07.19.
//

#ifndef cubinator_hpp
#define cubinator_hpp

#include "cameraFilter.hpp"

class cubinator : public cameraFilter {
private:
    int pixelSize;
    int rowNumber;
    int columnNumber;
    ofBoxPrimitive ** boxes;
    
    void updateBoxPosition(ofPixels& depthPixels);
    void updateBoxColor(ofPixels& colorPixels);
    
    // GUI
    ofxFloatSlider minDistance;
    ofxFloatSlider maxDistance;
protected:
    void initGui() override;
public:
    cubinator(ofxRealSense2 * realSense, int pixelSize);
    ~cubinator();
    
    void update() override;
    void draw() override;
};

#endif /* cubinator_hpp */
