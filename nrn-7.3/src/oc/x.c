#include <../../nrnconf.h>
#include <stdio.h>
#if defined(__TURBOC__) || defined(__linux__)
#ifndef NRNOC_X11
#define NRNOC_X11 0
#endif
#endif

#if defined(__alpha)
#undef USG
#endif

#if NRNOC_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

/* initial position of window */
#define WX	100
#define WY	100

/* size of window */
#define WIDTH 500
#define HEIGHT 390
#define MARGIN 0

/* size of icon */
#define IWIDTH 64
#define IHEIGHT 20

#define Plot(x,y)	XDrawPoint(display,win,gc,(x),(y))
#define Line(x1,y1,x2,y2)	XDrawLine(display,win,gc,(x1),(y1),(x2),(y2))
#define LAST	xold=xnew; yold=ynew

static int fast;	/* don't flush until plt(-1), use drawlines*/
static int nlinept;
static XPoint polyline[200];
static int maxnlinept=200;
static Display *display;
static Window win; 
static GC gc;
static int screen;
static XEvent report;

/*static XFontStruct *font;*/

static int D;
#define Color (D > 1)
#define Ncolors 11

static unsigned long colors[Ncolors];

extern void x11_open_window();
extern void x11_draw_vec();
int x11_init_done;
static int xnew, ynew;
static int xold, yold;
static double xscale, yscale;
#define TEKX 1000.
#define TEKY 780.

static set_colors();

void x11_fast(mode)
	int mode;
{
	fast = mode;
}

void x11flush() {
	if (fast && nlinept) {
		x11_draw_vec();
	}
	XFlush(display);
}

static void getscale() {
	int x,y;
	unsigned int width, height, border_width, depth;
	Window root;
	XGetGeometry(display, win, &root, &x, &y, & width,
		&height, &border_width, &depth);
	xscale = ((double)width)/TEKX;
	yscale = ((double)height)/TEKY;
}

void x11_coord(x, y)
	double x, y;
{
	xnew = (int)(xscale*x);
	ynew = (int)(yscale*(TEKY - y));
}

void x11_draw_vec() {
	if (nlinept > 1) {
		XDrawLines(display, win, gc, polyline, nlinept, CoordModeOrigin);
	}
	nlinept = 0;
}

void x11_vector()
{
	if (fast) {
		if (nlinept == 0) {
			polyline[0].x = xold;
			polyline[0].y = yold;
			++nlinept;
		}
		if (nlinept >= maxnlinept) {
			x11_draw_vec();
		}
		polyline[nlinept].x = xnew;
		polyline[nlinept].y = ynew;
		++nlinept;
	}else{
		Line(xold, yold, xnew, ynew);
		XFlush(display);
	}
	LAST;
}

void x11_point()
{
	Plot(xnew, ynew);
	LAST;
	if (!fast) {
		XFlush(display);
	}
}
void x11_move()
{
	if (fast) {
		if (nlinept && (xnew != xold || ynew != yold)) {
			x11_draw_vec();
		}
	}
	LAST;
}
void x11_clear(){
	XClearWindow(display, win);
	XFlush(display);
	getscale();
}

void x11_cleararea(){
	int w, h, x, y;
	w = xnew-xold;
	h = ynew-yold;
	if (w < 0) {
		w = -w;
		x = xnew;
	}else{
		x = xold;
	}
	if (h < 0) {
		h = -h;
		y = ynew;
	}else{
		y=yold;
	}
	XClearArea(display, win, x, y, w, h, False);
	if (!fast) {
		XFlush(display);
	}
}

void x11_put_text(s)
	char *s;
{
	if(fast && nlinept) {
		x11_draw_vec();
	}
	XDrawString(display, win, gc, xold, yold, s, strlen(s));
	if (!fast) {
		XFlush(display);
	}
}
void x11_setcolor(c)
	int c;
{
	if (!x11_init_done) {
		x11_open_window();
	}
	x11_draw_vec();
	if (c == 0) {
		XSetForeground(display, gc, BlackPixel(display, screen));
	} else if (!Color) {
		XSetForeground(display, gc, WhitePixel(display, screen));
	} else {
		XSetForeground(display, gc, colors[c%Ncolors]);
	}		
	if (!fast) {
		XFlush(display);
	}
}

void x11_open_window()
{
	char *window_name = "Xhocplot";
	char *display_name = NULL;
	XSizeHints size_hints;
	XWindowAttributes attr;

	if (x11_init_done) {
		return;
	}
	if ((display = XOpenDisplay(display_name)) == NULL)
	{
		(void)fprintf(stderr, "cannot connect to X server %s\n", 
					XDisplayName(display_name));
	}

	screen = DefaultScreen(display);
	
	win = XCreateSimpleWindow(display, RootWindow(display, screen), 
		WX, WY, WIDTH, HEIGHT, 0, 
		BlackPixel(display, screen), 
		WhitePixel(display, screen));

	XGetWindowAttributes(display, win, &attr);
	D = attr.depth;
	if (Color) set_colors();

	size_hints.flags = USPosition|USSize;
	size_hints.x = WX; size_hints.y = WY;
	size_hints.width = WIDTH; size_hints.height = HEIGHT;
	XSetStandardProperties(display, win, window_name, NULL, 
		0, NULL, 0, &size_hints);

	gc = XCreateGC(display, win, 0, NULL);
	XSetWindowBackground(display, win, BlackPixel(display, screen));
	XSetForeground(display, gc, WhitePixel(display, screen));
	XSetBackground(display, gc, BlackPixel(display, screen));
	XMapWindow(display, win);

/*
	font = XLoadQueryFont(display, "9x15");
	XSetFont(display, gc, font->fid);
*/
	
	XSelectInput(display, win, ExposureMask);
	XNextEvent(display, &report);
	XSelectInput(display, win, 0L);
	getscale();
	x11_init_done = 1;
}

void x11_close_window()
{
	if (x11_init_done) {
		XFreeGC(display, gc);
		XCloseDisplay(display);
	}
	x11_init_done = 0;
}
/*-----------------------------------------------------------------------------
 *   set_colors - set colors from user resources or defaults
 *---------------------------------------------------------------------------*/
char *color_names[Ncolors] = { "black", "white", "yellow", 
   "red",  "green", "blue",  "magenta", "cyan", "sienna", "orange", "coral" };

static set_colors() {
   int n;
   XColor used, exact;

   for(n=0; n<Ncolors; n++) {
      if (XAllocNamedColor(display, DefaultColormap(display,0),
      		color_names[n], &used, &exact)){
	 colors[n] = used.pixel; 
	 }
      else {
	 fprintf(stderr, "xhocplot: assuming %s:white\n", color_names[n]);
	 colors[n] = WhitePixel(display,0);
	 }
      }
   }

#endif
