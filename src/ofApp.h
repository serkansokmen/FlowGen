#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"

#include "TrackingParams.h"


using namespace cv;
using namespace ofxCv;


class ofApp : public ofBaseApp {

public:
    
    void setup();
    void setupGui();
    void update();
    void draw();
    void exit();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    
    void toggleCamera(bool open);
    
    ofVideoGrabber      grabber;
    cv::Mat             grabberGray;
    ofxCv::FlowPyrLK    flow;
    ofVec2f             p1;
    ofRectangle         rect;
    
    ofParameterGroup    parameters;
    TrackingParams      trackingParams;
    
    ofxPanel            gui;
    bool                bDrawGui;
};
