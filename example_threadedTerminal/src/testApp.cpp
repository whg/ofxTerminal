#include "testApp.h"



void testApp::setup(){
	ofSetFrameRate(30);
	
	// Setting up the Terminal
	terminalThread.setup();
}


void testApp::update(){

}


void testApp::draw(){
    ofBackground(255, 255, 255);

	// Draw the Terminal
	terminalThread.draw(0, 0);
	
	if(terminalThread.isThreadRunning() == false){
		ofDrawBitmapStringHighlight("Press '#' to start Terminal", 10, 20, ofColor::black, ofColor::yellow);
	}
}


void testApp::keyPressed(int key){
	//pass the key to the terminal
	terminalThread.keyPressed(key, '#');
}
