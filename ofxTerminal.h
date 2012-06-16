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


#pragma once

#include "ofMain.h"

#include <sstream>

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
	unsigned char color[3];
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
	unsigned char fontcolor[3];
	bool ishidden;
	float characterOffset, spaceOffset;
	bool autocompleteflag;
	
	vector<string> lines, results;
	vector<string> dictionary;
	
	vector< Function<T> > functions;
	
	string PATH; //this is where read finds files when the path doesn't begin with a '/'
	bool readFile(string path);

	int stringWidth(string s);
	
	void process(string command);
	void explode(string command, char sep, vector<string> &tokens);
	string execute(string command);
	
	void println(string line);
	void incrementPrompt();
		
	T *callingObj;
	
public:
	ofxTerminal();
	ofxTerminal(T *co, string fontpath="/System/Library/Fonts/Menlo.ttc", int fontsize=11);
	void setup();
	
	void draw(int xOffset=0, int yOffset=-2);
	void keyPressed(int key);
	
	void addFunction(string name, string(T::*func)(vector<string> args));
	void addToDictionary(string word);
	
	void setPS1(string s);
	void setPath(string s);
	void setCharacterOffset(float v);
	void setSpaceOffset(float v);
	void setBlinkingCursor(bool b, float freq=0.5);
	void setFontColor(int r, int g, int b);
	void setPromptColor(int r, int g, int b);
};


////////////////////////////////////////
/////////// IMPLEMENTATION /////////////
////////////////////////////////////////

template <class T>
ofxTerminal<T>::ofxTerminal(T *co, string fontpath, int fontsize) {
	
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

//empty default constructor, for when ofxTerminal is located on the stack
//this should never be explicitly called.
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
	PATH = "/Users/WHG/Desktop/"; //hardcode your own path here...
	spaceOffset = 0;
	characterOffset = 2;
	blinkCursor = false;
	blinker = true;
	blinkFrequency = 0.5;
	blinkCounter = 0;
	fontcolor[0] = fontcolor[1] = fontcolor[2] = 10;
	prompt.color[0] = prompt.color[1] = prompt.color[2] = 50;
	ishidden = false;
	autocompleteflag = false;
}


template <class T>
void ofxTerminal<T>::draw(int xOffset, int yOffset) {
	
	if (ishidden) return;

	//check to see if we need to move everything up to fit the new line
	if (prompt.y+lineHeight > ofGetHeight() - screenYPos) {
		screenYPos-= lineHeight;
	}

	ofFill();
	ofPushMatrix();
	ofPushStyle();
	ofTranslate(xOffset, screenYPos + yOffset);
	
	//this is where we draw all commands previous and present.
	//not future... yet
	int j = 0;
	for (int i = 0; i < results.size(); i++) {
		if (results[i] == "") {
			ofSetColor(fontcolor[0], fontcolor[1], fontcolor[2]);
			font.drawString(prompt.PS1 + lines[j++], 0, lineHeight*(i+1));
		} 
		//draw results/comments
		else {
			ofSetColor(fontcolor[0]*8, fontcolor[1]*8, fontcolor[2]*8);
			font.drawString(results[i], 0, lineHeight*(i+1));
		}		
	}	
	
	//this is where we draw the prompt
	ofSetColor(prompt.color[0], prompt.color[1], prompt.color[2], blinker ? 100 : 0); //make the prompt a transparent grey
	ofRect(stringWidth(prompt.PS1) + prompt.x, prompt.y, characterWidth, lineHeight);	
	
	ofPopStyle();
	ofPopMatrix();

	//we can't use a modulus for this because we might miss the timing because of
	//the framerate, so it's a little bit awkward, but i can't see how else to do it.
	//...unless the blink is defined in terms of frames... but i prefers seconds
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
		case ' ':
			lines[cl].insert(it+prompt.index, (char) key);
			prompt.x+= (spaceWidth);
			prompt.index++;
			break;
			
		// backspace
		case OF_KEY_BACKSPACE:
			if (prompt.x > 0) {			
				if (lines[cl].at(prompt.index-1) == ' ') prompt.x -= spaceWidth;
				else prompt.x -= characterWidth;
				lines[cl].erase(it+prompt.index-1);
				prompt.index--;
			}
			break;
			
		// left arrow
		case OF_KEY_LEFT:
			if (prompt.x > 0) { // don't go too far...
				if (lines[cl].at(prompt.index-1) == ' ') prompt.x -= spaceWidth;
				else prompt.x -= characterWidth;
				prompt.index--;
			}
			break;
			
		//  right arrow
		case OF_KEY_RIGHT:
			if (prompt.index < lines[cl].length()) { // can't go further than line
				if (lines[cl].at(prompt.index) == ' ') prompt.x += spaceWidth;
				else prompt.x += characterWidth;
				prompt.index++;
			}
			break;
			
		// up arrow - previous command
		case OF_KEY_UP:
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
		case OF_KEY_DOWN	:
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
		case OF_KEY_ESC:
			break;
			
		//tab : autocomplete
		//note: there is no OF_KEY for tab so, can't guarantee 
		//any kind of cross platform compatibility
		case 9: 
		{	
			//auto complete the word we are currently on, 
			//from the cursor to the preceeding space
			string todo = lines[cl].substr(0, prompt.index);
			int lastspace = todo.find_last_of(' ') + 1;
			if (lastspace < 1) lastspace = 0;
			todo = todo.substr(lastspace);
			
			vector<string> foundwords;
			
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
					foundwords.push_back(dictionary[i]);
				}
			}
			
			//if there is only one possibility then change the current line
			if (foundwords.size() == 1) {
				//change the command
				string t = lines[cl].substr(0, lastspace) + foundwords[0];
				
				//only add a space at the end if we are at the end of the line
				if (lines[cl].length() == prompt.index) {
					t+= " ";
				}
				
				//set the new line
				lines[cl] = t + lines[cl].substr(prompt.index);

				prompt.x = stringWidth(t);//move the prompt
				prompt.index = t.length();
			}
			
			//if we have multiple possible commands print them out in a list
			//and then restore the prompt.
			else if (foundwords.size() > 1) {
				if (!autocompleteflag) {
					autocompleteflag = true;
					return;
				}
				stringstream ss;
				for (int i = 0; i < foundwords.size(); i++) {
					ss << foundwords[i] << " ";
				}
				
				//save the current line
				string templine = lines[cl];
				int tempindex = prompt.index;
				int tempx = prompt.x;
				
				//print the commands
				println(ss.str());
				
				//restore the line
				lines[cl] = templine;
				prompt.index = tempindex;
				prompt.x = tempx;
				
				autocompleteflag = false;
			}
			
			
		}
		break;
			
		//enter, do things with current line
		case OF_KEY_RETURN:			
			//process the command
			process(lines[cl]);
			
			break;
		
		//- - - control commands - - -
		//no guarantee of cross platform compatibility, 
		//though I am pretty sure these are standard to ASCII
			
		//control-c, clear the screen
		case 3:			
			//clear screen... but in reality just move things up... similar to clear in UNIX
			screenYPos-= (screenYPos + prompt.y - prompt.yOffset);
			break;
		
		case 22:
			setup();
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
			
		//control-k, clear from point to end of line
		case 11:
			lines[cl] = lines[cl].substr(0, prompt.index);
			break;

		//control-h, toggle ishidden, which hides everything
		case 8:
			ishidden = !ishidden;
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


