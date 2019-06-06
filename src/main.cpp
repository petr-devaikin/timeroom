#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    ofGLFWWindowSettings settings;
    settings.setGLVersion(3, 2); //we define the OpenGL version we want to use
    settings.windowMode = OF_FULLSCREEN;
    
    //settings.windowMode = OF_GAME_MODE;
    //settings.width = 1024;
    //settings.height = 768;
    ofCreateWindow(settings);
    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
}
