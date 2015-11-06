/* See LICENSE file for copyright and license information*/

#define LENGTH(X)               (sizeof X / sizeof X[0])
#define CLEANMASK(mask) (mask & ~(numlock | LockMask))
#define BUTTONMASK ButtonPressMask|ButtonReleaseMask

typedef union {
	const char** com;
	const int i;
} Arg;


struct Key {
		unsigned int mod;
		KeySym keysym;
		void (*function)(const Arg arg);
		const Arg arg;
};

struct Button {
		unsigned int mask, button;
		void (*function)(const Arg arg);
		const Arg arg;
};


enum {P_LEFT, P_RIGHT, P_UP, P_DOWN};
enum { P_RESIZE, P_MOVE };

void mapRequest(XEvent *e);
void configureRequest(XEvent *e);
void keyPress(XEvent *e);
void buttonPress(XEvent  *e);
void enterNotify(XEvent *e);
void destroyNotify(XEvent *e);

void grabKeys();
void grabButtons();

void spawn(const Arg arg);
void quit (const Arg arg);
void moveWindow(const Arg arg);
void changeWindow(const Arg arg);
void changeDesktop(const Arg arg);
void moveWindowToDesktop(const Arg arg);
void killCurrent(const Arg arg);
void tile(const Arg arg);
void mouseMotion(const Arg arg);
void focusCurrent();
void eventLoop();
