// aMAZEing.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//
#ifdef _WIN32
    // Headers needed for Windows
    #include <windows.h>
#else
    // Headers needed for Linux
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
   #define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
#else
   #define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
#endif

#include "stdafx.h"

//For using OpenGL
#include "GL/glew.h"
#include "GL/glut.h"

// All OpenCL headers
#if defined (__APPLE__) || defined(MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/opencl.h>
#endif

#ifdef linux
    #if defined (__APPLE__) || defined(MACOSX)
        #include <OpenGL/OpenGL.h>
        #include <GLUT/glut.h>
    #else
//        #include <GL/gl.h>
        #include <GL/glx.h>
    #endif
#endif


#include "Common.h"
#include "CTimer.h"
#include "IAssignment.h"
#include "Simple.h"

using namespace std;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables and forward declarations

cl_context			g_CLContext			= NULL;
cl_command_queue	g_CLCommandQueue	= NULL;
cl_platform_id		g_CLPlatform		= NULL;
cl_device_id		g_CLDevice			= NULL;

//int					g_WindowWidth		= 1024;
//int					g_WindowHeight		= 768;
int					g_WindowWidth		= 640;
int					g_WindowHeight		= 480;
int					g_hGLUTWindow		= 0;

size_t				g_LocalWorkSize[3];

IAssignment*		g_pAssignment		= NULL;

CTimer				g_Timer;
double				g_LastTime			= -1;



bool InitCL();
bool InitGL(int argc, char** argv);

void Render();
void OnKeyboard(unsigned char Key, int X, int Y);
void OnKeyUp(unsigned char Key, int X, int Y);
void OnMouse(int Button, int State, int X, int Y);
void OnMouseMove(int X, int Y);
void OnIdle();
void OnReshape(int Width, int Height);

void Cleanup();

int _tmain(int argc, _TCHAR* argv[])
{
	
	if(InitCL() && InitGL(argc,(char**) argv) ) {
		
			g_pAssignment = new Simple();

			if(g_pAssignment->InitResources(g_CLDevice, g_CLContext)) {
				//main loop.
				glutMainLoop();
			} else {
				cout<<"Error during resource allocation. Aborting execution."<<endl;
				g_pAssignment->ReleaseResources();
			}

			delete g_pAssignment;


		cin.get();
		return 0;
	}
	cin.get();
	return 0;
}

bool InitCL()
{
		//error code
	cl_int clError;
	
	//get platform ID
	V_RETURN_FALSE_CL( clGetPlatformIDs(1, &g_CLPlatform, NULL), "Failed to get CL platform ID" );

	//get a reference to the first available GPU device
	V_RETURN_FALSE_CL( clGetDeviceIDs(g_CLPlatform, CL_DEVICE_TYPE_GPU, 1, &g_CLDevice, NULL), "No GPU device found." );

	char deviceName[256];
	V_RETURN_FALSE_CL( clGetDeviceInfo(g_CLDevice, CL_DEVICE_NAME, 256, &deviceName, NULL), "Unable to query device name.");
	cout << "Device: " << deviceName << endl;

	//Create a new OpenCL context on the selected device which supports sharing with OpenGL
	size_t extensionSize;
	V_RETURN_FALSE_CL( clGetDeviceInfo(g_CLDevice, CL_DEVICE_EXTENSIONS, 0, NULL, &extensionSize),
						"Failed to get OpenCL extensions" );
	char* extensions = new char[extensionSize];
	V_RETURN_FALSE_CL( clGetDeviceInfo(g_CLDevice, CL_DEVICE_EXTENSIONS, extensionSize, extensions, &extensionSize),
						"Failed to get OpenCL extensions" );
	std::string strExtensions(extensions);
	delete [] extensions;

	//the extension string is space delimited
    size_t szOldPos = 0;
    size_t szSpacePos = strExtensions.find(' ', szOldPos); // extensions string is space delimited
	bool sharingSupported = false;
	while (szSpacePos != strExtensions.npos)
    {
	    if( strcmp(GL_SHARING_EXTENSION, strExtensions.substr(szOldPos, szSpacePos - szOldPos).c_str()) == 0 ) 
        {
	        sharingSupported = true;
			break;
        }
        do 
        {
	        szOldPos = szSpacePos + 1;
	        szSpacePos = strExtensions.find(' ', szOldPos);
        } while (szSpacePos == szOldPos);
	}

	if(!sharingSupported)
	{
		cout<<"OpenGL - OpenCL context sharing is not supported."<<endl;
		return false;
	}

        // Define OS-specific context properties and create the OpenCL context
        #if defined (__APPLE__)
            CGLContextObj kCGLContext = CGLGetCurrentContext();
            CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
            cl_context_properties props[] = 
            {
                CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup, 
                0 
            };
            g_CLContext = clCreateContext(props, 0,0, NULL, NULL, &ciErrNum);
        #else
            #ifndef _WIN32
                cl_context_properties props[] = 
                {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), 
                    CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), 
                    CL_CONTEXT_PLATFORM, (cl_context_properties)g_CLPlatform, 
                    0
                };
                g_CLContext = clCreateContext(props, 1, &g_CLDevice, NULL, NULL, &clError);
				V_RETURN_FALSE_CL(clError, "clCreateContext");
            #else // Win32
                cl_context_properties props[] = 
                {
                    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), 
                    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), 
                    CL_CONTEXT_PLATFORM, (cl_context_properties)g_CLPlatform, 
                    0
                };
				//TODO
				//g_CLContext = clCreateContext(props, 1, &g_CLDevice, NULL, NULL, &clError);
				g_CLContext = clCreateContext(0,1,&g_CLDevice,NULL,NULL, &clError);
				V_RETURN_FALSE_CL(clError, "clCreateContext ELSE");
            #endif
        #endif

	//Finally, create the command queue. All the asynchronous commands to the device will be issued
	//from the CPU into this queue. This way the host program can continue the execution until some results
	//from that device are needed.
				
	g_CLCommandQueue = clCreateCommandQueue(g_CLContext, g_CLDevice, 0, &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create the command queue in the context");

	cout<<"OpenCL context initialized."<<endl;
	return true;
}

