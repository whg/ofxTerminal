/*
 * ofxTerminal.h
 * 
 * Copyright 2011 Will Gallia, wgallia.com
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


#ifndef _OFX_TERMINAL
#define _OFX_TERMINAL

#include "ofMain.h"

template <class T>
class Function {
public:
	Function<T>(string n, string(T::*f)(vector<string> args)) { 
		name = n; 
		func = f; 
	};
	
	string name;
	string(T::*func)(vector<string> args);
};

//this holds all the prompt data...
typedef struct {
	int x, y, yOffset;
	int index;
	string PS1;
} Prompt;


template <class T>
class ofxTerminal {
	
private:
	Prompt prompt;
	
	ofTrueTypeFont font;
	int lineHeight;
	float characterWidth, spaceWidth;
	int screenYPos, prevCommand;
	string tempCommand;
	int cl; //current line
	bool blinkCursor, blinker;
	float blinkFrequency;
	int blinkCounter;
	
	float characterOffset, spaceOffset;
	
	vector<string> lines, results;
	vector<string> dictionary;
	
	vector< Function<T> > functions;
	
	string PATH; //this is where read finds files when the path doesn't begin with a '/'
	bool readFile(string path);

	int stringWidth(string s);
	
	void process(string command);
	void explode(string command, char sep, vector<string> &tokens);
	string execute(string command);
		
	T *callingObj;
	
public:
	ofxTerminal();
	ofxTerminal(T *co, string fontpath, int fontsize);
	void setup();
	
	void draw(int xOffset, int yOffset);
	void update();
	void keyPressed(int key);
	
	void addFunction(string name, string(T::*func)(vector<string> args));
	void addToDictionary(string word);
	
	void setPS1(string s);
	void setPath(string s);
	void setCharacterOffset(float v);
	void setSpaceOffset(float v);
	void setBlinkingCursor(bool b, float freq);
};


////////////////////////////////////////
/////////// IMPLEMENTATION /////////////
////////////////////////////////////////

template <class T>
ofxTerminal<T>::ofxTerminal(T *co, string fontpath="/System/Library/Fonts/Menlo.ttc", int fontsize=11) {
	
	callingObj = co;
	
	setup();
	
	font.loadFont(fontpath, fontsize);
	//make this an int so we get rid of little errors
	lineHeight = (int) font.getLineHeight();
	//this is a bit silly... but seems to work
	spaceWidth = font.stringWidth("a") + spaceOffset;
	characterWidth = font.stringWidth("a") + characterOffset;

	//add the only built in function
	addToDictionary("read");
	setPS1("? "); //this is the default prompt
	
	ofEnableAlphaBlending();
}

//empty default constructor, should never be used
template <class T> ofxTerminal<T>::ofxTerminal() {}

template <class T>
void ofxTerminal<T>::setup() {

	lines.clear();
	results.clear();
	cl = 0;
	prompt.yOffset = 2;
	prompt.x = 0;
	prompt.y = prompt.yOffset;
	prompt.index = 0;
	lines.push_back("");
	results.push_back("");
	screenYPos = 0;
	prevCommand = 0;
	PATH = "/Users/WHG/Desktop/";
	spaceOffset = 0;
	characterOffset = 2;
	blinkCursor = false;
	blinker = true;
	blinkFrequency = 0.5;
	blinkCounter = 0;
}


template <class T>
void ofxTerminal<T>::draw(int xOffset=0, int yOffset=-2) {
		
	//check to see if we need to move everything up to fit the new line
	if (prompt.y+lineHeight > ofGetHeight() - screenYPos) {
		screenYPos-= lineHeight;
	}

	ofFill();
	ofPushMatrix();
	ofTranslate(xOffset, screenYPos + yOffset);
	
	//this is where we draw all commands previous and present.
	//not future... yet
	int j = 0;
	for (int i = 0; i < results.size(); i++) {
		if (results[i] == "") {
			ofSetColor(10, 10, 10);
			font.drawString(prompt.PS1 + lines[j++], 0, lineHeight*(i+1));
		} 
		//draw results/comments
		else {
			ofSetColor(100, 100, 100);
			font.drawString(results[i], 0, lineHeight*(i+1));
		}		
	}	
	cout << ofGetElapsedTimeMillis() << endl;
	
	//this is where we draw the prompt
	ofSetColor(50, 50, 50, blinker ? 100 : 0); //make the prompt a transparent grey
	ofRect(stringWidth(prompt.PS1) + prompt.x, prompt.y, characterWidth, lineHeight);	
	
	ofPopMatrix();
	
	if (blinkCursor && ofGetElapsedTimeMillis() > blinkFrequency*1000+blinkCounter) {
		blinker = !blinker;
		blinkCounter+= blinkFrequency*1000;
	}
}


/* - - - KEY ACTIONS - - - - - - - - - 
 
 this is where all the key presses are interpreted
 i am trying to copy the bash shell, so at the moment,
 we are implementing:
 
 - up and down for previous commands 
 - left and right to move along the line
 - tab to autocomplete
 - backspace to delete characters
 - enter to process command
 
 - - - - - - - - - - - - - - - - - - - */

