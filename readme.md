ofxTerminal
===========

ofxTerminal allows you to control your openFrameworks apps using a command line. Function pointers are passed to ofxTerminal, which binds them to specified words. ofxTerminal supports things like autocompletion via tab (works almost identically to sh/bash), command history via up and down keys and other thing you expect from a terminal such as a blinking cursor.

Usage
-----

Check out the example project if you are impatient, otherwise read on.

To instantiate ofxTerminal you need a class and a pointer to an object of that class that the functions you want to use belongs to. For example, if the functions you want to call reside in testApp, you might want to do something like this in setup():

`terminal = ofxTerminal<testApp>(this);`

Functions can then be added to the object using the `addFunction()` member function, which takes a string as the first argument, which is what you want your function to be called and a reference to the callback for the second argument. So if you have a function called hello and you want to call it when you type hello, you would write something like:

`terminal.addFunction("hello", &testApp::hello);`

ofxTerminal can only handle one type of function signature, that looks like this:

`string aFunction(vector<string> args)`

The argument passed to the function is a STL vector of the arguments passed to the function you specified, so for example, if in your app you write:

`hello my name is dave`

The vector passed to hello() will be ["my", "name", "is", "dave"]. Obviously, you can do what you want with these; convert to int, etc. 

Every function you write should return a string, this is used for errors/comments and the returned string will be printed under the current line, if you want the prompt to proceed as normal return an empty string.

If you want to see the terminal you will need to call the `draw() member function at some point. Also, for ofxTerminal to receive the key commands, pass oF's keyPressed() argument to the `keyPressed()` member function of ofxTerminal.

Reading files
-------------

The one built in function in ofxTerminal is *read*. This reads files for you and executes the contents. You can supply read with a absolute file name (i.e. starting with a /) or you can set a custom directory for it to search yourself, this is done via `setPath()`, something like:

`terminal.setPath("/Users/some_user/some_directory");`

Controls
--------

You can use a few control-key combos, it's the usual stuff like:

* **C-e** : Move point to the end of the line.
* **C-a** : Move point to the beginning of the line
* **C-u** : clear current line
* **C-k** : clear from point to end of the line
* **C-h** : hide the whole thing
* **C-c** : clear the screen (move everything up, like `clear` in UNIX)
* **C-v** : clears screen and all settings, including command history


Custom Settings
---------------

Along with setPath(), there are a few other things you can do to customise ofxTerminal, these include:

`setPS1(string s)` : This set the prompt, by default it's '? '. You will probably want to have a space at the end of your string so wrap your text in single quotes, double quotes won't work. If you want to use the single quote character as part of your prompt then you can escape it with \.

`setBlinkingCursor(bool b, float freq)` : freq is an optional argument and the default it 0.5Hz

`ofxTerminal(T *co, string fontpath, int fontsize)` : Along with the pointer, the constructor can has two optional arguments, these are the font path and the font size, by default these are: "/System/Library/Fonts/Menlo.ttc" and 11 respectively. If you change these you will probably want to use a monospaced font.

`setFontColor(int r, int g, int b)` & `setPromptColor(int r, int g, int b)` : these should be fairly self explanatory.

If you do decide to change the default text, you might need to use:

`setCharacterOffset(float v)` or `setSpaceOffset(float v)` to fix the alignment of things.

Sometimes you might have various words that you use a lot and would like to autocomplete them. To do this, you need to add the word into ofxTerminal's dictionary:

`terminal.addToDictionary("straight");`

Compatibility
-------------

I have only tested this on a Mac. 

Changelog
---------

21/3/12 - revamped autocomplete so multiple possibilities get printed out if tab is pressed twice
		- added C-v & C-k
		- changed keys to OF_KEY constants for better compatibility
		
	
	
	
email any questions to wgallia@gmail.com