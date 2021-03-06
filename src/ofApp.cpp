#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // General project setup. Title, resolution, framerate and background.
    ofSetWindowTitle("Video synth");
    ofSetWindowShape(1280, 720);
    ofSetFrameRate(60);
    ofBackground(ofColor::white);
    
    // General GUI setup. Load settings and auto-show GUI.
    showGui = true;
    gui.setup( "Parameters", "settings.xml" );
    
    // GUI for controlling actual drawing of vector graphic.
    gui.add( countX.setup( "countX", 50, 0, 200) );
    gui.add( stepX.setup( "stepX", 20, 0, 200 ) );
    gui.add( twistX.setup( "twistX", 5, -45, 45 ) );
    
    gui.add(countY.setup("countY", 0, 0, 50));
    gui.add(stepY.setup("stepY", 20, 0, 200));
    gui.add(twistY.setup("twistY", 0, -30, 30));
    gui.add(pinchY.setup("pinchY", 0, 0, 1));
    
    // Global GUI. Controls scale, rotation and background of vector drawing.
    globalGroup.setup("Global");
    globalGroup.add(Scale.setup("Scale", 1, 0.0, 1));
    globalGroup.add(Rotate.setup("Rotate", 0, -180, 180));
    globalGroup.add(Background.setup("Background", 255, 0, 255));
    gui.add(&globalGroup);
    
    // Primitive GUI. Controls shiftY, rotation, size, color, isFilled and type of vector drawing.
    primGroup.setup("Primitive");
    primGroup.add(shiftY.setup("shiftY", 0.0, -1000.0, 1000.0));
    primGroup.add(rotate.setup("rotate", 0.0, -180, 180));
    primGroup.add(size.setup("size",
                             ofVec2f(6,6),
                             ofVec2f(0,0),
                             ofVec2f(20,20)
                             ));
    primGroup.add(color.setup("color",
                              ofColor::black,
                              ofColor(0,0,0,0),
                              ofColor::white
                              ));
    primGroup.add(filled.setup("filled", false));
    primGroup.add(type.setup("type", false));
    gui.add(&primGroup);
    
    // Mixer GUI. Can set alpha level to image, video and camera.
    mixerGroup.setup("Mixer");
    mixerGroup.setHeaderBackgroundColor(ofColor::darkRed);
    mixerGroup.setBorderColor(ofColor::darkRed);
    mixerGroup.add(imageAlpha.setup("image", 100, 0, 255));
    mixerGroup.add(videoAlpha.setup("video", 200, 0, 255));
    mixerGroup.add(cameraAlpha.setup("camera", 100, 0, 255));
    gui.add(&mixerGroup);
    
    shader.load( "kaleido" );
    mixerGroup.add( kenabled.setup( "kenabled", true ) );
    mixerGroup.add( ksectors.setup( "ksectors", 10, 1, 100 ) );
    mixerGroup.add( kangle.setup( "kangle", 0, -180, 180 ) );
    mixerGroup.add( kx.setup( "kx", 0.5, 0, 1 ) );
    mixerGroup.add( ky.setup( "ky", 0.5, 0, 1 ) );
    
    gui.minimizeAll();
    gui.loadFromFile("settings.xml");
    
    ofLoadImage(image, "collage.png");
    video.load("flowing.mp4");
    video.play();
    
    // Frame Buffer Object initialaztion.
    fbo.allocate( ofGetWidth(), ofGetHeight(), GL_RGB );
    
    // Sphere
    sphere.set(250, 20);
    fbo2.allocate( ofGetWidth(), ofGetHeight(), GL_RGB );
    float w = fbo2.getWidth();
    float h = fbo2.getHeight();
    sphere.mapTexCoords(0, h, w, 0);
    sphere.rotate(180, 0, 1, 0);
    sphere.setGlobalPosition( ofGetWidth()/2, ofGetHeight()/2, 0 );
}

//--------------------------------------------------------------
void ofApp::update(){
    video.update();
    
    if (camera.isInitialized()) camera.update();
}

//--------------------------------------------------------------
void ofApp::matrixPattern() {
    for (int y=-countY; y<=countY; y++) {
        ofPushMatrix();
        // ----------------
        if (countY > 0) {
            float scl = ofMap(y, -countY, countY, 1-pinchY, 1);
            ofScale(scl, scl);
        }
        ofTranslate(0, y * stepY);
        ofRotateDeg(y * twistY);
        stripePattern();
        // ----------------
        ofPopMatrix();
    }
}

