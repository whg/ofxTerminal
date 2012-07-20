#pragma once

#include "ofMain.h"
#include "TerminalThread.h"



class testApp : public ofBaseApp {

	public:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);

	
	private:
		TerminalThread terminalThread;
	
};
