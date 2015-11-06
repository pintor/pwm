/* See LICENSE file for copyright and license information*/

#include <err.h>
#include<X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdarg.h>
#include<signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <err.h>


/**
 * There's no way to check accesses to destroyed windows,
 * thus those cases are ignored (especially on UnmapNotify's).
 */
int xerror( Display *display, XErrorEvent *ee) {
		if ((ee->error_code == BadAccess   && (ee->request_code == X_GrabKey
																			 ||  ee->request_code == X_GrabButton))
		|| (ee->error_code  == BadMatch    && (ee->request_code == X_SetInputFocus
																			 ||  ee->request_code == X_ConfigureWindow))
		|| (ee->error_code  == BadDrawable && (ee->request_code == X_PolyFillRectangle
		|| ee->request_code == X_CopyArea  ||  ee->request_code == X_PolySegment
																			 ||  ee->request_code == X_PolyText8))
		|| ee->error_code   == BadWindow) return 0;
		err(EXIT_FAILURE, "xerror: request: %d code: %d", ee->request_code, ee->error_code);
}

/**
 * error handler function to display an appropriate error message
 * when the window manager initializes (see setup - XSetErrorHandler)
 */
int xerrorstart(Display *display, XErrorEvent *ee) {
		errx(EXIT_FAILURE, "xerror: another window manager is already running");
}


unsigned int numlockmask(Display* display) {
	unsigned int i, j, mask;
	XModifierKeymap *modmap;

	mask = 0;
	modmap = XGetModifierMapping(display);
	for(i = 0; i < 8; i++)
		for(j = 0; j < modmap->max_keypermod; j++)
			if(modmap->modifiermap[i * modmap->max_keypermod + j]
				 == XKeysymToKeycode(display, XK_Num_Lock))
				mask = (1 << i);
	XFreeModifiermap(modmap);
	return mask;
}

void sigchld(int sig) {
		if (signal(SIGCHLD, sigchld) != SIG_ERR) while(0 < waitpid(-1, NULL, WNOHANG));
		else err(EXIT_FAILURE, "cannot install SIGCHLD handler");
}


void die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	}

	exit(1);
}