//--------------------------------------------------------------
void ofApp::stripePattern() {
    ofSetColor(color);
    ofSetLineWidth(1);
    if (filled) ofFill(); else ofNoFill();
    for (int i=-countX; i<countX; i++) {
        ofPushMatrix();
        ofTranslate(i*stepX, 0);
        ofRotateDeg(i * twistX);
        
        ofTranslate(0, shiftY);
        ofRotateDeg(rotate);
        ofScale(size->x, size->y);
        if (type) ofDrawRectangle(-50, -50, 100, 100);
        else ofDrawTriangle(0, 0, -50, 100, 50, 100);
        
        ofPopMatrix();
    }
}
//--------------------------------------------------------------
void ofApp::draw() {
    
    fbo.begin();
    draw2d();
    fbo.end();
    ofSetColor( 255 );
    
    fbo2.begin();
    // Draw shader.
    if ( kenabled ) {
        shader.begin();
        shader.setUniform1i( "ksectors", ksectors );
        shader.setUniform1f( "kangleRad", ofDegToRad(kangle) );
        shader.setUniform2f( "kcenter", kx*ofGetWidth(),
                            ky*ofGetHeight() );
        shader.setUniform2f( "screenCenter", 0.5*ofGetWidth(),
                            0.5*ofGetHeight() );
    }
    
    fbo.draw( 0, 0, ofGetWidth(), ofGetHeight() );
    
    if ( kenabled ) shader.end();
    fbo2.end();
    
    draw3d();
    
    // GUI toggle. Bound to z-key
    if (showGui) gui.draw();
}

//--------------------------------------------------------------
void ofApp::draw2d(){
    ofBackground(Background);
    
    // Enable additive blending mode and disable smoothing.
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofDisableSmoothing();
    
    // Image draw
    ofSetColor(255, imageAlpha);
    image.draw(0, 0, ofGetWidth(), ofGetHeight());
    // Video draw
    ofSetColor(255, videoAlpha);
    video.draw(0, 0, ofGetWidth(), ofGetHeight());
    // Camera draw
    if (camera.isInitialized()) {
        ofSetColor(255, cameraAlpha);
        camera.draw(0, 0, ofGetHeight(), ofGetHeight());
    }
    // Switch to Alpha blending  (from additive) and enable smoothing.
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    

    
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);
    float Scl = pow(Scale, 4.0f);
    ofScale(Scl, Scl);
    ofRotateDeg(Rotate);
    matrixPattern();
    
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::draw3d() {
    fbo2.getTextureReference().bind();
    light.setPosition( ofGetWidth()/2, ofGetHeight()/2, 600);
    light.enable();
    material.begin();
    ofEnableDepthTest();
    
    float time = ofGetElapsedTimef();
    float longitude = 10 * time;
    float latitude = 10 * sin(time*0.8);
    float radius = 600 + 50*sin(time*0.4);
    
    cam.orbit( longitude, latitude, radius, ofPoint(0, 0, 0) );
    
    cam.begin();
    ofSetColor( ofColor::white );
    sphere.draw();
    cam.end();
    
    ofDisableDepthTest();
    material.end();
    light.disable();
    ofDisableLighting();
    fbo2.getTextureReference().unbind();
}

//--------------------------------------------------------------
void ofApp::exit() {
    gui.saveToFile( "settings.xml" );
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'z') showGui = !showGui;
    if (key == OF_KEY_RETURN) ofSaveScreen("screenshot.png");
    if (key == 's') {
        ofFileDialogResult res;
        res = ofSystemSaveDialog("preset.xml", "Saving Preset");
        if (res.bSuccess) gui.saveToFile(res.filePath);
    }
    
    if (key == 'l') {
        ofFileDialogResult res;
        res = ofSystemLoadDialog("Loading Preset");
        if (res.bSuccess) gui.loadFromFile(res.filePath);
    }
    
    if (key == 'c') {
        camera.setDeviceID(0);
        camera.setDesiredFrameRate(30);
        camera.initGrabber(1280, 720);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    if ( showGui && x < 250 ) cam.disableMouseInput();
    else cam.enableMouseInput();
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

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
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
