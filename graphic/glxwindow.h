/*
 *  Copyright (c) 2013 - 2018 Naezzhy Petr(Наезжий Пётр) <petn@mail.ru>
 *  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* 
 * File:   glxwindow.h
 * Author: Naezzhy Petr(Наезжий Пётр) <petn@mail.ru>
 *
 * Created on 7 апреля 2018 г., 11:36
 */

#ifndef GLXWINDOW_H
#define GLXWINDOW_H


#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/glext.h>




class cGLXWindow
{
public:
				cGLXWindow();
	virtual		~cGLXWindow();
	int32_t		create_window(	uint32_t uWidth, 
								uint32_t uHeight, 
								const char *szWinCaption,
								void (*)(uint8_t, uint8_t, uint32_t, uint32_t));
	int32_t		update_window(void);
	void		destroy_window();
	int32_t		set_window_size(uint32_t uWidth, uint32_t uHeight);
	int32_t		set_window_fullscreen_popup();
	int32_t		unset_window_fullscreen_popup(uint32_t uWidth, uint32_t uHeight);
	void		hide_cursor(void);
	void		show_cursor(void);
	Window		get_window_XID(void);
	Display*	get_window_display();
	
private:
	
	int32_t		get_screen_resolution(uint32_t *puWidth, uint32_t *puHeight);
	int32_t		set_fullscreen(void);
	
	/* Callback for class returns width, height, init and resize flag */
	void	(*redraw_callback)(uint8_t initFlag, uint8_t resizeFlag, uint32_t uWidth, uint32_t uHeight);
	
	Display					*display;
	Window					window;
	XEvent					event;
	int						screen;
	
	XVisualInfo				*vi;
	Colormap				cmap;
	XSetWindowAttributes	swa;
	GLXContext				glc;
	XWindowAttributes		gwa;
	Atom					wmDelete;
	
	uint32_t				uCurrentWidth;
	uint32_t				uCurrentHeight;
	
};


cGLXWindow::
cGLXWindow()
{
	display = NULL;
	window = 0;
	cmap = 0;
	vi = NULL;
	uCurrentWidth = 0;
	uCurrentHeight = 0;
	glc = NULL;
	wmDelete = 0;
	
	redraw_callback = NULL;
}


cGLXWindow::
~cGLXWindow()
{
	destroy_window();
}

/*
 * Deinitialize window
 */
void cGLXWindow::
destroy_window()
{
	if(glc)
	{
		if(!glXMakeCurrent(display, None, NULL))
		{
			fprintf(stderr, "Could not release drawing context\n\r");
		}
		
		glXDestroyContext(display, glc);
		glc = NULL;
	}
   	
	if(display != NULL && window != 0)
	{
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = NULL;
		window = 0;
	}
}

/*
 *	Create window with uWidth and uHeight dimentions
 *  If uWidth and uHeight equal zero then window
 *  dimentions sets to fullscreen
 *  Returns 1 on success or -1 on fail
 */
