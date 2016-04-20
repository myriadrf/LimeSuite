/* 
 * File:   Lms_glWindow.h
 * Author: ignas
 *
 * Created on January 12, 2016, 11:35 AM
 */

#ifndef LMS_GLWINDOW_H
#define	LMS_GLWINDOW_H

#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include <FL/Fl.H>

class Lms_glWindow : public Fl_Gl_Window {
    
    double fg;                       // foreground brightness
    double bg;                       // background brightness
    float *data_x;
    float *data_y;
    int data_size;
    float grid_xspacing;
    float grid_yspacing;
    float display_xmin;
    float display_ymin;
    float display_xmax;
    float display_ymax;
	float data_xmin;
	float data_xstep;
	int graph_type;
	struct { 
			 float x1,x2,y1,y2;
			 bool active;
			} selection;
    // FIX OPENGL VIEWPORT
    //     Do this on init or when window's size is changed
    void FixViewport(int W,int H);   
    // DRAW METHOD
    void draw();
    // HANDLE WINDOW RESIZING
    void resize(int X,int Y,int W,int H);
	void FixGrid();
    static void Timer_CB(void *data);
    
public:
    int handle(int event);
    // OPENGL WINDOW CONSTRUCTOR
	void SetData(float* y);
    void SetData(float* y, float* x);
    void SetData(float* y, float* x, int size);
    void SetData(float* y,int size, float xmin, float xmax);
    void SetDisplayLimits(float xmin, float xmax, float ymin, float ymax, bool reinit_x = false);
    void InitData(int size);
    void InitData(int size,float xmin, float xmax);
    void SetGridSpacing(float xspacing, float yspacing);
    Lms_glWindow(int X,int Y,int W,int H,const char*L=0);   
    ~Lms_glWindow(){};  
};

#endif	/* LMS_GLWINDOW_H */

