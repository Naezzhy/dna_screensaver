/*
 *  Copyright (c) 2013 - 2020 Naezzhy Petr(Наезжий Пётр) <petn@mail.ru>
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
 * File:   dna_screensaver.cpp
 * Author: Naezzhy Petr(Наезжий Пётр) <petn@mail.ru>
 *
 * Created on 22 января 2020 г., 14:54
 */

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <linux/limits.h>

#include <math.h> 

#include <GL/gl.h>
#include <GL/glu.h>

#include "utils/time_func.h"
#include "graphic/glxwindow.h"


uint64_t		_uPrevMillis;
float			_fDelta = 0;
GLUquadricObj*  _quadrObj = NULL;
uint64_t		_uCurrMillis;



/* Redraw window callback */
void
redraw_window(uint8_t initFlag, uint8_t redrawFlag,
				uint32_t uWidth, uint32_t uHeight)
{

	GLfloat		const	lightPosition[] = { 1, 1.0f, 2.0f, 0.0f };
	GLfloat		const	materialSpecular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	
	if(initFlag)
	{
		_uPrevMillis = get_millisec();
		
		glEnable(GL_POLYGON_SMOOTH);
		glShadeModel(GL_SMOOTH);

		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
		glEnable(GL_AUTO_NORMAL);

		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_DEPTH_TEST);

		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,false);
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
		glMaterialf(GL_FRONT, GL_SHININESS, 128);

		_quadrObj = gluNewQuadric();
		
		glDepthFunc(GL_LEQUAL);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glFlush();
	}
	
	if(redrawFlag)
	{
		glViewport(0, 0, uWidth, uHeight);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(70,(double)uWidth/uHeight,1,170);
		gluLookAt( 0,0,1, 0,0,0, 0,1,0 );

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0, 0, 6,     0, 0, 0,     0, 1, 0);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	}
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

	glTranslatef(0,0,-2);
	glTranslatef(0,2.7,0);
	glRotatef(_fDelta,0,1,0);
	
	_uCurrMillis = get_millisec();
	_fDelta += 0.05 * float(_uCurrMillis-_uPrevMillis);
	_uPrevMillis = _uCurrMillis;
	//delta+=0.3;
	if(_fDelta>360)
		_fDelta=0;

	for(int i=0; i<13;i++)
	{
		glTranslatef(0,-0.4,0);
		glRotatef(20*cos((_fDelta*3.14)/180),0,1,0);

		glColor3f(1, sin((_fDelta*3.14)/180*i), cos((_fDelta*3.14)/180*i));
		glPushMatrix();
		   glTranslatef(-0.8,0,0);
			 gluSphere (_quadrObj, 0.2, 30, 30);
		glPopMatrix();

		glPushMatrix();
		   glTranslatef(0.8,0,0);
			 gluSphere (_quadrObj, 0.2, 30, 30);
		glPopMatrix();

		glPushMatrix();
		glColor3f(cos((_fDelta*3.14)/180*i), 1, sin((_fDelta*3.14)/180*i));
		   glTranslatef(-0.6,0,0);
		   glRotatef(90,0,1,0);
		   gluCylinder (_quadrObj, 0.07, 0.07, 1.3, 30, 1);
		glPopMatrix();
	}

    glFlush();

}
/*
 * 
 */
int main(int argc, char** argv)
{
	cGLXWindow				window;
	
	
	window.create_window(800, 600, "dna", redraw_window);
	
	window.hide_cursor();
	window.set_window_fullscreen_popup();
	
	while( 0 < window.update_window() )
	{
		sleep_millisec(1);
	}

	
	window.show_cursor();

	return 0;
}

