#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(false);
    ofSetLogLevel(OF_LOG_NOTICE);
    
    drawWidth = 1280;
    drawHeight = 720;
    // process all but the density on 16th resolution
    flowWidth = drawWidth / 4;
    flowHeight = drawHeight / 4;
    
    // FLOW & MASK
    opticalFlow.setup(flowWidth, flowHeight);
    velocityMask.setup(drawWidth, drawHeight);
    
    // enable depth->video image calibration
    kinect.setRegistration(true);
    
    kinect.init();
    //kinect.init(true); // shows infrared instead of RGB video image
    //kinect.init(false, false); // disable video image (faster fps)
    
    kinect.open();		// opens first available kinect
    //kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
    //kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #

    // print the intrinsic IR sensor values
    if(kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }
    didCamUpdate = false;
    cameraFbo.allocate(kinect.getWidth(), kinect.getHeight());
    cameraFbo.clear();
    
    // Allocate images
    colorImg.allocate(kinect.width, kinect.height, OF_IMAGE_COLOR);
    grayImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshNear.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayThreshFar.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    grayPreprocImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    
    // Configure contour finder
    contourFinder.setMinAreaRadius(10);
    contourFinder.setMaxAreaRadius(200);
    contourFinder.setFindHoles(false);
    

    
    // FLUID & PARTICLES
#ifdef USE_FASTER_INTERNAL_FORMATS
    fluidSimulation.setup(flowWidth, flowHeight, drawWidth, drawHeight, true);
    particleFlow.setup(flowWidth, flowHeight, drawWidth, drawHeight, true);
#else
    fluidSimulation.setup(flowWidth, flowHeight, drawWidth, drawHeight, false);
    particleFlow.setup(flowWidth, flowHeight, drawWidth, drawHeight, false);
#endif
    
    obstacleImage.load("obstacle.png");
    fluidSimulation.addObstacle(obstacleImage.getTexture());
    
    // VISUALIZATION
    displayScalar.setup(flowWidth, flowHeight);
    velocityField.setup(flowWidth / 4, flowHeight / 4);
    temperatureField.setup(flowWidth / 4, flowHeight / 4);
    pressureField.setup(flowWidth / 4, flowHeight / 4);
    velocityTemperatureField.setup(flowWidth / 4, flowHeight / 4);
    
    // MOUSE DRAW
    mouseForces.setup(flowWidth, flowHeight, drawWidth, drawHeight);
    
    // GUI
    setupGui();
    
    lastTime = ofGetElapsedTimef();
    
}

