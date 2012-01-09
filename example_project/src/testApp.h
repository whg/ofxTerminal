#pragma once

#include "ofMain.h"
#include "ofxTerminal.h"

// usage:
// you can call:
// - amplitude
// - frequency
// - speed
// - length
//
// with some kind of value


class testApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);

	ofxTerminal<testApp> terminal;
	
	string setFrequency(vector<string> args);
	string setAmplitude(vector<string> args);
	string setLength(vector<string> args);
	string setSpeed(vector<string> args);
	
	string blink(vector<string> args);
	string setPS1(vector<string> args);
	
	float counter, speed;
	int length;
	float frequency, amplitude;
	
};
