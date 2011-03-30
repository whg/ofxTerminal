#ifndef _OF_TERMINAL
#define _OF_TERMINAL


#include "ofMain.h"

class ofTerminal : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	
	ofTrueTypeFont font;
	int lineHeight;
	int characterWidth, spaceWidth;
	int stringWidth;
	
	ofPoint prompt;
	int promptIndex;
	string promptString;
	int basicWidth;
	
	int prevCommand;
	string tempCommand;
	
	vector<string> lines;
	vector<string> results;
	
	vector<string> commands;
	void setCommands(vector<string> &c);
	
	int cl; //current line
	
	void process(string command);
	void explode(string command, char sep, vector<string> &tokens);
		
};

#endif
