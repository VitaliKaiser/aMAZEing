#ifndef COMMON_H
#define COMMON_H

// All OpenCL headers
#if defined(WIN32)
    #include <CL/opencl.h>
#elif defined (__APPLE__) || defined(MACOSX)
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
    #include <CL/cl_gl.h>
#endif 

#include <stdlib.h>
#include <math.h>
#include <string>
#include <iostream>

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


//For using OpenGL
#include "GL/glew.h"
#include "GL/glut.h"


static const char* errorToString(cl_int error)
{
        switch(error)
        {
#define CL_ERROR(x) case (x): return #x;
                CL_ERROR(CL_SUCCESS);
                CL_ERROR(CL_DEVICE_NOT_FOUND);
                CL_ERROR(CL_DEVICE_NOT_AVAILABLE);
                CL_ERROR(CL_COMPILER_NOT_AVAILABLE);
                CL_ERROR(CL_MEM_OBJECT_ALLOCATION_FAILURE);
                CL_ERROR(CL_OUT_OF_RESOURCES);
                CL_ERROR(CL_OUT_OF_HOST_MEMORY);
                CL_ERROR(CL_PROFILING_INFO_NOT_AVAILABLE);
                CL_ERROR(CL_MEM_COPY_OVERLAP);
                CL_ERROR(CL_IMAGE_FORMAT_MISMATCH);
                CL_ERROR(CL_IMAGE_FORMAT_NOT_SUPPORTED);
                CL_ERROR(CL_BUILD_PROGRAM_FAILURE);
                CL_ERROR(CL_MAP_FAILURE);
                CL_ERROR(CL_INVALID_VALUE);
                CL_ERROR(CL_INVALID_DEVICE_TYPE);
                CL_ERROR(CL_INVALID_PLATFORM);
                CL_ERROR(CL_INVALID_DEVICE);
                CL_ERROR(CL_INVALID_CONTEXT);
                CL_ERROR(CL_INVALID_QUEUE_PROPERTIES);
                CL_ERROR(CL_INVALID_COMMAND_QUEUE);
                CL_ERROR(CL_INVALID_HOST_PTR);
                CL_ERROR(CL_INVALID_MEM_OBJECT);
                CL_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
                CL_ERROR(CL_INVALID_IMAGE_SIZE);
                CL_ERROR(CL_INVALID_SAMPLER);
                CL_ERROR(CL_INVALID_BINARY);
                CL_ERROR(CL_INVALID_BUILD_OPTIONS);
                CL_ERROR(CL_INVALID_PROGRAM);
                CL_ERROR(CL_INVALID_PROGRAM_EXECUTABLE);
                CL_ERROR(CL_INVALID_KERNEL_NAME);
                CL_ERROR(CL_INVALID_KERNEL_DEFINITION);
                CL_ERROR(CL_INVALID_KERNEL);
                CL_ERROR(CL_INVALID_ARG_INDEX);
                CL_ERROR(CL_INVALID_ARG_VALUE);
                CL_ERROR(CL_INVALID_ARG_SIZE);
                CL_ERROR(CL_INVALID_KERNEL_ARGS);
                CL_ERROR(CL_INVALID_WORK_DIMENSION);
                CL_ERROR(CL_INVALID_WORK_GROUP_SIZE);
                CL_ERROR(CL_INVALID_WORK_ITEM_SIZE);
                CL_ERROR(CL_INVALID_GLOBAL_OFFSET);
                CL_ERROR(CL_INVALID_EVENT_WAIT_LIST);
                CL_ERROR(CL_INVALID_EVENT);
                CL_ERROR(CL_INVALID_OPERATION);
                CL_ERROR(CL_INVALID_GL_OBJECT);
                CL_ERROR(CL_INVALID_BUFFER_SIZE);
                CL_ERROR(CL_INVALID_MIP_LEVEL);
				

#undef CL_ERROR
        default:
                return "Unknown error code";
        }
   }

#define CHECK_FOR_OGL_ERROR()                                  \
   do {                                                        \
     GLenum err;                                               \
     err = glGetError();                                       \
     if (err != GL_NO_ERROR)                                   \
     {                                                         \
       if (err == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)        \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: Invalid Framebuffer Operation\n",\
                 __FILE__, __LINE__);                          \
       }                                                       \
       else                                                    \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: %s\n",               \
                 __FILE__, __LINE__, gluErrorString(err));     \
       }                                                       \
     }                                                         \
   } while(0)

