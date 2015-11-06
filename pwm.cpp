/* See LICENSE file for copyright and license information*/

#include<vector>
#include<algorithm>
#include<unistd.h>
#include<X11/Xlib.h>
#include <X11/keysym.h>
#include"util.h"
#include"pwm.h"
#include"config.h"



// Our main Desktop struct.
struct Desktop {
	std::vector<Window>  windows;
	int  current;

	Desktop() : windows(), current(0) {}

public:
	Bool isEmpty(){
		return (windows.size() == 0);
	}
	Window currentWindow() {
		return windows[current];
	};

	void addWindow(Window window) {
		windows.push_back(window);
		current = (windows.size()-1);
	};

	Window changeCurrent(int i) {
		i = (i >= 1) ? 1 : -1;
		current = (current + i) % windows.size();
		return windows[current];
	};
	void removeWindow(Window w){
		if (windows.size() > 0) {
			windows.erase( std::remove( windows.begin(), windows.end(), w ), windows.end());
			if (windows.size() == 0) {
				current = 0;
			} else
				current = (current - 1) % windows.size();

		}
	}
	Window removeCurrent() {
		Window w = windows[current];
		removeWindow(w);
		return w;
	}

};


/* GLOBAL VARS */
static Display *display;
static Window root;
static Desktop desktops[DESKTOPS];
static int currentDesktop = 0;
static Bool running = True;
static int displayWidth, displayHeight;


/**
 *	Handle event when a mouse button is pressed.
 *	Simply go through the buttons array and find the appropriate function.
 */
void buttonPress(XEvent *e) {

	unsigned int numlock = numlockmask(display);

	for (unsigned int i = 0; i < LENGTH(buttons); i++)
		if (CLEANMASK(buttons[i].mask) == CLEANMASK(e->xbutton.state) &&
				buttons[i].button == e->xbutton.button) {
			buttons[i].function(buttons[i].arg);
		}
}

/**
 *	Handle event when a keyboard button is pressed.
 *	Simply go through the keys array and find the appropriate function.
 */
void keyPress(XEvent *e) {
		int i;
		XKeyEvent ke = e->xkey;
		KeySym keysym = XKeycodeToKeysym(display,ke.keycode,0);

		for(i = 0; i < LENGTH(keys); i++) {
				if(keys[i].keysym == keysym && keys[i].mod == ke.state) {
						keys[i].function(keys[i].arg);
				}
		}
}


/**
 *	Handle event when a new window needs to be mapped to the screen.
 *  Here is where we store a window into our vector
 */
void mapRequest(XEvent *e) {
	XMapRequestEvent *ev = &e->xmaprequest;

	auto v = desktops[currentDesktop].windows;
	if((std::find(v.begin(), v.end(), ev->window) != v.end())) {
		XMapWindow(display,ev->window);
		return;
	}

	desktops[currentDesktop].addWindow(ev->window);
	XMapWindow(display,ev->window);
	XSelectInput(display, ev->window, EnterWindowMask);
	focusCurrent();

}



/**
 * Quite self-explanatory. Focus the current window on the current desktop
 */
void focusCurrent() {

	if (!(desktops[currentDesktop].isEmpty())) {
		Window w = desktops[currentDesktop].currentWindow();
		XSetInputFocus(display, w, RevertToPointerRoot, CurrentTime);
		XRaiseWindow(display, w);
	}
}


/**
 * Handle event to respect an application prefered size and position
 */
void configureRequest(XEvent *e) {

		XConfigureRequestEvent *ev = &e->xconfigurerequest;
		XWindowChanges wc;
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(display, ev->window, ev->value_mask, &wc);
}


/**
 * Handle event when a window is destroyed.
 */
void destroyNotify(XEvent *e) {
	if (!(desktops[currentDesktop].isEmpty())) {
		desktops[currentDesktop].removeWindow(e->xdestroywindow.window);
		focusCurrent();
	}
}

