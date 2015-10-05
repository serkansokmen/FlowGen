#ifndef TrackingParams_h
#define TrackingParams_h

//#include "ofParameterGroup.h"
//#include "ofParameter.h"
#include "ofMain.h"

#define CAM_WIDTH           320
#define CAM_HEIGHT          240

//class TrackingParams;

//template<typename ParameterType>
//class ofTrackingParamsParam: public ofReadOnlyParameter<ParameterType,TrackingParams>{
//    friend class TrackingParams;
//};

class TrackingParams {
public:
    TrackingParams();
    
    void setup(string name);
    void draw();
    
    ofParameterGroup        parameters;
    ofParameter<ofVec2f>    camPosition;
    ofParameter<bool>       enabled;
};


#endif /* TrackingParams_h */
