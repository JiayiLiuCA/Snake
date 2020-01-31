/*

- - - - - - - - - - - - - - - - - - - - - -

Commands to compile and run:

    g++ -o snake snake.cpp -L/usr/X11R6/lib -lX11 -lstdc++
    ./snake

Note: the -L option and -lstdc++ may not be needed on some machines.
*/

#include <iostream>
#include <list>
#include <cstdlib>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <deque>
#include <string>
#include <cmath>



/*
 * Header files for X functions
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;
 


int main ( int argc, char *argv[] );
/*
 * Global game state variables
 */
const int Border = 1;
const int BufferSize = 10;
int FPS = 60;
int Speed = 10;
int Level = 0;
bool splash = true;
const int width = 800;
const int height = 600;

/*
 * Information to draw on the window.
 */
struct XInfo {
	Display	 *display;
	int		 screen;
	Window	 window;
	GC		 gc[3];
	int		width;		// size of window
	int		height;
};


/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
  cerr << str << endl;
  exit(0);
}


/*
 * An abstract class representing displayable things. 
 */
class Displayable {
	public:
		virtual void paint(XInfo &xinfo) = 0;
};

class Text : public Displayable {
public:
	virtual void paint(XInfo &xinfo) {
		string tmp = s + " : " + to_string(tribute);
		XDrawImageString(xinfo.display, xinfo.window, xinfo.gc[0], x,y, tmp.c_str(), tmp.length());
	}

	Text(int x, int y, int t, string str) : x(x), y(y), tribute(t), s(str) {
	}


	void setTribute(int t) {
		tribute = t;
	}
	int getTribute() {
		return tribute;
	}
private:
	int x;
	int y;
	int tribute;
	string s;
};

Text score(40, 540, 0, "score");


class Fruit : public Displayable {
	public:
		virtual void paint(XInfo &xinfo) {
			XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], x, y, 20, 20);
        }

        Fruit() {
            x = 20 * (rand() % 38 + 1);
            y = 20 * (rand() % 28 + 1);
            if (Level == 1) {
            	if ((x >= 380 && x <= 400) || (y >= 280 && y <= 300)) {
            		Fruit();
            	}
            }
        }

        int getX () {
        	return x;
        }

        int getY () {
        	return y;
        }
        void setX () {
        	x = 20 * (rand() % 38 + 1);
        }
        void setY () {
        	y = 20 * (rand() % 28 + 1);
        }


    private:
        int x;
        int y;
};

Fruit fruit;

class Snake : public Displayable {
	private:
		//int x;
		//int y;
		int blockSize;
		int speed;
		int direction;
		bool eaten;
	public:
		deque<pair<int,int> > blocks;
	public:
		virtual void paint(XInfo &xinfo) {
			for (int i = 0; i < blocks.size(); i++) {
				XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], blocks[i].first, blocks[i].second, 20, blockSize);
			}
			//XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], x, y, 20, blockSize);
		}
		
		void move(XInfo &xinfo) {
			eaten = false;
			int a = blocks[0].first;
			int b = blocks[0].second;
			if (a == fruit.getX() && b == fruit.getY()) {
				eatFruit();
			}
			switch (direction) {
				case 1:
					//x += speed;
					blocks.push_front(make_pair((a+speed)%800,b));
					break;
				case 3:
					//x -= speed;
					blocks.push_front(make_pair((a-speed+800)%800,b));
					break;
				case 0:
					//y -= speed;
					blocks.push_front(make_pair(a,(b-speed+600)%600));
					break;
				case 2:
					//y += speed;
					blocks.push_front(make_pair(a,(b+speed)%600));
					break;
			}
			if (!eaten) {
				blocks.pop_back();
			}
		}
		

		int getDirection() {
			return direction;
		}
		int getSpeed() {
			return speed;
		}

        /*
         * ** ADD YOUR LOGIC **
         * Use these placeholder methods as guidance for implementing the snake behaviour. 
         * You do not have to use these methods, feel free to implement your own.
         */ 
        void eatFruit() {
        	eaten = true;
        	score.setTribute(score.getTribute() + 1);

        	fruit.setX();
        	fruit.setY();

        }

        

        void secDirection (char dir) {
        	direction = dir;
        }
        void setSpeed (int sp) {
        	speed = sp;
        }
		
		Snake(int x, int y) {
			for (int i = 0;i < 4; i++) {
				blocks.push_back(make_pair(x - i*blockSize,y));
			}
			//blocks.push_back(make_pair(x,y));

			speed = 20; //speed
            blockSize = 20;
            direction = 1;
            eaten = false;
		}
	

};