template <class T>
void ofxTerminal<T>::keyPressed(int key) {
	
	string::iterator it = lines[cl].begin();
	int x;
	
	switch (key) {
			
		//space
		case 32:
			lines[cl].insert(it+prompt.index, (char) key);
			prompt.x+= (spaceWidth);
			prompt.index++;
			break;
			
		// backspace
		case 127:
			if (prompt.x > 0) {			
				if (lines[cl].at(prompt.index-1) == ' ') prompt.x -= spaceWidth;
				else prompt.x -= characterWidth;
				lines[cl].erase(it+prompt.index-1);
				prompt.index--;
			}
			break;
			
		// left arrow
		case 356:
			if (prompt.x > 0) { // don't go too far...
				if (lines[cl].at(prompt.index-1) == ' ') prompt.x -= spaceWidth;
				else prompt.x -= characterWidth;
				prompt.index--;
			}
			break;
			
		//  right arrow
		case 358:
			if (prompt.index < lines[cl].length()) { // can't go further than line
				if (lines[cl].at(prompt.index) == ' ') prompt.x += spaceWidth;
				else prompt.x += characterWidth;
				prompt.index++;
			}
			break;
			
		// up arrow - previous command
		case 357:
			//save the current command
			if (prevCommand == 0) {
				tempCommand = lines[cl];
			}
			//fetch previous commands
			if (prevCommand < cl) {
				prevCommand++;
				lines[cl] = lines[cl-prevCommand];
				prompt.x = stringWidth(lines[cl]); //move the prompt
				prompt.index = lines[cl].length();
			}
			break;
			
		// down arrow
		case 359:
			if (prevCommand > 0) {
				prevCommand--;
				lines[cl] = lines[cl-prevCommand];
			}
			//get the saved command
			if (prevCommand == 0) {
				lines[cl] = tempCommand;
			}
			prompt.x = stringWidth(lines[cl]); //move the prompt
			prompt.index = lines[cl].length();
			break;
			
		// esc, do nothing
		case 27:
			break;
			
		//tab : autocomplete
		case 9: 
		{	
			//auto complete the word we are currently on, 
			//from the cursor to the preceeding space
			string todo = lines[cl].substr(0, prompt.index);
			int lastspace = todo.find_last_of(' ') + 1;
			if (lastspace < 1) lastspace = 0;
			todo = todo.substr(lastspace);
			
			//before any uber computer scientist has a go at me, 
			//i don't think we are going to have a large dictionary so
			//i think a linear search is good enough...
			for (int i = 0; i < dictionary.size(); i++) {
				bool show = true;
				for (int j = 0; j < todo.length() && j < dictionary[i].length(); j++) {
					if (dictionary[i].at(j) != todo.at(j)) {
						show = false;
						break;
					}
				}
				if (show) {
					//change the command
					string t = lines[cl].substr(0, lastspace) + dictionary[i] + " ";
					lines[cl] = t + lines[cl].substr(prompt.index);
					//add a space on to the end of the command... bash style
					prompt.x = stringWidth(t);//move the prompt
					prompt.index = t.length();
					break;
				}
			}
		}
		break;
			
		//enter, do things with current line
		case 13:			
			//process the command
			process(lines[cl]);
			
			//now do the housework
			cl++;
			prompt.index = 0;
			prompt.x = 0;
			prompt.y+= lineHeight;
			prevCommand = 0;
			break;
			
		//control-c, clear the screen
		case 3:			
			//clear screen... but in reality just move things up... similar to clear in UNIX
			screenYPos-= (screenYPos + prompt.y - prompt.yOffset);
			break;

		//control-a, beginning of line
		case 1:
			prompt.x = 0;
			prompt.index = 0;
			break;
			
		//control-e, end of line
		case 5:
			prompt.index = lines[cl].length();
			prompt.x = stringWidth(lines[cl]) ;
			break;
		
		//control-u, clear the current line
		case 21:
			prompt.x = 0;
			prompt.index = 0;
			lines[cl] = "";
			break;

		//all other keys...
		//hopefully ofTrueTypeFont can draw them...
		default:
			if (key < 31) return;
			
			lines[cl].insert(it+prompt.index, (char) key);
			prompt.x+= characterWidth;
			prompt.index++;
			break;
			
	}
}


