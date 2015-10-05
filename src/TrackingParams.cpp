#include "TrackingParams.h"


TrackingParams::TrackingParams(){    
}

void TrackingParams::setup(string name){
    parameters.setName(name);
    parameters.add(enabled.set("COLOR TRACKER", false));
    parameters.add(camPosition.set("Video draw position",
                                   ofGetWindowRect().getTopLeft(),
                                   ofGetWindowRect().getTopLeft(),
                                   ofPoint(ofGetWindowRect().getBottomRight().x - CAM_WIDTH,
                                           ofGetWindowRect().getBottomRight().y - CAM_HEIGHT)));
}

void TrackingParams::draw(){

}