//--------------------------------------------------------------
void ofApp::setupGui() {
    
    gui.setup("settings");
    gui.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));
    gui.setDefaultFillColor(ofColor(160, 160, 160, 160));
    gui.add(guiFPS.set("average FPS", 0, 0, 60));
    gui.add(guiMinFPS.set("minimum FPS", 0, 0, 60));
    gui.add(doFullScreen.set("fullscreen (F)", false));
    doFullScreen.addListener(this, &ofApp::setFullScreen);
    gui.add(toggleGuiDraw.set("show gui (G)", false));
    gui.add(doFlipCamera.set("flip camera", true));
    gui.add(showObstacle.set("show obstacle", true));
    gui.add(doDrawCamBackground.set("draw camera (C)", true));
    
    gui.add(drawMode.set("draw mode", DRAW_COMPOSITE, DRAW_COMPOSITE, DRAW_MOUSE));
    drawMode.addListener(this, &ofApp::drawModeSetName);
    gui.add(drawName.set("MODE", "draw name"));
    
    
    int guiColorSwitch = 0;
    ofColor guiHeaderColor[2];
    guiHeaderColor[0].set(160, 160, 80, 200);
    guiHeaderColor[1].set(80, 160, 160, 200);
    ofColor guiFillColor[2];
    guiFillColor[0].set(160, 160, 80, 200);
    guiFillColor[1].set(80, 160, 160, 200);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(opticalFlow.parameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(velocityMask.parameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(fluidSimulation.parameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(particleFlow.parameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(mouseForces.leftButtonParameters);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(mouseForces.rightButtonParameters);
    
    
    kinectParameters.setName("input source");
    kinectParameters.add(nearThreshold.set("near threshold", 230, 0, 500));
    kinectParameters.add(farThreshold.set("near threshold", 220, 0, 500));
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(kinectParameters);
    
    
    visualizeParameters.setName("visualizers");
    visualizeParameters.add(showScalar.set("show scalar", true));
    visualizeParameters.add(displayScalarScale.set("scalar scale", 0.15, 0.05, 0.5));
    displayScalarScale.addListener(this, &ofApp::setDisplayScalarScale);
    visualizeParameters.add(velocityFieldScale.set("velocity scale", 0.1, 0.0, 0.5));
    velocityFieldScale.addListener(this, &ofApp::setVelocityFieldScale);
    visualizeParameters.add(temperatureFieldScale.set("temperature scale", 0.1, 0.0, 0.5));
    temperatureFieldScale.addListener(this, &ofApp::setTemperatureFieldScale);
    visualizeParameters.add(pressureFieldScale.set("pressure scale", 0.02, 0.0, 0.5));
    pressureFieldScale.addListener(this, &ofApp::setPressureFieldScale);
    visualizeParameters.add(velocityLineSmooth.set("line smooth", false));
    velocityLineSmooth.addListener(this, &ofApp::setVelocityLineSmooth);
    
    gui.setDefaultHeaderBackgroundColor(guiHeaderColor[guiColorSwitch]);
    gui.setDefaultFillColor(guiFillColor[guiColorSwitch]);
    guiColorSwitch = 1 - guiColorSwitch;
    gui.add(visualizeParameters);
    
    // if the settings file is not present the parameters will not be set during this setup
    if (!ofFile("settings.xml"))
        gui.saveToFile("settings.xml");
    
    gui.loadFromFile("settings.xml");
    
    gui.minimizeAll();
    toggleGuiDraw = true;
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    kinect.update();
    
    deltaTime = ofGetElapsedTimef() - lastTime;
    lastTime = ofGetElapsedTimef();
    
    if (kinect.isFrameNew()) {

        // Load grayscale depth image from the kinect source
        grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
        
        // Threshold image
        threshold(grayImage, grayThreshNear, nearThreshold, true);
        threshold(grayImage, grayThreshFar, farThreshold);
        
        // Convert to CV to perform AND operation
        Mat grayThreshNearMat = toCv(grayThreshNear);
        Mat grayThreshFarMat = toCv(grayThreshFar);
        Mat grayImageMat = toCv(grayImage);
        
        // cvAnd to get the pixels which are a union of the two thresholds
        bitwise_and(grayThreshNearMat, grayThreshFarMat, grayImageMat);
        
        // Save pre-processed image for drawing it
        grayPreprocImage = grayImage;
        
        // Process image
        dilate(grayImage);
        dilate(grayImage);
        //erode(grayImage);
        
        // Mark image as changed
        grayImage.update();
        
        // Find contours
        //contourFinder.setThreshold(ofMap(mouseX, 0, ofGetWidth(), 0, 255));
        contourFinder.findContours(grayImage);
        
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        cameraFbo.begin();
        ofSetColor(ofColor::white);
        if (doFlipCamera)
            kinect.drawDepth(cameraFbo.getWidth(), 0, -cameraFbo.getWidth(), cameraFbo.getHeight());  // Flip Horizontal
        else
            kinect.drawDepth(0, 0, cameraFbo.getWidth(), cameraFbo.getHeight());
        cameraFbo.end();
        ofDisableBlendMode();
        ofPopStyle();
        
        opticalFlow.setSource(cameraFbo.getTexture());
        opticalFlow.update(deltaTime);
        
        velocityMask.setDensity(cameraFbo.getTexture());
        velocityMask.setVelocity(opticalFlow.getOpticalFlow());
        velocityMask.update();
    }
    
    
    fluidSimulation.addVelocity(opticalFlow.getOpticalFlowDecay());
    fluidSimulation.addDensity(velocityMask.getColorMask());
    fluidSimulation.addTemperature(velocityMask.getLuminanceMask());
    
    mouseForces.update(deltaTime);
    
    for (int i=0; i<mouseForces.getNumForces(); i++) {
        if (mouseForces.didChange(i)) {
            switch (mouseForces.getType(i)) {
                case FT_DENSITY:
                    fluidSimulation.addDensity(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
                    break;
                case FT_VELOCITY:
                    fluidSimulation.addVelocity(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
                    particleFlow.addFlowVelocity(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
                    break;
                case FT_TEMPERATURE:
                    fluidSimulation.addTemperature(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
                    break;
                case FT_PRESSURE:
                    fluidSimulation.addPressure(mouseForces.getTextureReference(i), mouseForces.getStrength(i));
                    break;
                case FT_OBSTACLE:
                    fluidSimulation.addTempObstacle(mouseForces.getTextureReference(i));
                default:
                    break;
            }
        }
    }
    
    fluidSimulation.update();
    
    if (particleFlow.isActive()) {
        particleFlow.setSpeed(fluidSimulation.getSpeed());
        particleFlow.setCellSize(fluidSimulation.getCellSize());
        particleFlow.addFlowVelocity(opticalFlow.getOpticalFlow());
        particleFlow.addFluidVelocity(fluidSimulation.getVelocity());
        //		particleFlow.addDensity(fluidSimulation.getDensity());
        particleFlow.setObstacle(fluidSimulation.getObstacle());
    }
    particleFlow.update();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case 'G':
        case 'g': toggleGuiDraw = !toggleGuiDraw; break;
        case 'f':
        case 'F': doFullScreen.set(!doFullScreen.get()); break;
        case 'c':
        case 'C': doDrawCamBackground.set(!doDrawCamBackground.get()); break;
            
        case '1': drawMode.set(DRAW_COMPOSITE); break;
        case '2': drawMode.set(DRAW_FLUID_FIELDS); break;
        case '3': drawMode.set(DRAW_FLUID_VELOCITY); break;
        case '4': drawMode.set(DRAW_FLUID_PRESSURE); break;
        case '5': drawMode.set(DRAW_FLUID_TEMPERATURE); break;
        case '6': drawMode.set(DRAW_OPTICAL_FLOW); break;
        case '7': drawMode.set(DRAW_FLOW_MASK); break;
        case '8': drawMode.set(DRAW_MOUSE); break;
            
        case 'r':
        case 'R':
            fluidSimulation.reset();
            fluidSimulation.addObstacle(obstacleImage.getTexture());
            mouseForces.reset();
            break;
            
        default: break;
    }
}

//--------------------------------------------------------------
void ofApp::drawModeSetName(int &_value) {
    switch(_value) {
        case DRAW_COMPOSITE:		drawName.set("Composite      (1)"); break;
        case DRAW_PARTICLES:		drawName.set("Particles      "); break;
        case DRAW_FLUID_FIELDS:		drawName.set("Fluid Fields   (2)"); break;
        case DRAW_FLUID_DENSITY:	drawName.set("Fluid Density  "); break;
        case DRAW_FLUID_VELOCITY:	drawName.set("Fluid Velocity (3)"); break;
        case DRAW_FLUID_PRESSURE:	drawName.set("Fluid Pressure (4)"); break;
        case DRAW_FLUID_TEMPERATURE:drawName.set("Fld Temperature(5)"); break;
        case DRAW_FLUID_DIVERGENCE: drawName.set("Fld Divergence "); break;
        case DRAW_FLUID_VORTICITY:	drawName.set("Fluid Vorticity"); break;
        case DRAW_FLUID_BUOYANCY:	drawName.set("Fluid Buoyancy "); break;
        case DRAW_FLUID_OBSTACLE:	drawName.set("Fluid Obstacle "); break;
        case DRAW_OPTICAL_FLOW:		drawName.set("Optical Flow   (6)"); break;
        case DRAW_FLOW_MASK:		drawName.set("Flow Mask      (7)"); break;
        case DRAW_SOURCE:			drawName.set("Source         "); break;
        case DRAW_MOUSE:			drawName.set("Left Mouse     (8)"); break;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofClear(0,0);
    if (doDrawCamBackground.get())
        drawSource();
    
    
    if (!toggleGuiDraw) {
        ofHideCursor();
        drawComposite();
    }
    else {
        ofShowCursor();
        switch(drawMode.get()) {
            case DRAW_COMPOSITE: drawComposite(); break;
            case DRAW_PARTICLES: drawParticles(); break;
            case DRAW_FLUID_FIELDS: drawFluidFields(); break;
            case DRAW_FLUID_DENSITY: drawFluidDensity(); break;
            case DRAW_FLUID_VELOCITY: drawFluidVelocity(); break;
            case DRAW_FLUID_PRESSURE: drawFluidPressure(); break;
            case DRAW_FLUID_TEMPERATURE: drawFluidTemperature(); break;
            case DRAW_FLUID_DIVERGENCE: drawFluidDivergence(); break;
            case DRAW_FLUID_VORTICITY: drawFluidVorticity(); break;
            case DRAW_FLUID_BUOYANCY: drawFluidBuoyance(); break;
            case DRAW_FLUID_OBSTACLE: drawFluidObstacle(); break;
            case DRAW_FLOW_MASK: drawMask(); break;
            case DRAW_OPTICAL_FLOW: drawOpticalFlow(); break;
            case DRAW_SOURCE: drawSource(); break;
            case DRAW_MOUSE: drawMouseForces(); break;
        }
        drawGui();
    }
}

//--------------------------------------------------------------
void ofApp::drawComposite(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    fluidSimulation.draw(_x, _y, _width, _height);
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    if (particleFlow.isActive())
        particleFlow.draw(_x, _y, _width, _height);
    
    if (showObstacle) {
        obstacleImage.draw(_x, _y, _width, _height);
    }
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawParticles(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    if (particleFlow.isActive())
        particleFlow.draw(_x, _y, _width, _height);
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidFields(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    pressureField.setPressure(fluidSimulation.getPressure());
    pressureField.draw(_x, _y, _width, _height);
    velocityTemperatureField.setVelocity(fluidSimulation.getVelocity());
    velocityTemperatureField.setTemperature(fluidSimulation.getTemperature());
    velocityTemperatureField.draw(_x, _y, _width, _height);
    temperatureField.setTemperature(fluidSimulation.getTemperature());
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidDensity(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    
    fluidSimulation.draw(_x, _y, _width, _height);
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidVelocity(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    if (showScalar.get()) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        //		ofEnableBlendMode(OF_BLENDMODE_ALPHA); // altenate mode
        displayScalar.setSource(fluidSimulation.getVelocity());
        displayScalar.draw(_x, _y, _width, _height);
    }
    if (showField.get()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        velocityField.setVelocity(fluidSimulation.getVelocity());
        velocityField.draw(_x, _y, _width, _height);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidPressure(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    ofClear(128);
    if (showScalar.get()) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        displayScalar.setSource(fluidSimulation.getPressure());
        displayScalar.draw(_x, _y, _width, _height);
    }
    if (showField.get()) {
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        pressureField.setPressure(fluidSimulation.getPressure());
        pressureField.draw(_x, _y, _width, _height);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidTemperature(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    if (showScalar.get()) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        displayScalar.setSource(fluidSimulation.getTemperature());
        displayScalar.draw(_x, _y, _width, _height);
    }
    if (showField.get()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        temperatureField.setTemperature(fluidSimulation.getTemperature());
        temperatureField.draw(_x, _y, _width, _height);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidDivergence(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    if (showScalar.get()) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        displayScalar.setSource(fluidSimulation.getDivergence());
        displayScalar.draw(_x, _y, _width, _height);
    }
    if (showField.get()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        temperatureField.setTemperature(fluidSimulation.getDivergence());
        temperatureField.draw(_x, _y, _width, _height);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidVorticity(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    if (showScalar.get()) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        displayScalar.setSource(fluidSimulation.getConfinement());
        displayScalar.draw(_x, _y, _width, _height);
    }
    if (showField.get()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(255, 255, 255, 255);
        velocityField.setVelocity(fluidSimulation.getConfinement());
        velocityField.draw(_x, _y, _width, _height);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidBuoyance(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    if (showScalar.get()) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        displayScalar.setSource(fluidSimulation.getSmokeBuoyancy());
        displayScalar.draw(_x, _y, _width, _height);
    }
    if (showField.get()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        velocityField.setVelocity(fluidSimulation.getSmokeBuoyancy());
        velocityField.draw(_x, _y, _width, _height);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawFluidObstacle(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    fluidSimulation.getObstacle().draw(_x, _y, _width, _height);
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawMask(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    velocityMask.draw(_x, _y, _width, _height);
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawOpticalFlow(int _x, int _y, int _width, int _height) {
    
    ofPushStyle();
    //	opticalFlow.getOpticalFlow().draw(_x, _y, _width, _height);
    
    if (showScalar.get()) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        displayScalar.setSource(opticalFlow.getOpticalFlowDecay());
        displayScalar.draw(0, 0, _width, _height);
    }
    if (showField.get()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        velocityField.setVelocity(opticalFlow.getOpticalFlowDecay());
        velocityField.draw(0, 0, _width, _height);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawSource(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    cameraFbo.draw(_x, _y, _width, _height);
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawMouseForces(int _x, int _y, int _width, int _height) {
    ofPushStyle();
    ofClear(0,0);
    
    for(int i=0; i<mouseForces.getNumForces(); i++) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        if (mouseForces.didChange(i)) {
            ftDrawForceType dfType = mouseForces.getType(i);
            if (dfType == FT_DENSITY)
                mouseForces.getTextureReference(i).draw(_x, _y, _width, _height);
        }
    }
    
    for(int i=0; i<mouseForces.getNumForces(); i++) {
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        if (mouseForces.didChange(i)) {
            switch (mouseForces.getType(i)) {
                case FT_VELOCITY:
                    velocityField.setVelocity(mouseForces.getTextureReference(i));
                    velocityField.draw(_x, _y, _width, _height);
                    break;
                case FT_TEMPERATURE:
                    temperatureField.setTemperature(mouseForces.getTextureReference(i));
                    temperatureField.draw(_x, _y, _width, _height);
                    break;
                case FT_PRESSURE:
                    pressureField.setPressure(mouseForces.getTextureReference(i));
                    pressureField.draw(_x, _y, _width, _height);
                    break;
                default:
                    break;
            }
        }
    }
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawGui() {
    guiFPS = (int)(ofGetFrameRate() + 0.5);
    
    // calculate minimum fps
    deltaTimeDeque.push_back(deltaTime);
    
    while (deltaTimeDeque.size() > guiFPS.get())
        deltaTimeDeque.pop_front();
    
    float longestTime = 0;
    for (int i=0; i<deltaTimeDeque.size(); i++){
        if (deltaTimeDeque[i] > longestTime)
            longestTime = deltaTimeDeque[i];
    }
    
    guiMinFPS.set(1.0 / longestTime);
    
    
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    gui.draw();
    
    // HACK TO COMPENSATE FOR DISSAPEARING MOUSE
    ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
    ofDrawCircle(ofGetMouseX(), ofGetMouseY(), ofGetWindowWidth() / 300.0);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofDrawCircle(ofGetMouseX(), ofGetMouseY(), ofGetWindowWidth() / 600.0);
    ofPopStyle();
}