/* 
 * File:   Lms_glWindow.cpp
 * Author: ignas
 * 
 * Created on January 12, 2016, 11:35 AM
 */

#include "Lms_glWindow.h"
#include <string> 

void Lms_glWindow::Timer_CB(void *data)
{
        Fl::repeat_timeout(1.0f/20.0f, Timer_CB, nullptr);
    }

Lms_glWindow::Lms_glWindow(int X,int Y,int W,int H,const char*L) : Fl_Gl_Window(X,Y,W,H,L) {

        data_x = NULL;
        data_y = NULL;
        data_size = 0;  
        grid_xspacing = 100;
        grid_yspacing = 100;
        display_xmin = -W/2.0f;
        display_ymin = -H/2.0f;
        display_xmax = W/2.0f;
        display_ymax = H/2.0f;
        data_xstep = 1.0f;
        FixGrid();
        graph_type = GL_LINE_STRIP;
        selection.active = false;
        Fl::add_timeout(1.0f/20.0f, Timer_CB, nullptr); 
        end();       
}

void Lms_glWindow::FixGrid()
{
	while ((display_ymax-display_ymin)/grid_yspacing > h()/45.0f)
		grid_yspacing *= 2; 
	while ((display_ymax-display_ymin)/grid_yspacing < h()/90.0f)
		grid_yspacing /= 2; 
	while ((display_xmax-display_xmin)/grid_xspacing > w()/60.0f)
		grid_xspacing *= 2; 
	while((display_xmax-display_xmin)/grid_xspacing < w()/120.0f)
		grid_xspacing /= 2; 
}

int Lms_glWindow::handle(int event)
{
    static float start_x;
    static float start_y;
    switch (event)
    {
        case FL_PUSH:   
			selection.active = false;
			if (Fl::event_button() == FL_RIGHT_MOUSE)
			{
				start_x = Fl::event_x()*(display_xmax-display_xmin)/w();
				start_y = Fl::event_y()*(display_ymax-display_ymin)/h();
			}
			else if (Fl::event_button() == FL_LEFT_MOUSE)
			{
				selection.x1 = Fl::event_x();
				selection.y1 = h()-Fl::event_y();
			}
                        return 1;
        case FL_DRAG:
       {
            if (Fl::event_button() == FL_RIGHT_MOUSE)
			{
				float fx = Fl::event_x()*(display_xmax-display_xmin)/w();
				float fy = Fl::event_y()*(display_ymax-display_ymin)/h(); 
				display_ymin += fy-start_y;
				display_xmin -= fx-start_x;
				display_ymax += fy-start_y;
				display_xmax -= fx-start_x;
				start_x = fx;
				start_y = fy;
				redraw();
				}
			else  if (Fl::event_button() == FL_LEFT_MOUSE)
			{
		     selection.active = true;
		     selection.x2 = Fl::event_x(); 
			 selection.y2 = h()-Fl::event_y();
		     redraw();
			 }
            return 1;
         }
        case FL_RELEASE:
        {         
			if (selection.active && Fl::event_button() ==FL_LEFT_MOUSE)
			{
				selection.x1 *= (display_xmax-display_xmin)/w();
				selection.x2 *= (display_xmax-display_xmin)/w();
                                selection.y1 *= (display_ymax-display_ymin)/h();
				selection.y2 *= (display_ymax-display_ymin)/h();
                                selection.x1 += display_xmin;
				selection.x2 += display_xmin;
                                selection.y1 += display_ymin;
				selection.y2 += display_ymin;
                                display_ymin = selection.y1 > selection.y2 ? selection.y2:selection.y1;
				display_ymax = selection.y1 > selection.y2 ? selection.y1:selection.y2;
				display_xmin = selection.x1 > selection.x2 ? selection.x2:selection.x1;
				display_xmax = selection.x1 > selection.x2 ? selection.x1:selection.x2;
				selection.active = false;
				FixGrid();
				redraw();
                            }
                        
			else if (Fl::event_button() ==FL_MIDDLE_MOUSE && data_y != nullptr && data_x != nullptr)	
			{
			 display_ymin = display_ymax = data_y[0];
			 display_xmin = display_xmax = data_x[0];
			 for (int i = 1; i < data_size; i++)
			 {
			  
			  if (display_ymin  > data_y[i])
				  display_ymin = data_y[i];
			  else if (display_ymax  < data_y[i])
				  display_ymax = data_y[i];

			  if (display_xmin  > data_x[i])
				  display_xmin = data_x[i];
			  else if (display_xmax  < data_x[i])
				  display_xmax = data_x[i];			  
			  }
                         FixGrid();
			 redraw();
			 }
                        return 1;;
                    }
        case FL_MOUSEWHEEL:
        {
            float fdy =  Fl::event_dy();
            float fdx =  fdy;
            fdy *= (display_ymax-display_ymin)/50.0f;
            fdx *= (display_xmax-display_xmin)/50.0f;
            display_ymin += fdy;
            display_xmin += fdx;
            display_ymax -= fdy;
            display_xmax -= fdx;
            if ((display_ymax-display_ymin)/grid_yspacing > h()/45.0f)
               grid_yspacing *= 2; 
            else if((display_ymax-display_ymin)/grid_yspacing < h()/90.0f)
               grid_yspacing /= 2; 
            if ((display_xmax-display_xmin)/grid_xspacing > w()/60.0f)
               grid_xspacing *= 2; 
            else if((display_xmax-display_xmin)/grid_xspacing < w()/120.0f)
               grid_xspacing /= 2; 
            redraw();
            return 1;
        }
        default: break;
    }
    return Fl_Gl_Window::handle(event);
}

void Lms_glWindow::FixViewport(int W,int H) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0,0,W,H);
        glOrtho(0,W,0,H,-1,1);
        glMatrixMode(GL_MODELVIEW);
    }

