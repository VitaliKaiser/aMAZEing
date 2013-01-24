#include "Common.h"
#include "CTimer.h"

using namespace std;

bool CreateShaderFromFile(const char* Path, GLhandleARB shader)
{
	char* sourceCode = NULL;

	size_t sourceLength;

	LoadProgram(Path, &sourceCode, &sourceLength);

	if(sourceCode == NULL)
		return false;

	glShaderSourceARB(shader, 1, (const char**)&sourceCode, NULL);


	if(!CompileGLSLShader(shader))
	{
		delete [] sourceCode;
		cout<<"Failed to compile shader: "<<Path<<endl;
		return false;
	}

	delete [] sourceCode;
	return true;
}

bool CompileGLSLShader(GLhandleARB obj)
{

	GLint success;
	glCompileShaderARB(obj);	
	glGetShaderiv(obj, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE)
	{
		int infoLogLength = 0;
		char infoLog[1024];
		cout<<"There were compile errors:"<<endl;
		glGetShaderInfoLog(obj, 1024, &infoLogLength, infoLog);
		if(infoLogLength > 0)
			cout<<infoLog<<endl;

		return false;
	}

	return true;
}

bool LinkGLSLProgram(GLhandleARB program)
{
	GLint success;

	glLinkProgramARB(program);

    CHECK_FOR_OGL_ERROR();
	glGetProgramiv(program, GL_LINK_STATUS, &success);
    CHECK_FOR_OGL_ERROR();

	if(success == GL_FALSE)
	{
		int infoLogLength = 0;
		char infoLog[1024];
		cout<<"There were link errors:"<<endl;
		glGetProgramInfoLog(program, 1024, &infoLogLength, infoLog);
		if(infoLogLength > 0)
			cout<<infoLog<<endl;

		return false;
	}

	return true;
}

void PrintBuildLog(cl_program Program, cl_device_id Device)
{
	cl_build_status buildStatus;
	clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &buildStatus, NULL);
	if(buildStatus == CL_SUCCESS)
		return;

	//there were some errors.
	char* buildLog;
	size_t logSize;
	clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
	buildLog = new char[logSize + 1];

	clGetProgramBuildInfo(Program, Device, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
	buildLog[logSize] = '\0';

	cout<<"There were build errors:"<<endl;
	cout<<buildLog<<endl;

	delete [] buildLog;
}

void LoadProgram(const char* Path, char** pSource, size_t* SourceSize)
{
	FILE* pFileStream = NULL;

	// open the OpenCL source code file
    #ifdef _WIN32   // Windows version
        if(fopen_s(&pFileStream, Path, "rb") != 0) 
        {       
            cout<<"File not found: "<<Path;
			return;
        }
    #else           // Linux version
        pFileStream = fopen(Path, "rb");
        if(pFileStream == 0) 
        {       
            cout<<"File not found: "<<Path;
			return;
        }
    #endif

	//get the length of the source code
	fseek(pFileStream, 0, SEEK_END);
	*SourceSize = ftell(pFileStream);
	fseek(pFileStream, 0, SEEK_SET);

	*pSource = new char[*SourceSize + 1];
	fread(*pSource, *SourceSize, 1, pFileStream);
	fclose(pFileStream);
	(*pSource)[*SourceSize] = '\0';
}

size_t GetGlobalWorkSize(size_t DataSize, size_t LocalWorkSize)
{
	size_t r = DataSize % LocalWorkSize;
	if(r == 0)
	{
		return DataSize;
	}
	else
	{
		return DataSize + LocalWorkSize - r;
	}
}

void GetGlobalWorkSize3(size_t DataSize[3], size_t LocalWorkSize[3], size_t GlobalWorkSize[3])
{
	for(int i = 0; i < 3; i++)
	{
	
		size_t r = DataSize[i] % LocalWorkSize[i];
		if(r == 0)
		{
			GlobalWorkSize[i] = DataSize[i];
		}
		else
		{
			GlobalWorkSize[i] = DataSize[i] + LocalWorkSize[i] - r;
		}

	}
}

double RunKernelNTimes(cl_command_queue CommandQueue, cl_kernel Kernel, cl_uint Dimensions, const size_t* pGlobalWorkSize,
						const size_t* pLocalWorkSize, unsigned int NIterations)
{
	CTimer timer;
	cl_int clErr;

	//wait until the command queue is empty... iniefficient but allows accurate timing
	clErr = clFinish(CommandQueue);

	timer.Start();

	//run the kernel N times
	for(unsigned int i = 0; i < NIterations; i++)
	{
		clErr |= clEnqueueNDRangeKernel(CommandQueue, Kernel, Dimensions, NULL, pGlobalWorkSize, pLocalWorkSize, 0, NULL, NULL);
	}
	//wait until the command queue is empty again
	clErr |= clFinish(CommandQueue);

	timer.Stop();

	if(clErr != CL_SUCCESS)
	{
		cout<<"kernel execution failure"<<endl;
		return -1;
	}

	double ms = 1000 * timer.GetElapsedTime() / double(NIterations);

	return ms;
}

void RGBToGrayScale(float R, float G, float B, float& Gray)
{
	Gray = 0.3f * R + 0.59f * G + 0.11f * B;
}

unsigned char To8BitChannel(float Channel)
{
	Channel = Channel * 255.0f;
	if(Channel > 255.0f)
		Channel = 255.0f;
	else if(Channel < 0.0f)
		Channel = 0;

	return (unsigned char)(Channel);
}