/**
 * Handle event when the mouse enters a window
 */
void enterNotify(XEvent *e) {
	if (e->xcrossing.window != root){
		auto v = desktops[currentDesktop].windows;
		int i = std::find(v.begin(), v.end(), e->xcrossing.window) - v.begin();
		if (i < v.size()) {
			desktops[currentDesktop].current = i;
			XSetInputFocus(display, e->xcrossing.window, RevertToPointerRoot, CurrentTime);
		}
	}

}

/**
 * Kill the current window
 */
void killCurrent(const Arg arg) {
	if (!(desktops[currentDesktop].isEmpty())){
		Window w = desktops[currentDesktop].removeCurrent();
		XKillClient(display, w);
		focusCurrent();
	}
}

/**
 *  Change from one desktop to another
 */
void changeDesktop(const Arg arg) {

	int i = arg.i % DESKTOPS;

	if (i == currentDesktop) return;

	for (Window w : desktops[currentDesktop].windows) {
		XUnmapWindow(display, w);
	}
	for (Window w: desktops[i].windows) {
		XMapWindow(display, w);
	}
	currentDesktop = i;
	focusCurrent();
}

/**
 *  Move a window to a different desktop
 */
void moveWindowToDesktop(const Arg arg) {
	int i = arg.i % DESKTOPS;
	if (i == currentDesktop) return;
	Window w =  desktops[currentDesktop].removeCurrent();
	desktops[i].addWindow(w);
	XUnmapWindow(display, w);
	focusCurrent();
}

/**
 * Go to the next or previous window.
 */
void changeWindow(const Arg arg) {
	desktops[currentDesktop].changeCurrent(arg.i);
	focusCurrent();

}

/*
 * Snap windows like in Windows
 */
void tile(const Arg arg) {

	XWindowAttributes wa;
	XGetWindowAttributes(display, desktops[currentDesktop].currentWindow(), &wa);

	int x = 0;
	int y = 0;
	int w = displayWidth;
	int h = displayHeight;
	switch (arg.i) {
	case P_LEFT:
		w = w / 2;
		break;
	case P_UP:
		h = h / 2;
		break;
	case P_DOWN:
		y = h / 2;
		h = h / 2;
		break;
	case P_RIGHT:
		x = w / 2;
		w = w / 2;
		break;
	}
	XMoveResizeWindow(display, desktops[currentDesktop].currentWindow(), x, y, w, h);
}

/*
 * Resize or move windows using mouse.
 */
void mouseMotion(const Arg arg) {
	 XWindowAttributes wa;
	 XEvent ev;
	 Window cw  = desktops[currentDesktop].currentWindow();

		if (!cw || !XGetWindowAttributes(display, cw, &wa)) return;

		if (arg.i == P_RESIZE) XWarpPointer(display, cw, cw, 0, 0, 0, 0, --wa.width, --wa.height);

		int rx, ry, c, xw, yh; unsigned int v; Window w;
		if (!XQueryPointer(display, root, &w, &w, &rx, &ry, &c, &c, &v) || w != cw) return;

		if (XGrabPointer(display, root, False, BUTTONMASK|PointerMotionMask, GrabModeAsync,
										 GrabModeAsync, None, None, CurrentTime) != GrabSuccess) return;


		do {
				XMaskEvent(display, BUTTONMASK|PointerMotionMask|SubstructureRedirectMask, &ev);
				switch (ev.type) {
				case MotionNotify:
						xw = (arg.i == P_MOVE ? wa.x : wa.width)  + ev.xmotion.x - rx;
						yh = (arg.i == P_MOVE ? wa.y : wa.height) + ev.xmotion.y - ry;
						if (arg.i == P_RESIZE) XResizeWindow(display, w,
																							 xw > 50 ? xw:wa.width, yh > 50 ? yh:wa.height);
						else if (arg.i == P_MOVE) XMoveWindow(display, w, xw, yh);
						break;
				case ConfigureRequest:
					configureRequest(&ev);
					break;
				case MapRequest:
					mapRequest(&ev);
					break;
				}
		} while (ev.type != ButtonRelease);

		XUngrabPointer(display, CurrentTime);


}

