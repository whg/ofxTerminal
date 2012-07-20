/*
 * ThreadedTerminal.h
 * 
 * Copyright 2011 Paul Vollmer, wrong-entertainment.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _THREADEDTERMINAL
#define _THREADEDTERMINAL

#include "ofxTerminal.h"



class TerminalThread : public ofThread {
	
	public:
	
		ofxTerminal<TerminalThread> terminal;
	
		float counter, speed;
		int length;
		float frequency, amplitude;
	
	
	
		/**
		 * Constructor
		 */
		TerminalThread(){}
	
		void start(){
			startThread(true, false);   // blocking, verbose
		}
	
		void stop(){
			stopThread();
		}
	
		void threadedFunction(){
			while( isThreadRunning() != 0 ){
				if( lock() ){
					unlock();
					ofSleepMillis(1 * 1000);
				}
			}
		}
	
	
	
		/**
		 * setup
		 */
		void setup(){
			terminal = ofxTerminal<TerminalThread>(this);
			
			counter = 0;
			frequency = 1;
			length = 360;
			amplitude = 50;
			speed = 0.1;
			
			terminal.addFunction("frequency", &TerminalThread::setFrequency);
			terminal.addFunction("amplitude", &TerminalThread::setAmplitude);
			terminal.addFunction("length", &TerminalThread::setLength);
			terminal.addFunction("speed", &TerminalThread::setSpeed);
			terminal.addFunction("blink", &TerminalThread::blink);
			terminal.addFunction("ps1", &TerminalThread::setPS1);
		}
	
	
	
		/**
		 * Draw
		 */
		void draw(int x, int y){
			if(lock()){
			
				if(isThreadRunning() == true){
					//we have to draw the terminal
					terminal.draw(0, 0);
				}
			
				unlock();
			} else {
				cout << "unlock() can't lock!\neither an error\nor the thread has stopped" << endl;
			}
			
			
			// NOTE: At the moment this is not the final solution.
			// I think the sine wave need to moved to testApp::draw()...
			// Solution: Using a Singleton to manipulate the vars (speed, frequency, amplitude etc.)
			ofFill();
			ofSetColor(0, 0, 0);
			//we are going to draw a sine wave,
			//the user can change the frequency, amplitude and length
			ofPushMatrix();
			//translate to wave in in the center
			ofTranslate(ofGetWidth()*0.5-(length*0.5), ofGetHeight()*0.5);
			
			ofEnableSmoothing();
			for (int i = 0; i < length; i++) {
				float y = sin((i+counter)/TWO_PI * frequency) * amplitude;
				ofCircle(i, y, 2);
			}
			ofDisableSmoothing();
			
			counter+= speed;
			ofPopMatrix();
		}
	
	
	
		/**
		 * keyPressed
		 */
		void keyPressed(int key, int shortcutToRunTerminal){
			if(key == shortcutToRunTerminal){
				if(isThreadRunning() == true){
					stop();
					terminal.keyPressed(3);
					//cout << "STOP" << endl;
				} else {
					start();
					//cout << "START" << endl;
				}
			}
			
			// if our thread is running and the pressed key is not '#'...
			if (isThreadRunning() == true && key != '#') {
				//pass the key to the terminal
				terminal.keyPressed(key);
			}
		}
	
	
	
	
		string setFrequency(vector<string> args){
			if(args.size() < 1){
				return "usage: frequency f";
			}
			frequency = ofToFloat(args[0]);
			return "";
		}
	
	
		// here we have an example where the user can reveive feedback based on the input
		string setAmplitude(vector<string> args){
			//ofToFloat returns 0 if something other than a number is passed to it,
			//so alert the user...
			if (!ofToFloat(args[0])) {
				return args[0] + " is not a number";
			}
			amplitude = ofToFloat(args[0]);
			return "";
		}

	
		string setLength(vector<string> args){
			length = ofToInt(args[0]);
			return "";
		}
	
	
		string setSpeed(vector<string> args){
			speed = ofToFloat(args[0]);	
			return "";
		}
	
	
		string blink(vector<string> args){
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
	
	
		string setPS1(vector<string> args) {
			terminal.setPS1(args[0]);
			return "";
		}
	
	
};

#endif