int32_t cGLXWindow::
create_window(uint32_t uWidth, uint32_t uHeight, const char *szWinCaption, 
			  void(*callback_func)(uint8_t initFlag, uint8_t resizeFlag, uint32_t uWidth, uint32_t uHeight))
{
	GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
	Window		rootWindow = 0;
	
	if(callback_func == NULL)
	{
		fprintf(stderr, "Invalid callback function\r\n");
		return -1;
	}
	
	redraw_callback = (void (*)(uint8_t, uint8_t, uint32_t, uint32_t))callback_func;
	
	if(uWidth == 0 && uHeight == 0)
	{
		if (1 > get_screen_resolution(&uWidth, &uHeight) )
		{
			fprintf(stderr, "Could not get screen resolution\r\n");
			return -1;
		}
	}
	else if(uWidth == 0 || uHeight == 0)
	{
		fprintf(stderr, "Invalid screen dimentions\r\n");
		return -1;
	}
	
	if(display != NULL || window != 0)
	{
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = NULL;
		window = 0;
	}
	
	uCurrentWidth = uWidth;
	uCurrentHeight = uHeight;
	
/****************************** Begin init  ***********************************/	
	display = XOpenDisplay( NULL );
	if ( display == NULL )
	{
		fprintf(stderr, "Cannot open display\r\n");
		return -1;
	}

	screen = DefaultScreen( display );
	
	rootWindow = DefaultRootWindow(display);
	if ( rootWindow == 0 )
	{
		fprintf(stderr, "Cannot get root window XID\r\n");
		goto ERRORS;
	}
	
	vi = glXChooseVisual(display, 0, att);
	if( vi == NULL )
	{
		fprintf(stderr, "No appropriate visual found\r\n");
		goto ERRORS;
	}
	
	cmap = XCreateColormap(display, rootWindow, vi->visual, AllocNone);
	if( cmap == 0 )
	{
		fprintf(stderr, "Cannot create colormap\r\n");
		goto ERRORS;
	}
	
	swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | ButtonPressMask;

	window = XCreateWindow(
		display,
		rootWindow,
		0,
		0,
		uWidth,
		uHeight,
		0,
		vi->depth,
		InputOutput,
		vi->visual,
		CWColormap | CWEventMask | CWColormap,
		&swa);

	if(window == 0)
	{
		fprintf(stderr, "Cannot create window\r\n");
		goto ERRORS;
	}
	
//	XSetStandardProperties(display, window, szWinCaption,
 //           szWinCaption, None, NULL, 0, NULL);
 //   XMapRaised(display, window);
	
	wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(display, window, &wmDelete, 1);

	if(szWinCaption != NULL)
		XStoreName(display, window, szWinCaption);
	
	XMapWindow( display, window );
	
	
	glc = glXCreateContext(display, vi, NULL, GL_TRUE);
	if(glc == NULL)
	{
		fprintf(stderr, "Cannot create OpenGL context\r\n");
		goto ERRORS;
	}
	
    glXMakeCurrent(display, window, glc);
	
	if (glXIsDirect(display, glc)) 
        fprintf(stderr, "Direct Rendering is supported\n\r");
    else
        fprintf(stderr, "Direct Rendering is not supported\n\r");
	
//	XFlush(display);
	
	/* Set all flags in first running redraw */
	redraw_callback(1, 1, uCurrentWidth, uCurrentHeight);
	glXSwapBuffers(display, window);
	
	return 1;
	
/**************************** Errors processing	*******************************/	
ERRORS:
	
	if(glc)
	{
		if(!glXMakeCurrent(display, None, NULL))
		{
			fprintf(stderr, "Could not release drawing context\n\r");
		}
		
		glXDestroyContext(display, glc);
		glc = NULL;
	}
   	
	if(display != NULL && window != 0)
	{
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = NULL;
		window = 0;
	}
	
	return -1;
}

/*
 * Get current screen resolution
 * Returns 1 on success or -1 on fail
 */
int32_t cGLXWindow::
get_screen_resolution(uint32_t* puWidth, uint32_t* puHeight)
{
	if(puWidth == NULL || puHeight == NULL)
	{
		fprintf(stderr, "Invalid input\r\n");
		return -1;
	}
	Display	*display = NULL;
	Screen	*screen = NULL;
	display = XOpenDisplay(NULL);
	screen = DefaultScreenOfDisplay(display);
	
	if(screen == NULL)
		return -1;
	
	*puHeight = screen->height;
	*puWidth  = screen->width;
	
	XCloseDisplay(display);
	
	return 1;
}


void cGLXWindow::
hide_cursor()
{
	if(display == NULL || window == 0)
		return;
	
	XFixesHideCursor(display, window);
	XFlush(display);
}


void cGLXWindow::
show_cursor()
{
	if(display == NULL || window == 0)
		return;
	
	XFixesShowCursor(display, window);
	XFlush(display);
}


/*
 *  Set window to fullscreen popup mode without borders
 *  Returns 1 on success or -1 on fail
 */
int32_t cGLXWindow::
set_window_fullscreen_popup()
{
	if(!glc)
		return -1;
	
	XEvent		e;
	uint32_t	uWidth;
	uint32_t	uHeight;
   
	Atom		NET_WM_STATE = XInternAtom(display, "_NET_WM_STATE",	0);
	Atom		NET_WM_FULLSCREEN_MONITORS = XInternAtom(display, "_NET_WM_FULLSCREEN_MONITORS", 0);
	Atom		NET_WM_STATE_FULLSCREEN = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", 0);
	
	if (1 > get_screen_resolution(&uWidth, &uHeight) )
	{
		fprintf(stderr, "Could not get screen resolution\r\n");
		return -1;
	}
								 
	
	e.xany.type = ClientMessage;
	e.xany.window = window;
	e.xclient.message_type = NET_WM_FULLSCREEN_MONITORS;
	e.xclient.format = 32;
	e.xclient.data.l[0] = 0;
	e.xclient.data.l[1] = 0;
	e.xclient.data.l[2] = uWidth;
	e.xclient.data.l[3] = uHeight;
	e.xclient.data.l[4] = 0;

	XSendEvent(display, RootWindow(display, screen), 0, SubstructureNotifyMask | SubstructureRedirectMask, &e);
	
	e.xany.type = ClientMessage;
	e.xany.window = window;
	e.xclient.message_type = NET_WM_STATE;
	e.xclient.format = 32;
	e.xclient.data.l[0] = 1;		// flag to set or unset fullscreen state
	e.xclient.data.l[1] = NET_WM_STATE_FULLSCREEN;
	e.xclient.data.l[2] = 0;
	e.xclient.data.l[3] = 0;
	e.xclient.data.l[4] = 0;
	
	XSendEvent(display, RootWindow(display, screen), 0, SubstructureNotifyMask | SubstructureRedirectMask, &e);
	
	XFlush(display);

	return 1;
}


