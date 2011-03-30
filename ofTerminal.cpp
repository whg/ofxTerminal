#include "ofTerminal.h"

//--------------------------------------------------------------
void ofTerminal::setup(){
	ofSetFrameRate(25);
	ofDisableSmoothing();
	ofSetBackgroundAuto(true);
	ofBackground(255, 255, 255);
	ofSetColor(30, 30, 30);
	font.loadFont("/Library/Fonts/Andale Mono.ttf", 12);
	lineHeight = (int) font.getLineHeight();
	characterWidth = (int) font.stringWidth("a") - 3; //this is the same for all... monospace
	spaceWidth = characterWidth-1;
	printf("characterWidth = %i\n", characterWidth);
	cl = 0;
	promptString = "sybil:$ ";
	lines.push_back("");
	results.push_back("");
	stringWidth = characterWidth*8;
	
	prompt = ofPoint(0, 4);
	promptIndex = 0;
	basicWidth = stringWidth;
	prevCommand = 0;
	
	setCommands(commands);
}

//--------------------------------------------------------------
void ofTerminal::update(){
}

//--------------------------------------------------------------
void ofTerminal::draw(){
	
	ofNoFill();
//	ofRect(0, mouseY, characterWidth, lineHeight);
//	printf("mouse Y = %i\n", mouseY);
	
	ofFill();
	
	
	int j = 0;
	for (int i = 0; i < results.size(); i++) {
		if (results[i] == "") {
			ofSetColor(10, 10, 10);
			font.drawString(promptString + lines[j], 0, lineHeight*(i+1));
			j++;
		} else {
			ofSetColor(100, 100, 100);
			font.drawString(results[i], 0, lineHeight*(i+1));

		}

	}
	
	
	ofPushMatrix();
	ofTranslate(characterWidth*promptString.length(), 0);
	ofSetColor(100, 100, 100);
	ofRect(prompt.x, prompt.y, characterWidth, lineHeight);
	
	ofPopMatrix();
}


