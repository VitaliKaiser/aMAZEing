#pragma once
#include "IAssignment.h"

#define NUM_BANKS	32

class Simple :
	public IAssignment
{
protected:

	void renderCube(struct float3 center, float size);
	void renderRaster();


	cl_program			m_ScanP;
	cl_kernel			m_ScanNaiveKernel;
	cl_kernel			m_ScanWorkEfficientKernel;
	cl_kernel			m_ScanWorkEfficientAddKernel;


	bool				m_KeyboardMask[255];

	// mouse
	int					m_MouseState;
	int					m_Buttons;
	int					m_PrevX;
	int					m_PrevY;

	//for camera handling
	float				m_RotateX;
	float				m_RotateY;
	float				m_TranslateZ;



public:
	Simple(void);
	~Simple(void);

	virtual bool InitResources(cl_device_id Device, cl_context Context);
	
	virtual void ReleaseResources();

	virtual void ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]);

	virtual void Render();

	virtual void OnKeyboard(unsigned char Key, int X, int Y);

	virtual void OnKeyUp(unsigned char Key, int X, int Y);

	virtual void OnMouse(int Button, int State, int X, int Y);

	virtual void OnMouseMove(int X, int Y);

	virtual void OnIdle(double Time, float ElapsedTime);

	virtual void OnWindowResized(int Width, int Height);
};