/*
 *  Unset window from fullscreen popup mode to normal state
 *  If uWidth and uHeight equal zero then window
 *  dimentions sets to fullscreen
 *  Returns 1 on success or -1 on fail
 */
int32_t cGLXWindow::
unset_window_fullscreen_popup(uint32_t uWidth, uint32_t uHeight)
{
	if(!glc)
		return -1;
	
	if(uWidth == 0 && uHeight == 0)
	{
		if (1 > get_screen_resolution(&uWidth, &uHeight) )
		{
			fprintf(stderr, "Could not get screen resolution\r\n");
			return -1;
		}
	}
	else if(uWidth == 0 || uHeight == 0)
	{
		fprintf(stderr, "Invalid screen dimentions\r\n");
		return -1;
	}
	
	XEvent		e;
	Atom		NET_WM_STATE = XInternAtom(display, "_NET_WM_STATE",	0);
	Atom		NET_WM_STATE_FULLSCREEN = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", 0);

	e.xany.type = ClientMessage;
	e.xany.window = window;
	e.xclient.message_type = NET_WM_STATE;
	e.xclient.format = 32;
	e.xclient.data.l[0] = 0;		// flag to set or unset fullscreen state
	e.xclient.data.l[1] = NET_WM_STATE_FULLSCREEN;
	e.xclient.data.l[2] = 0;
	e.xclient.data.l[3] = 0;
	e.xclient.data.l[4] = 0;
	
	XSendEvent(display, RootWindow(display, screen), 0, SubstructureNotifyMask | SubstructureRedirectMask, &e);
	XMoveResizeWindow(display, window, 0, 0, uWidth, uHeight);
	XMapRaised(display, window);
	
	XFlush(display);
	
	return 1;
}

/*
 *	Resize window to uWidth and uHeight dimentions
 *  If uWidth and uHeight equal zero then window
 *  dimentions sets to fullscreen
 *  Returns 1 on success or -1 on fail
 */
int32_t cGLXWindow::
set_window_size(uint32_t uWidth, uint32_t uHeight)
{
	if(!glc)
		return -1;
	
	if(uWidth == 0 && uHeight == 0)
	{
		if (1 > get_screen_resolution(&uWidth, &uHeight) )
		{
			fprintf(stderr, "Could not get screen resolution\r\n");
			return -1;
		}
	}
	else if(uWidth == 0 || uHeight == 0)
	{
		fprintf(stderr, "Invalid screen dimentions\r\n");
		return -1;
	}
	
	XMoveResizeWindow(display, window, 0, 0, uWidth, uHeight);
	XMapRaised(display, window);
	
	XFlush(display);
	
	return 1;
}


/*
 * Returns current window XID (window)
 */
Window cGLXWindow::
get_window_XID()
{
	return window;
}

/*
 * Returns current window Dispalay
 */
Display* cGLXWindow::
get_window_display()
{
	return display;
}

/*
 * Updating window events, running callbacks
 * Returns 1 on success and -1 if window was closed or not created
 */
int32_t cGLXWindow::
update_window(void)
{
	if(!glc)
		return -1;
	
	/* handle the events in the queue */
	if (XPending(display) > 0)
	{
		XNextEvent(display, &event);
		
		switch (event.type)
		{
			case Expose:
				if (event.xexpose.count != 0)
					break;

				redraw_callback(0, 0, uCurrentWidth, uCurrentHeight);
				glXSwapBuffers(display, window);
			return 1;
			case ConfigureNotify:
				/* Set resize flag only if window size was changed */
				if ((event.xconfigure.width != uCurrentWidth) || 
					(event.xconfigure.height != uCurrentHeight))
				{
					uCurrentWidth = event.xconfigure.width;
					uCurrentHeight = event.xconfigure.height;
						
					redraw_callback(0, 1, uCurrentWidth, uCurrentHeight);
					glXSwapBuffers(display, window);
				}
				
			return 1;
			case ClientMessage:
				if (event.xclient.data.l[0] == wmDelete)
					destroy_window();
            return -1;
			case ButtonPress:
                    
            return 1;
            case KeyPress:
				if (XLookupKeysym(&event.xkey, 0) == XK_Escape)
				{
					destroy_window();
					return -1;
				}
			return 1;
		}
		
	}
	else
	{
		redraw_callback(0, 0, uCurrentWidth, uCurrentHeight);
		glXSwapBuffers(display, window);
	}
	
	return 1;
}

#endif /* GLXWINDOW_H */

