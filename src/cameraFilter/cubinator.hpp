//
//  cubinator.hpp
//  timeroom
//
//  Created by Petr Devaikin on 17.07.19.
//

#ifndef cubinator_hpp
#define cubinator_hpp

#include "cameraFilter.hpp"

struct coloredBox {
    ofBoxPrimitive primitive;
    ofMaterial material;
};

class cubinator : public cameraFilter {
private:
    void generateBoxes();
    void deleteBoxes();
    coloredBox ** boxes;
    int columnNumber;
    int rowNumber;
    
    void rescaleImage(ofImage &img); // crop and rescale to extract color and position
    
    void regenerateCubes(int &p);
    
    void updateBoxPosition(ofPixels& depthPixels);
    void updateBoxColor(ofPixels& colorPixels);
    
    // Scene
    void setupScene();
    void updateScene();
    ofCamera camera;
    ofLight light;
    ofFbo result;
    
    // GUI
    void initGui();
    ofxIntSlider cameraMarginHor;
    ofxIntSlider cameraMarginVert;
    ofxIntSlider pixelSize;
    ofxFloatSlider minDistance;
    ofxFloatSlider maxDistance;
    ofxFloatSlider cubePerMeter;
    
    ofxIntSlider cubeSize;
    
    ofxVec3Slider cameraPosition;
    ofxVec2Slider cameraRotation;
public:
    cubinator(ofxRealSense2 * realSense);
    ~cubinator();
    
    void update() override;
    void draw() override;
};

#endif /* cubinator_hpp */