class Frame : public Displayable {
public:
	virtual void paint(XInfo &xinfo) {
		if (Level == 0) {
			XFillRectangle(xinfo.display, xinfo.window,xinfo.gc[0],0,0,20,600);
			XFillRectangle(xinfo.display, xinfo.window,xinfo.gc[0],0,0,800,20);
			XFillRectangle(xinfo.display, xinfo.window,xinfo.gc[0],0,580,800,20);
			XFillRectangle(xinfo.display, xinfo.window,xinfo.gc[0],780,0,20,600);
		}
		if (Level == 1) {
			XFillRectangle(xinfo.display, xinfo.window,xinfo.gc[0],380,0,40,600);
			XFillRectangle(xinfo.display, xinfo.window,xinfo.gc[0],0,280,800,40);
		}
	}
};





list<Displayable *> dList;           // list of Displayables
Snake snake(100, 400);
Text option1(700, 520, FPS, "FPS");
Text option2(700, 540, Speed, "Speed");

Frame frame;


/*
 * Initialize X and create a window
 */
void initX(int argc, char *argv[], XInfo &xInfo) {
	XSizeHints hints;
	unsigned long white, black;

   /*
	* Display opening uses the DISPLAY	environment variable.
	* It can go wrong if DISPLAY isn't set, or you don't have permission.
	*/	
	xInfo.display = XOpenDisplay( "" );
	if ( !xInfo.display )	{
		error( "Can't open display." );
	}
	
   /*
	* Find out some things about the display you're using.
	*/
	xInfo.screen = DefaultScreen( xInfo.display );

	white = XWhitePixel( xInfo.display, xInfo.screen );
	black = XBlackPixel( xInfo.display, xInfo.screen );

	hints.x = 100;
	hints.y = 100;
	hints.width = 800;
	hints.height = 600;
	hints.flags = PPosition | PSize;

	xInfo.window = XCreateSimpleWindow( 
		xInfo.display,				// display where window appears
		DefaultRootWindow( xInfo.display ), // window's parent in window tree
		hints.x, hints.y,			// upper left corner location
		hints.width, hints.height,	// size of the window
		Border,						// width of window's border
		black,						// window border colour
		white );					// window background colour
		
	XSetStandardProperties(
		xInfo.display,		// display containing the window
		xInfo.window,		// window whose properties are set
		"animation",		// window's title
		"Animate",			// icon's title
		None,				// pixmap for the icon
		argv, argc,			// applications command line args
		&hints );			// size hints for the window

	/* 
	 * Create Graphics Contexts
	 */
	int i = 0;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);


	XSelectInput(xInfo.display, xInfo.window, 
		ButtonPressMask | KeyPressMask | 
		PointerMotionMask | 
		EnterWindowMask | LeaveWindowMask |
		StructureNotifyMask);  // for resize events

	/*
	 * Put the window on the screen.
	 */
	XMapRaised( xInfo.display, xInfo.window );
	XFlush(xInfo.display);
}

/*
 * Function to repaint a display list
 */