#define V_RETURN_OGL_ERROR()                            \
   do{                                                        \
     GLenum err;                                               \
     err = glGetError();                                       \
	if (err != GL_NO_ERROR)                                   \
     {                                                         \
       if (err == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)        \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: Invalid Framebuffer Operation\n",\
                 __FILE__, __LINE__);                          \
       }                                                       \
       else                                                    \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: %s\n",               \
                 __FILE__, __LINE__, gluErrorString(err));     \
       }                                                       \
       return false;                                           \
     }                                                         \
   } while(0)

#define V_RETURN_FALSE_CL(expr, errmsg) {cl_int e=(expr);if(CL_SUCCESS!=e){cout<<"Error: "<<errmsg<<" ["<<errorToString(e)<<"]"<<endl; return false; }}
#define V_RETURN_CL(expr, errmsg) {cl_int e=(expr);if(CL_SUCCESS!=e){cout<<"Error: "<<errmsg<<" ["<<errorToString(e)<<"]"<<endl; return; }}

#define SAFE_DELETE(ptr) {if(ptr){ delete ptr; ptr = NULL; }}
#define SAFE_DELETE_ARRAY(x) {if(x){delete [] x; x = NULL;}}

#define SAFE_RELEASE_KERNEL(ptr) {if(ptr){ clReleaseKernel(ptr); ptr = NULL; }}
#define SAFE_RELEASE_PROGRAM(ptr) {if(ptr){ clReleaseProgram(ptr); ptr = NULL; }}
#define SAFE_RELEASE_MEMOBJECT(ptr) {if(ptr){ clReleaseMemObject(ptr); ptr = NULL; }}
#define SAFE_RELEASE_SAMPLER(ptr) {if(ptr){ clReleaseSampler(ptr); ptr = NULL; }}
#define SAFE_RELEASE_CONTEXT(ptr) {if(ptr){ clReleaseContext(ptr); ptr = NULL; }}
#define SAFE_RELEASE_COMMANDQUEUE(ptr) {if(ptr){ clReleaseCommandQueue(ptr); ptr = NULL; }}


#define SAFE_RELEASE_GL_BUFFER(ptr){if(ptr){ glBindBuffer(1, ptr); glDeleteBuffers(1, &ptr); ptr = NULL; }}
#define SAFE_RELEASE_GL_SHADER(ptr){if(ptr){ glDeleteShader(ptr); ptr = NULL; }}
#define SAFE_RELEASE_GL_PROGRAM(ptr){if(ptr){ glDeleteProgram(ptr); ptr = NULL; }}

//this utility function gets building error messages for an OpenCL program object
void PrintBuildLog(cl_program Program, cl_device_id Device);

//loads an OpenCL program from a file to the memory (as a string)
void LoadProgram(const char* Path, char** pSource, size_t* SourceSize);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//OpenGL utility functions
bool CreateShaderFromFile(const char* Path, GLhandleARB shader);

//utility function which attempts to compile a GLSL shader, then prints the error messages on failure
bool CompileGLSLShader(GLhandleARB shader);

bool LinkGLSLProgram(GLhandleARB program);

/////////////////////////////////////////////////////////////////////////////////////////////////////


//it is also a common task to determine how many OpenCL work groups are needed to run over a given dataset.
//If the size of the work group is given, we can round up the data size to be multiple of this number.
size_t GetGlobalWorkSize(size_t DataSize, size_t LocalWorkSize);

//to accurately measure the kernel time, we check the value of the sytem time before
//and after the kernel is executed. The scheduling cost of the kernel can be amortized if we enqueue
//the kernel multiple times. If your kernel is simple and fast, use a high number of iterations!
//this function returns the measured execution time of the kernel in milliseconds
double RunKernelNTimes(cl_command_queue CommandQueue, cl_kernel Kernel, cl_uint Dimensions, const size_t* pGlobalWorkSize,
						const size_t* pLocalWorkSize, unsigned int NIterations);

//helper functions for color conversion
void RGBToGrayScale(float R, float G, float B, float& Gray);

unsigned char To8BitChannel(float Channel);

#endif
