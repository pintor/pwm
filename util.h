int xerror(Display *display, XErrorEvent *ee);
int xerrorstart(Display *display, XErrorEvent *ee);
unsigned int numlockmask(Display *display);
void sigchld(int sig);
void die(const char *fmt, ...);
