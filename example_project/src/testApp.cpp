#include "testApp.h"

void testApp::setup(){

	ofSetFrameRate(30);
	counter = 0;
	frequency = 1;
	length = 360;
	amplitude = 50;
	speed = 0.1;
	
	terminal = ofxTerminal<testApp>(this);
	
	terminal.addFunction("frequency", &testApp::setFrequency);
	terminal.addFunction("amplitude", &testApp::setAmplitude);
	terminal.addFunction("length", &testApp::setLength);
	terminal.addFunction("speed", &testApp::setSpeed);
	terminal.addFunction("blink", &testApp::blink);
	terminal.addFunction("ps1", &testApp::setPS1);
	
}

void testApp::update(){

}

void testApp::draw(){
    ofBackground(255, 255, 255);

	//we have to draw the terminal
	terminal.draw(0, 0);
	
	ofFill();
	ofSetColor(0, 0, 0);
	
	
	//we are going to draw a sine wave,
	//the user can change the frequency, amplitude and length
	ofPushMatrix();
	
	//translate to wave in in the center
	ofTranslate(ofGetWidth()*0.5-(length*0.5), ofGetHeight()*0.5);
	
	for (int i = 0; i < length; i++) {
		float y = sin((i+counter)/TWO_PI * frequency) * amplitude;
		ofCircle(i, y, 2);
	}
	counter+= speed;
	
	ofPopMatrix();
	
}

void testApp::keyPressed(int key){

	//pass the key to the terminal
	terminal.keyPressed(key);
}

string testApp::setFrequency(vector<string> args) {
	
	if (args.size() < 1) {
		return "usage: frequency f";
	}

	frequency = ofToFloat(args[0]);
	return "";
}

// here we have an example where the user can reveive feedback based on the input
string testApp::setAmplitude(vector<string> args) {
	
	//ofToFloat returns 0 if something other than a number is passed to it,
	//so alert the user...
	if (!ofToFloat(args[0])) {
		return args[0] + " is not a number";
	}
	
	amplitude = ofToFloat(args[0]);

	return "";
}

string testApp::setLength(vector<string> args) {
	
	length = ofToInt(args[0]);
	return "";
}

string testApp::setSpeed(vector<string> args) {
	
	speed = ofToFloat(args[0]);	
	return "";
}

string testApp::blink(vector<string> args) {

    if (args.size() != 1) {
        return "usage: blink on|off";
    }
    
	if (args[0] == "on") {
		terminal.setBlinkingCursor(true);
	}
	else if (args[0] == "off") {
		terminal.setBlinkingCursor(false);
	}
	else {
		return "don't understand " + args[0];
	}

	return "";
}

string testApp::setPS1(vector<string> args) {

	terminal.setPS1(args[0]);
	return "";
}