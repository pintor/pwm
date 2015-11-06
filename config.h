/* See LICENSE file for copyright and license information*/

#define MOD Mod4Mask
#define DESKTOPS 10

const char* dmenucmd[] = {"dmenu_run",NULL};
const char* termcmd[] = {"xterm",NULL};

static struct Key keys[] = {
	{MOD, XK_p, spawn, {.com = dmenucmd}},
	{MOD, XK_Return, spawn, {.com = termcmd}},
	{MOD, XK_Left, moveWindow, {.i = P_LEFT}},
	{MOD, XK_Right, moveWindow, {.i = P_RIGHT}},
	{MOD, XK_Up, moveWindow, {.i = P_UP}},
	{MOD, XK_Down, moveWindow, {.i = P_DOWN}},

	{MOD|ShiftMask, XK_Left, tile, {.i = P_LEFT}},
	{MOD|ShiftMask, XK_Right, tile, {.i = P_RIGHT}},
	{MOD|ShiftMask, XK_Up, tile, {.i = P_UP}},
	{MOD|ShiftMask, XK_Down, tile, {.i = P_DOWN}},

	{MOD, XK_j, changeWindow, {.i = 1}},
	{MOD, XK_k, changeWindow, {.i = -1}},

	{MOD, XK_1, changeDesktop, {.i = 0}},
	{MOD, XK_2, changeDesktop, {.i = 1}},
	{MOD|ShiftMask, XK_1, moveWindowToDesktop, {.i = 0}},
	{MOD|ShiftMask, XK_2, moveWindowToDesktop, {.i = 1}},

	{MOD, XK_q, killCurrent, {NULL}},
	{MOD|ShiftMask, XK_q, quit, {NULL}},
};


static Button buttons[] = {
		{  MOD,    Button1,     mouseMotion,   {.i = P_MOVE}},
		{  MOD|ShiftMask,    Button1,     mouseMotion,   {.i = P_RESIZE}},

};