/*
 * Move window using keyboard
 */
void moveWindow(const Arg arg) {
	XWindowAttributes wa;
	XGetWindowAttributes(display, desktops[currentDesktop].currentWindow(), &wa);

	int x = 0;
	int y = 0;
	switch (arg.i) {
	case P_LEFT:
		x = -10;
		break;
	case P_UP:
		y = -10;
		break;
	case P_DOWN:
		y = 10;
		break;
	case P_RIGHT:
		x = 10;
		break;
	}
	XMoveWindow(display, desktops[currentDesktop].currentWindow(), wa.x+x, wa.y+y);

}




/*
 * Spawn a new process
 */
void spawn(const Arg arg) {
		if (fork())
			return;
		if (display)
			close(ConnectionNumber(display));
		setsid();
		execvp((char*)arg.com[0], (char**)arg.com);
}


/**
 * Quit the window manager
 */
void quit(const Arg arg) {
	running = False;
}

/**
 * Grab keys from keys array
 */
void grabKeys() {

	unsigned int numlock = numlockmask(display);
	unsigned int i, j;
	unsigned int modifiers[] = { 0, LockMask, numlock, numlock|LockMask };
	KeyCode code;

	XUngrabKey(display, AnyKey, AnyModifier, root);
	for(i = 0; i < LENGTH(keys); i++)
		if((code = XKeysymToKeycode(display, keys[i].keysym)))
			for(j = 0; j < LENGTH(modifiers); j++)
				XGrabKey(display, code, keys[i].mod | modifiers[j], root,
								 True, GrabModeAsync, GrabModeAsync);

}

/**
 * Grab buttons from buttons array
 */
void grabButtons() {
		unsigned int numlock = numlockmask(display);
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlock, numlock|LockMask };
		for(i = 0; i < LENGTH(buttons); i++)
				for(j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(display, buttons[i].button, buttons[i].mask|modifiers[j], root,
											False, BUTTONMASK, GrabModeAsync, GrabModeAsync, None, None);
}

/**
 *  Initial setup function.
 */
void setup() {
	int screen;
	XSetWindowAttributes wa;

	// Kill zombies :)
	sigchld(0);

	screen         = DefaultScreen(display);
	root           = RootWindow(display, screen);
	displayWidth  = XDisplayWidth(display, screen);
	displayHeight = XDisplayHeight(display, screen);

	//Initialize desktops
	for(int i = 0; i < DESKTOPS; i++) {
		desktops[i] = Desktop();
	}

	grabKeys();
	grabButtons();

	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask;
	XChangeWindowAttributes(display, root, CWEventMask, &wa);
	XSelectInput(display, root, wa.event_mask);


}

void eventLoop() {
		XEvent ev;

		while(running && !XNextEvent(display, &ev)) {
			switch(ev.type){
			case ButtonPress:
				buttonPress(&ev);
				break;
			case KeyPress:
				keyPress(&ev);
				break;
			case MapRequest:
				mapRequest(&ev);
				break;
			case ConfigureRequest:
				configureRequest(&ev);
				break;
			case DestroyNotify:
				destroyNotify(&ev);
				break;
			case EnterNotify:
				enterNotify(&ev);
				break;
			}
		}

}

int main(int argc, char* argv[]) {

	display = XOpenDisplay(NULL);

	if (!display) die("Display cannot be opened");

	// Check if other WM is running
	XSetErrorHandler(xerrorstart);
	XSelectInput(display, DefaultRootWindow(display), SubstructureRedirectMask);
	XSync(display, False);
	XSetErrorHandler(xerror);
	XSync(display, False);

	setup();
	eventLoop();

	XCloseDisplay(display);
}
