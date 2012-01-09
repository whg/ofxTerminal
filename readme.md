ofxTerminal
===========

ofxTerminal allows you to control your openFrameworks apps using a command line. Function pointers are passed to ofxTerminal, which binds them to specified words. ofxTerminal supports things like autocompletion via tab, command history via up and down keys and other thing you expect from a terminal such as a blinking cursor.

Usage
-----

Check out the example project if you are impatient, otherwise read on.

To instantiate ofxTerminal you need a class and a pointer to an object of that class that the functions you want to use belong to. For example if the functions you want to call reside in testApp, you might want to do something like this in setup():

`terminal = ofxTerminal<testApp>(this);`

Functions can then be added to the object using the addFunction() member function, which takes a string as the first argument, which is what you want your function to be called and a refence to the callback for the second argument. So if you have a function called hello and you want to call it when you type hello, you would write something like:

`terminal.addFunction("hello", &testApp::hello);`

ofxTerminal can only handle one type of function signature, that looks like this:

`string aFunction(vector<string> args)`

The argument passed to the function is vector of the arguments passed to the function you specified, so for example, if in your app you write:

`hello my name is dave`

The vector passed to hello() will be ["my", "name", "is", "dave"]. Obviously, you can do what you want with these; convert to int, etc. 

Every function you write should return a string, this is used for errors/comments and the returned string will be printed under the current line, if you want the prompt to proceed as normal return and empty string.

Reading files
-------------

The one built in function in ofxTerminal is *read*. This reads files for you and executes the contents. You can supply read with a absolute file name or you can set a custom directory for it to search yourself, this is done via setPath(), something like:

`terminal.setPath("/Users/some_users/some_thing_here");`

Custom Settings
---------------

Along with setPath(), there are a few other things you can do to customise ofxTerminal, these include:

`setPS1(string s)` : This set the prompt, by default it's "? ". 

`setBlinkingCursor(bool b, float freq)` : freq is an optional argument and the default it 0.5Hz

`ofxTerminal(T *co, string fp, int fs)` : Along with the pointer, the constructor can has two optional arguments, these are the font path and the font size, by default these are: "/System/Library/Fonts/Menlo.ttc" and 11 respectively. If you change these you will probably want to use a monospaced font.

If you do decide to change the default text, you might need to use:

`setCharacterOffset(float v)` or `setSpaceOffset(float v)` to fix the alignment of things.



I have only tested this on a Mac. 