//--------------------------------------------------------------
void ofTerminal::keyPressed(int key){

	printf("key = %i\n", key);
	//printf("prev command before = %i\n", prevCommand);
	//lines[cl] += (char) key;
	
	string::iterator it = lines[cl].begin();
	
	//int p =  ceil(prompt.x/characterWidth);
	//printf("p = %i\n", p);
	
	//space
	if (key == 32) {
		lines[cl].insert(it+promptIndex, (char) key);
		prompt.x+= (spaceWidth);
		promptIndex++;
	}
	
	// backspace
	else if (key == 127) {
		if (prompt.x > 0) {			
			if (lines[cl].at(promptIndex-1) == ' ') prompt.x -= spaceWidth;
			else prompt.x -= characterWidth;
			lines[cl].erase(it+promptIndex-1);
			promptIndex--;
		}
	}
	
	// left arrow
	else if (key == 356) {
		if (prompt.x > 0) { // don't go too far...
			if (lines[cl].at(promptIndex-1) == ' ') prompt.x -= spaceWidth;
			else prompt.x -= characterWidth;
			promptIndex--;
		}
	}
	
	//  right arrow
	else if (key == 358) {
		if (promptIndex < lines[cl].length()) { // can't go further than line
			if (lines[cl].at(promptIndex) == ' ') prompt.x += spaceWidth;
			else prompt.x += characterWidth;
			promptIndex++;
		}
	}
	
	// up arrow
	else if (key == 357) {
		//save the current command
		if (prevCommand == 0) {
			tempCommand = lines[cl];
		}
		//fetch previous commands
		if (prevCommand < cl) {
			prevCommand++;
			lines[cl] = lines[cl-prevCommand];
			//now move prompt
			int x = 0;
			for (int i = 0; i < lines[cl].length(); i++) {
				if (lines[cl].at(i) == ' ') x+= spaceWidth;
				else x+= characterWidth;
			}
			prompt.x = x;
			promptIndex = lines[cl].length();
		}
	}
	
	// down arrow
	else if (key == 359) { 
		if (prevCommand > 0) {
			prevCommand--;
			lines[cl] = lines[cl-prevCommand];
		}
		//get the saved command
		if (prevCommand == 0) {
			lines[cl] = tempCommand;
		}
		//now move prompt
		int x = 0;
		for (int i = 0; i < lines[cl].length(); i++) {
			if (lines[cl].at(i) == ' ') x+= spaceWidth;
			else x+= characterWidth;
		}
		prompt.x = x;
		promptIndex = lines[cl].length();
	}
	
	// esc, do nothing
	else if (key == 27) { }
	
	//tab, do autocomplete
	else if (key == 9) {
		for (int i = 0; i < commands.size(); i++) {
			bool show = false;
			for (int j = 0; j < lines[cl].length(); j++) {
				if (commands[i].at(j) == lines[cl].at(j)) {
					show = true;
				} else {
					show = false;
					break;
				}
			}
			if (show) {
				//change the command
				lines[cl] = commands[i];
				//move the prompt
				int x = 0;
				for (int i = 0; i < lines[cl].length(); i++) {
					if (lines[cl].at(i) == ' ') x+= spaceWidth;
					else x+= characterWidth;
				}
				prompt.x = x;
				promptIndex = lines[cl].length();
			}
		}
	}
	
	//enter, do things with current line
	else if (key == 13) { 
		cout << lines[cl] << endl;
		
		//process the command
		process(lines[cl]);
		
		//now do the housework
		cl++;
		promptIndex = 0;
		prompt.x = 0;
		prompt.y+= lineHeight;
	}
	
	//all other keys...
	else {
		lines[cl].insert(it+promptIndex, (char) key);
		prompt.x+= characterWidth;
		promptIndex++;
	}

	//printf("prev command after = %i\n", prevCommand);

	//printf("width = %f\n", font.stringWidth(lines[cl]));
	//printf("o width = %f\n", stringWidth);
	
}

void ofTerminal::process(string command) {
	
	//don't try and process an empty line
	if (command != "") {
	
		//split the line up into tokens
		vector<string> tokens;
		explode(command, ' ', tokens);
		string comment = "";
		
		
		if (tokens[0] == "take") {
			comment = "you wrote take";
		}
		else {
			comment = tokens[0] +	": command not found";
		}
		
		//show the comment, if there is one and inc promt.y
		if (comment != "") {
			results.push_back(comment);
			prompt.y+= lineHeight;
		}

	}
	
	lines.push_back("");
	results.push_back("");
	
}


void ofTerminal::explode(string command, char sep, vector<string> &tokens) {
	tokens.clear();
	string t = "";
	for (int i = 0; i < command.length(); i++) {
		if (command[i] != sep) t+= command[i];
		else {
			cout << t.length() << endl;
			//don't things starting with a space..
			if (t[0] != ' ') {
				tokens.push_back(t);
				t = "";
			}
		}
	}
	// add the final one...
	if (t.length() != 1 && t[0] != 0) {
		tokens.push_back(t);
		t = "";
	}
}

void ofTerminal::setCommands(vector<string> &c) {
	c.clear();
	c.push_back("line");
	c.push_back("arc");
	c.push_back("move");
	c.push_back("circle");
	c.push_back("rect");
	
}

//--------------------------------------------------------------
void ofTerminal::keyReleased(int key){
	
}

//--------------------------------------------------------------
void ofTerminal::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofTerminal::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofTerminal::mousePressed(int x, int y, int button){
	cout << "prompt.x = " << prompt.x << endl;
	cout << "promptIndex = " << promptIndex << endl;
	cout << "lines length = " << lines[cl].length() << endl;
	cout << "string size = " << font.stringWidth(lines[cl]) << endl;
}

//--------------------------------------------------------------
void ofTerminal::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofTerminal::windowResized(int w, int h){

}