void repaint( XInfo &xinfo) {
	list<Displayable *>::const_iterator begin = dList.begin();
	list<Displayable *>::const_iterator end = dList.end();

	XClearWindow( xinfo.display, xinfo.window );
	
	// get height and width of window (might have changed since last repaint)

	/*
	XWindowAttributes windowInfo;
	XGetWindowAttributes(xinfo.display, xinfo.window, &windowInfo);
	unsigned int height = windowInfo.height;
	unsigned int width = windowInfo.width;
	*/

	// big black rectangle to clear background
    
	// draw display list
	while( begin != end ) {
		Displayable *d = *begin;
		d->paint(xinfo);
		begin++;
	}
	XFlush( xinfo.display );
}


int handleKeyPress(XInfo &xinfo, XEvent &event, int eventHandled) {
	KeySym key;
	char text[BufferSize];
	
	/*
	 * Exit when 'q' is typed.
	 * This is a simplified approach that does NOT use localization.
	 */
	int i = XLookupString( 
		(XKeyEvent *)&event, 	// the keyboard event
		text, 					// buffer when text will be written
		BufferSize, 			// size of the text buffer
		&key, 					// workstation-independent key symbol
		NULL );					// pointer to a composeStatus structure (unused)
	if ( i == 1) {
		printf("Got key press -- %c\n", text[0]);
		if (text[0] == 'q') {
			error("Terminating normally.");
		}
		if (text[0] == 'r') { //reset
			snake = Snake(100,400);
			score.setTribute(0);
			fruit = Fruit();
			if (eventHandled == 3) {
				dList.pop_back();
			}
			if (!splash) {
				eventHandled = 0;
			}
		}
		if (text[0] == 'p') {
			splash = false;
			if (eventHandled == 0) { //pause
				return 2;
			}
			if (eventHandled == 2) { //resume
				return 0;
			}
		}
		if (text[0] == 'w' && eventHandled == 0) {
			if (snake.getDirection() == 1 || snake.getDirection() == 3) {

				if (Level == 1) {
					snake.secDirection(2);
					return 1;
				}
				snake.secDirection(0);
				//cout << "up" << endl;
				return 1;
			}
		}
		if (text[0] == 'a' && eventHandled == 0) {

			if (snake.getDirection() == 0 || snake.getDirection() == 2) {
				if (Level == 1) {
					snake.secDirection(1);
					return 1;
				}
				snake.secDirection(3);
				//cout << "left" << endl;
				return 1;
			}
		}
		if (text[0] == 's' && eventHandled == 0) {
			
			if (snake.getDirection() == 1 || snake.getDirection() == 3) {
				if (Level == 1) {
					snake.secDirection(0);
					return 1;
				}
				snake.secDirection(2);
				//cout << "down" << endl;
				return 1;
			}
		}
		if (text[0] == 'd' && eventHandled == 0) {
			
			if (snake.getDirection() == 0 || snake.getDirection() == 2) {
				if (Level == 1) {
					snake.secDirection(3);
					return 1;
				}
				snake.secDirection(1);
				//cout << "right" << endl;
				return 1;
			}
		}
	}
	return eventHandled;
}

void handleAnimation(XInfo &xinfo, int inside) {

	if (inside) {
		snake.move(xinfo);
	}
	//snake.move(xinfo);
	
	
}

// get microseconds
unsigned long now() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

Text option3(350, 200, score.getTribute(), "Game Over, Score");