//my own version of ofTTF stringWidth()... 
//seems to be bit more accurate as i've hardcoded the values (spaceWidth & characterWidth)
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
				
		//execute the command...
		//if your debugger brought you here, you need to return a string from your function
		string comment = execute(command);	  
		
		println(comment);
		return;		
	}
	
	incrementPrompt();
	
	
}


//this is done this way so we can optionally use the returned comment...
template <class T>
string ofxTerminal<T>::execute(string command) {
	
	//split the line up into tokens
	vector<string> tokens;
	explode(command, ' ', tokens);
	
	//if we have an empty line return nothing.. ie empty line
	//this is just a safety precaution, i don't think execute is ever passed an empty string
	if (!tokens.size()) {
		return "";
	}
	
	//the one built in function we have is read.
	//this executes the contents of a specified file.
	else if (tokens[0] == "read" ) {
		if (tokens.size() != 2) {
			return "usage: read filename";
		}
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

	//if we get to this point, we haven't found the command...
	return tokens[0] + ": command not found";
}

/* - - - PROMPT STUFF - - - */
//prints a results line if the argument is not and empty string
//otherwise it just increments the prompt, ie incrementPrompt()

template <class T>
void ofxTerminal<T>::println(string line) {
	//show the comment if there is one and increment prompt.y
	if (line != "") {
		results.push_back(line);
		//need an extra lineHeight added to prompt.y
		prompt.y+= lineHeight;
		
	}
	incrementPrompt();
}


template <class T>
void ofxTerminal<T>::incrementPrompt() {
	//prepare for next line...
	lines.push_back("");
	results.push_back("");
	
	//now do the housework
	cl++;
	prompt.index = 0;
	prompt.x = 0;
	prompt.y+= lineHeight;
	prevCommand = 0;
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

//default freq = 0.5
template <class T>
void ofxTerminal<T>::setBlinkingCursor(bool b, float freq) {	
	blinkCursor = b;
	blinkFrequency = freq;
	blinker = true; //setting this to true is quite important
	blinkCounter = ofGetElapsedTimeMillis();
}

template <class T>
void ofxTerminal<T>::setFontColor(int r, int g, int b) {	
	fontcolor[0] = r;
	fontcolor[1] = g;
	fontcolor[2] = b;
}

template <class T>
void ofxTerminal<T>::setPromptColor(int r, int g, int b) {	
	prompt.color[0] = r;
	prompt.color[1] = g;
	prompt.color[2] = b;
}