bool InitGL(int argc, char** argv)
{
    // initialize GLUT 
    glutInit(&argc, (char**)argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(g_WindowWidth, g_WindowHeight);
    g_hGLUTWindow = glutCreateWindow("aMAZING GPU Labyrinth");

    // register GLUT callback functions
	glutDisplayFunc(Render);
    glutKeyboardFunc(OnKeyboard);
	glutKeyboardUpFunc(OnKeyUp);
	glutMouseFunc(OnMouse);
	glutMotionFunc(OnMouseMove);
	glutIdleFunc(OnIdle);
	glutReshapeFunc(OnReshape);

	//call Cleanup when terminating the application
	atexit(Cleanup);

	// initialize necessary OpenGL extensions
    glewInit();
   if(!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"))
   {
	   cout<<"Missing OpenGL extension: GL_VERSION_2_0 GL_ARB_pixel_buffer_object"<<endl;
	   return false;
   }

    // default initialization
    glClearColor(0.0, 0.0, 0.0, 1.0f);
	glClearDepth(1.0f);
    glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    // viewport
    glViewport(0, 0, g_WindowWidth, g_WindowHeight);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)g_WindowWidth / (GLfloat) g_WindowHeight, 0.1, 10.0);

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -2.0f);
    //glRotatef(rotate_x, 1.0, 0.0, 0.0);
    //glRotatef(rotate_y, 0.0, 1.0, 0.0);


	glEnable(GL_TEXTURE_2D);

	cout<<"OpenGL context initialized."<<endl;

	return true;
}


// GLUT rendering callback
void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(g_pAssignment)
	{
		g_pAssignment->ComputeGPU(g_CLContext, g_CLCommandQueue, g_LocalWorkSize);
		g_pAssignment->Render();
	}

 //    flip backbuffer to screen
    glutSwapBuffers();
    glutPostRedisplay();
}

// GLUT keyboard callback
void OnKeyboard(unsigned char Key, int X, int Y)
{
    switch(Key) 
    {
        case 27: // escape quits
	        exit(0);
            break;
    }

	if(g_pAssignment)
		g_pAssignment->OnKeyboard(Key, X, Y);
}

void OnKeyUp(unsigned char Key, int X, int Y)
{
	if(g_pAssignment)
		g_pAssignment->OnKeyUp(Key, X, Y);
}

//GLUT mouse button callback
void OnMouse(int Button, int State, int X, int Y)
{
	if(g_pAssignment)
		g_pAssignment->OnMouse(Button, State, X, Y);
}

//GLUT mouse motion callback
void OnMouseMove(int X, int Y)
{
	if(g_pAssignment)
		g_pAssignment->OnMouseMove(X, Y);
}

void OnIdle()
{
	if(g_LastTime < 0)
	{
		//start timer
		g_Timer.Start();
		g_LastTime = 0;
	}
	else
	{
		g_Timer.Stop();
		double time = g_Timer.GetElapsedTime();
		float elapsedTime = time - g_LastTime;
		g_LastTime = time;

		if(g_pAssignment)
			g_pAssignment->OnIdle(time, elapsedTime);
	}

}

void OnReshape(int Width, int Height)
{
	if(g_pAssignment)
		g_pAssignment->OnWindowResized(Width, Height);
}

void Cleanup()
{
	if(g_pAssignment)
		g_pAssignment->ReleaseResources();

	//SAFE_DELETE(g_pAssignment);
	SAFE_RELEASE_COMMANDQUEUE(g_CLCommandQueue);
	SAFE_RELEASE_CONTEXT(g_CLContext);
}