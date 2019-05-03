//
//  rgbdFrame.hpp
//  timeroom
//
//  Created by Petr Devaikin on 03.05.19.
//

#ifndef rgbdFrame_hpp
#define rgbdFrame_hpp

#include "../librealsense2/rs.hpp"

class rgbdFrame {
private:
    int width;
    int height;
public:
    rgbdFrame(rs2::video_frame videoFrame, rs2::depth_frame depthFrame);
    ~rgbdFrame();
    
    unsigned short * depthData;
    unsigned char * rgbData;
    double timestamp;
};

#endif /* rgbdFrame_hpp */
