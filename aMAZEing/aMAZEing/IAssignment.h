/*
Common interface for the assignments.
*/

#ifndef IASSIGNMENT_H
#define IASSIGNMENT_H

#include "Common.h"

class IAssignment
{
public:

	//Init any resources specific to the current task
	virtual bool InitResources(cl_device_id Device, cl_context Context) = 0;

	//release everything allocated in InitResources()
	virtual void ReleaseResources() = 0;

	//Perform calculations on the GPU
	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]) = 0;

	//OpenGL render callback
	virtual void Render() = 0;

	//GLUT keyboard callback
	virtual void OnKeyboard(unsigned char Key, int X, int Y) = 0;

	virtual void OnKeyUp(unsigned char Key, int X, int Y) = 0;

	//GLUT mouse callback
	virtual void OnMouse(int Button, int State, int X, int Y) = 0;

	//GLUT mouse move callback
	virtual void OnMouseMove(int X, int Y) = 0;

	virtual void OnIdle(double Time, float ElapsedTime) = 0;

	virtual void OnWindowResized(int Width, int Height) = 0;
};

#endif