void Lms_glWindow::draw() {
        if (!valid()) { valid(1); FixViewport(w(), h()); }      
        // Clear screen 
        glClearColor(0.975f, 0.975f, 0.975f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        //reset matrix       
        glLoadIdentity();
		//temp scaling variables
		float tmp_x = w()/(display_xmax-display_xmin);
        float tmp_y = h()/(display_ymax-display_ymin);
		//draw numbering
		glColor3f(0.0f, 0.0f, 0.0f);  
		gl_font(FL_HELVETICA,14);
	    int itmp = display_xmin/grid_xspacing;
		for (float i = grid_xspacing*itmp; i < display_xmax; i+=grid_xspacing)
		  {
			char tmp_str[16];
			sprintf(tmp_str,"%.4g",i);			
			gl_draw(tmp_str,(i-display_xmin)*tmp_x,5.0f);
		  }  
		itmp = display_ymin/grid_yspacing;
		for (float i = grid_yspacing*itmp; i < display_ymax; i+=grid_yspacing)
		  {
			char tmp_str[16];
			sprintf(tmp_str,"%.4g",i);
			gl_draw(tmp_str,5.0f,(i-display_ymin)*tmp_y-4.0f);
		  }  
		//draw selection rectangle
		if (selection.active)
		{
			glColor3f(0.6f, 0.8f, 0.8f);  
			glBegin(GL_LINE_STRIP);
			glVertex3f(selection.x1,selection.y1,0.0f);
			glVertex3f(selection.x1,selection.y2,0.0f);
			glVertex3f(selection.x2,selection.y2,0.0f);
			glVertex3f(selection.x2,selection.y1,0.0f);
			glVertex3f(selection.x1,selection.y1,0.0f);
			glEnd();
			glColor3f(0.0f, 0.0f, 0.0f); 
		}

        //prapare to draw grid and values
        glScalef(tmp_x,tmp_y,1.0f);
        glTranslatef(-display_xmin,-display_ymin,0.0f); 
	   //draw axis
        glBegin(GL_LINES);
        glVertex3f(display_xmin,0.0f,0.0f);glVertex3f(display_xmax,0.0f,0.0f);
        glVertex3f(0.0f,display_ymin,0.0f);glVertex3f(0.0f,display_ymax,0.0f);
        glEnd();
		//draw grid
        glLineStipple(1, 0x1111);
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINES);
		itmp = 2*display_xmin/grid_xspacing;  
		for (float i = grid_xspacing*itmp/2; i < display_xmax; i+=grid_xspacing/2) {
		   glVertex3f(i,display_ymin,0.0f);glVertex3f(i,display_ymax,0.0f);   
		}  
		itmp = 2*display_ymin/grid_yspacing;
		for (float i = grid_yspacing*itmp/2; i < display_ymax; i+=grid_yspacing/2) {
		   glVertex3f(display_xmin,i,0.0f);glVertex3f(display_xmax,i,0.0f);   
		  }
		glEnd();
		glDisable(GL_LINE_STIPPLE);
        //draw data values
        glColor3f(0.125f, 0.6f, 0.2f);
        glBegin(graph_type);
		for (int i = 0; i < data_size; i++) 
             glVertex3f(data_x[i], data_y[i],0.0f); 
        glEnd();       
    }

void Lms_glWindow::InitData(int size, float xmin, float xmax)
{
 if (data_y != NULL)
     delete [] data_y;
 if (data_x != NULL)
     delete [] data_x;
 data_x = new float[size];
 data_y = new float[size];
 data_size = size;
 data_xmin = xmin;
 data_xstep = (xmax-xmin)/size;
 float f = data_xmin;
 for (int i =0; i < size; i++)
 {
	 data_x[i] = f;
	 f += data_xstep;
 }
}

void Lms_glWindow::InitData(int size)
{
 if (data_y != NULL)
     delete [] data_y;
 if (data_x != NULL)
     delete [] data_x;
 data_x = new float[size];
 data_y = new float[size];
 data_size = size;
}

void Lms_glWindow::SetData(float* y, float* x)
{
 for (int i = 0; i < data_size; i++)
 {
  data_y[i] = y[i];
  data_x[i] = x[i] ;
 }
 redraw();
}

void Lms_glWindow::SetData(float* y)
{
 for (int i = 0; i < data_size; i++)
  data_y[i] = y[i];

 redraw();
}

void Lms_glWindow::SetData(float* y, float* x, int size)
{
	InitData(size);
	SetData(y,x);
}

void Lms_glWindow::SetData(float* y,int size, float xmin, float xmax)
{
	InitData(size, xmin, xmax);
	SetData(y);
 }

void Lms_glWindow::SetDisplayLimits(float xmin, float xmax, float ymin, float ymax, bool reinit_x)
{
    if (xmin >= xmax)
        return;
    if (ymin >= ymax)
        return;
        
    if (reinit_x && data_x != NULL)
    {
         data_xmin = xmin;
         data_xstep = (xmax-xmin)/data_size;
         float f = data_xmin;
         for (int i =0; i < data_size; i++)
         {
                data_x[i] = f;
                f += data_xstep;
         }
    }
    
    display_ymin = ymin;
    display_xmin = xmin;
    display_ymax = ymax;
    display_xmax = xmax;  
    FixGrid();
}

void Lms_glWindow::SetGridSpacing(float xspacing, float yspacing)
{
  grid_xspacing =xspacing;
  grid_yspacing =yspacing;  
  FixGrid();
}


void Lms_glWindow::resize(int X,int Y,int W,int H) {
        Fl_Gl_Window::resize(X,Y,W,H);
        FixViewport(W,H);
        redraw();
    }




