#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    //------------------------------------------------
    // Main setup
    //------------------------------------------------
    ofBackground(0);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofEnableAntiAliasing();
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
    
    grabber.initGrabber(640,480);
    
    setupGui();
    bDrawGui = true;
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    if(grabber.isFrameNew()){
        flow.calcOpticalFlow(grabber);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofFill();
    ofSetColor(ofColor::white);
    grabber.draw(0,0);
    flow.draw();
    
    ofSetColor(ofColor::white);
    if(ofGetMousePressed()){
        ofNoFill();
        ofDrawRectangle(rect);
    }
    
    if (bDrawGui && trackingParams.enabled) {
        grabber.draw(0, 0, grabber.getWidth(), grabber.getHeight());
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch (key){
        case ' ':
            bDrawGui = !bDrawGui;
            break;
        case 's':
            gui.saveToFile("settings.xml");
            break;
        case 'l':
            gui.loadFromFile("settings.xml");
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    ofVec2f p2(x,y);
    rect.set(p1,p2.x-p1.x,p2.y-p1.y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    p1.set(x,y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    ofVec2f p2(x,y);
    rect.set(p1,p2.x-p1.x,p2.y-p1.y);
    vector<KeyPoint> keypoints;
    vector<KeyPoint> keypointsInside;
    vector<cv::Point2f> featuresToTrack;
    copyGray(grabber, grabberGray);
    FAST(grabberGray,keypoints,2);
    for(int i=0;i<keypoints.size();i++){
        if(rect.inside(toOf(keypoints[i].pt))){
            keypointsInside.push_back(keypoints[i]);
        }
    }
#if CV_MAJOR_VERSION>=2 && (CV_MINOR_VERSION>4 || (CV_MINOR_VERSION==4 && CV_SUBMINOR_VERSION>=1))
    KeyPointsFilter::retainBest(keypointsInside,30);
#endif
    KeyPoint::convert(keypointsInside,featuresToTrack);
    flow.setFeaturesToTrack(featuresToTrack);
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::toggleCamera(bool isOpen){
    if (isOpen) {
        grabber.initGrabber(CAM_WIDTH, CAM_HEIGHT);
    } else {
        if (grabber.isInitialized()) {
            grabber.close();
        }
        ofBackground(0);
    }
}

//--------------------------------------------------------------
void ofApp::setupGui(){
    
    trackingParams.setup("COLOR TRACKING");
    
    parameters.setName("CLYD");
    parameters.add(trackingParams.parameters);
    
    gui.setup(parameters);
    
    //    gui.setDefaultTextPadding(40);
    
//    trackingParams.enabled.addListener(this, &ofApp::toggleCamera);
    
    gui.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofApp::exit(){
//    trackingParams.enabled.removeListener(this, &ofApp::toggleCamera);
}