//my own version of ofTTF stringWidth()... seems to be bit more accurate as i've hardcoded the values
template <class T>
int ofxTerminal<T>::stringWidth(string s) {
	int x = 0;
	for (int i = 0; i < s.length(); i++) {
		if (s.at(i) == ' ') x+= spaceWidth;
		else x+= characterWidth;
	}	
	return x;
}

/* - - -  PROCESS - - - - - - - - */

template <class T>
void ofxTerminal<T>::process(string command) {
	
	//don't try and process an empty line
	if (command != "") {
		
		string comment = "";
		
		//execute the command...
		//if your debugger brought you here, you need to return a string from your function
		comment = execute(command);	  
		
		//show the comment if there is one and increment prompt.y
		if (comment != "") {
			results.push_back(comment);
			//need an extra lineHeight added to prompt.y
			prompt.y+= lineHeight;
		}
		
	}
	
	//prepare for next line...
	lines.push_back("");
	results.push_back("");
	
}


//this is done this way so we can optionally use the returned comment...
template <class T>
string ofxTerminal<T>::execute(string command) {
	
	//split the line up into tokens
	vector<string> tokens;
	explode(command, ' ', tokens);
	
	//if we have an empty line return nothing.. ie empty line
	if (!tokens.size()) {
		return "";
	}
	
	//the one built in function we have is read.
	//this executes the contents of a specified file.
	else if (tokens[0] == "read" && tokens.size() == 2) {
		if (readFile(tokens[1])) {
			return "";	
		}
		else {
			return tokens[1] + ": can't read file";
		}
	}
	
	//now try and find a valid command...
	//... obviously there are much faster ways of doing this but
	//i don't think a user will have many functions, so i think it's ok
	for (int i = 0; i < functions.size(); i++) {
		if (tokens[0] == functions[i].name) {
			tokens.erase(tokens.begin());
			return ((callingObj)->*(functions[i].func))(tokens);
		}
	}

	return tokens[0] + ": command not found";
}

/* - - - ADDING STUFF - - - */

template <class T>
void ofxTerminal<T>::addToDictionary(string word) {
	dictionary.push_back(word);
}

template <class T>
void ofxTerminal<T>::addFunction(string name, string (T::*func)(vector<string> args)) {
	functions.push_back(Function<T>(name, func));
	addToDictionary(name);
}

//this is a helper method, inspired by PHP's explode
//note: realised after doing this that there are methods in c++ to do this for you... stupid me...

//it takes a string and splits the separate words into tokens,
//extra white space is removed in the process
//string are preserved if inside single quotes, ie '
//if you want to use a ' you have to escape it like \'

template <class T>
void ofxTerminal<T>::explode(string command, char sep, vector<string> &tokens) {
	tokens.clear();
	bool inquotes = false;
	bool escape = false;
	
	string t = "";
	for (int i = 0; i < command.length(); i++) {
		if (command[i] == '\\') {
			escape = true;
			continue;
		}
		if (command[i] != sep || inquotes) {
			if (command[i] == '\'' && !escape) {
				inquotes = !inquotes;
			}
			else {
				t+= command[i];
			}
		}
		else if (!inquotes) {
			//don't add things starting with a space..
			//or an empty string...
			if (t[0] != ' ' && t != "") {
				tokens.push_back(t);
				t = "";
			}
		}
		escape = false;
	}
	
	// add the final one...
	// but don't add it if it's a space... or nothing
	if (t != " " && t != "") {
		tokens.push_back(t);
	}
}

/* - - - FILE HANDLING - - - */

//returns true if file could be read and executed, false otherwise
template <class T>
bool ofxTerminal<T>::readFile(string path) {
	
	//set up stream
	ifstream file;
	
	if (path.at(0) == '/') {
		file.open(path.c_str());
	}
	
	//if the path is not absolute look in specified directory
	else {
		string p = PATH + path;
		file.open(p.c_str());
	}
	
	//check to see something is open
	if (file.is_open()) {
		
		while (file.good()) {
			string line;
			getline(file, line);
			
			//we execute here instead of process so we don't blank lines printed
			execute(line);
		}
		
		//and close...
		file.close();
		
		return true;
	}
	
	return false;
}

/* - - - USER SETTINGS/SETTERS - - - */

template <class T>
void ofxTerminal<T>::setPS1(string s) {	
	prompt.PS1 = s;
}

template <class T>
void ofxTerminal<T>::setPath(string s) {	
	PATH = s;
}

template <class T>
void ofxTerminal<T>::setCharacterOffset(float v) {	
	characterOffset = v;
}

template <class T>
void ofxTerminal<T>::setSpaceOffset(float v) {	
	spaceOffset = v;
}

template <class T>
void ofxTerminal<T>::setBlinkingCursor(bool b, float freq=0.5) {	
	blinkCursor = b;
	blinkFrequency = freq;
	blinker = true; //setting this to true is quite important
	blinkCounter = ofGetElapsedTimeMillis();
}

#endif