void eventLoop(XInfo &xinfo) {
	dList.push_front(&score);
	dList.push_front(&option1);
	dList.push_front(&option2);
	dList.push_front(&snake);
    dList.push_front(&fruit);
    dList.push_front(&frame);
	
	XEvent event;
	int points = 0;

	unsigned long lastRepaint = 0;
	int inside = 0;
	unsigned long tmp;
	int counter = 0;
	int eventHandled = 2;
	//int eventHandled = 0;
	

	while( true ) {

		
		//cout << "start loop" << endl;
		

		if (XPending(xinfo.display) > 0) {
			XNextEvent( xinfo.display, &event );
			//cout << "event.type=" << event.type << "\n";
			switch( event.type ) {
				case KeyPress:
					eventHandled = handleKeyPress(xinfo, event, eventHandled);
					break;
				case EnterNotify:
					inside = 1;
					break;
				case LeaveNotify:
					inside = 0;
					break;
			}
		} 

		if (splash) {
			XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], 0, 0, 800, 600);
			string s1 = "j437liu";
			string s2 = "Jiayi Liu";
			string s3 = "WASD to control the snake";
			string s4 = "Press P to play or pause, R to reset, Q to quit";

			XDrawImageString(xinfo.display, xinfo.window, xinfo.gc[0], 300,200, s1.c_str(), s1.length());
			XDrawImageString(xinfo.display, xinfo.window, xinfo.gc[0], 300,230, s2.c_str(), s2.length());
			XDrawImageString(xinfo.display, xinfo.window, xinfo.gc[0], 300,260, s3.c_str(), s3.length());
			XDrawImageString(xinfo.display, xinfo.window, xinfo.gc[0], 300,290, s4.c_str(), s4.length());
			usleep(1000000/FPS);
			continue;
		}
		
		unsigned long end = now();
		if (end - lastRepaint > 1000000 / FPS) {
	
			
			if (counter == FPS/(2*Speed) && eventHandled <= 1) {
				handleAnimation(xinfo, inside);
				
				if (Level == 0) {
					if (snake.blocks[0].first <= 0 || snake.blocks[0].first >= 780 ||
					  	snake.blocks[0].second <= 0|| snake.blocks[0].second>= 580) {
						cout << "bound" << endl;
						option3.setTribute(score.getTribute());
						dList.push_back(&option3);
						eventHandled = 3; // GameOver
					}
				}
				if (Level == 1) {
					if ((snake.blocks[0].first >= 380 && snake.blocks[0].first <= 400) ||
						(snake.blocks[0].second >= 280 && snake.blocks[0].second <= 300)) {
						cout << "bound" << endl;
						option3.setTribute(score.getTribute());
						dList.push_back(&option3);
						eventHandled = 3; // GameOver
					}
				}
				if (snake.blocks.size() > 4) {
					for (int i = 4; i < snake.blocks.size(); i++) {
						//cout << "block : " << snake.blocks[i].first << " " << snake.blocks[i].second << endl;

						if (i >= 4 && snake.blocks[i] == snake.blocks[0]) {
							cout << "eaten" << endl;
							option3.setTribute(score.getTribute());
							dList.push_back(&option3);
							eventHandled = 3;
							break;
						} 
					}
				}
			}

			repaint(xinfo);

			//cout << "Repaint" << endl;
			counter += 1;
			if (counter > FPS/(2*Speed)) {
				if (eventHandled <= 1) {
					eventHandled = 0;
				}
				counter = 0;
			}
			//cout << "counter : " << counter << endl;
			//cout << "eventHandled : " << eventHandled << endl;
			
			//cout << "repaint took time : " <<  now()-end << endl;
			lastRepaint = now();
			//cout << "repaint took time : " <<  tmp << endl;
		}



		if (XPending(xinfo.display) == 0) {

			usleep(1000000 / FPS - (end - lastRepaint));
			

		}		
		//usleep(1);
	}
}


/*
 * Start executing here.
 *	 First initialize window.
 *	 Next loop responding to events.
 *	 Exit forcing window manager to clean up - cheesy, but easy.
 */
int main ( int argc, char *argv[] ) {
	XInfo xInfo;
	if (argc >= 3) {
		//cout << "assignment" << endl;
		FPS = atoi(argv[1]);
		Speed = atoi(argv[2]);
		//cout << argc << endl;
		if (argc >= 4) {
			Level = atoi(argv[3]);
		}
		option1.setTribute(FPS);
		option2.setTribute(Speed);
	}
	initX(argc, argv, xInfo);
	eventLoop(xInfo);
	XCloseDisplay(xInfo.display);
}
