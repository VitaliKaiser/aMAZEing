#include "Simple.h"


float angle = 0.0f;

struct float3 { float x; float y; float z;};

int sizeX = 10;
int sizeY = 10;

int field[100];

bool rGitter = true;

using namespace std;

Simple::Simple(void)
{


}


Simple::~Simple(void)
{
}

//Init any resources specific to the current task
bool Simple::InitResources(cl_device_id Device, cl_context Context) {

	




	//////////////////
	/// Build Kernels

	// Scan kernels
	cl_int clError;
	char* programCode = NULL;
	size_t programSize = 0;
	LoadProgram("Scan.cl", &programCode, &programSize);

	//create program object
	m_ScanP = clCreateProgramWithSource(Context, 1, (const char**) &programCode, &programSize, &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create program from file.");

	//build program
	clError = clBuildProgram(m_ScanP, 1, &Device, NULL, NULL, NULL);
	if(clError != CL_SUCCESS) {
		PrintBuildLog(m_ScanP, Device);
		return false;
	}


	//create kernels
	m_ScanNaiveKernel = clCreateKernel(m_ScanP, "Scan_Naive", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel.");

	m_ScanWorkEfficientKernel = clCreateKernel(m_ScanP, "Scan_WorkEfficient", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Scan_WorkEfficient kernel.");

	m_ScanWorkEfficientAddKernel = clCreateKernel(m_ScanP, "Scan_WorkEfficientAdd", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Scan_WorkEfficientAdd kernel.");

	SAFE_DELETE(programCode);


	////////////////////////
	/// GameCreation 
	int fieldCount = sizeX * sizeY;	

	//Create Field
	for(int i = 0; i < fieldCount;i++) {
		field[i] = 1;
	}
	//Create Maze aka remove fields



	return true;
}




//release everything allocated in InitResources()
void Simple::ReleaseResources() {

	return;
};

//Perform calculations on the GPU
void Simple::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3]) {


	return;
};

//OpenGL render callback
void Simple::Render() {

	if(rGitter) {
		renderRaster();
	}
	// Reset transformations
	glLoadIdentity();
	//Set the camera
	gluLookAt(	0.0f, 0.0f, 10.0f,
			0.0f, 0.0f,  0.0f,
			0.0f, 1.0f,  0.0f);

	//glRotatef(angle, 0.0f, 1.0f, 0.0f);

	glPointSize(10.0);
	glBegin(GL_POINTS);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(0,0,0);
	glEnd();

	glBegin(GL_POLYGON);
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);
 
		glColor3f(0,0,1);
		glVertex3f(1,0,0);
		glVertex3f(1,1,0);
 

	glEnd();



	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(1,1,0);
		glVertex3f(2,1,1);
	glEnd();

	struct float3 center = {2,2,0};
	renderCube(center, 0.4);

	/*glBegin(GL_TRIANGLES);
		glVertex3f(-2.0f,-2.0f, 0.0f);
		glVertex3f( 2.0f, 0.0f, 0.0);
		glVertex3f( 0.0f, 2.0f, 0.0);
	glEnd();*/

	angle+=0.1f;

	return;
};

void Simple::renderRaster() {

}
void Simple::renderCube(struct float3 center, float size) {
		
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINES);
		
		glVertex3f(center.x - size, center.y + size,center.z);
		glVertex3f(center.x + size, center.y + size,center.z);
	glEnd();

	glBegin(GL_LINES);
		
		glVertex3f(center.x + size, center.y + size,center.z);
		glVertex3f(center.x + size, center.y - size,center.z);
	glEnd();

	glBegin(GL_LINES);
		
		glVertex3f(center.x + size, center.y - size,center.z);
		glVertex3f(center.x - size, center.y - size,center.z);		
	glEnd();

	glBegin(GL_LINES);
		
		glVertex3f(center.x - size, center.y - size,center.z);
		glVertex3f(center.x - size, center.y + size,center.z);
	glEnd();


}
//GLUT keyboard callback
void Simple::OnKeyboard(unsigned char Key, int X, int Y) {

	
		m_KeyboardMask[Key] = true;

	//if(Key == 'f')
	//	m_ShowForceField = !m_ShowForceField;
};

void Simple::OnKeyUp(unsigned char Key, int X, int Y) {

	
	m_KeyboardMask[Key] = false;
};

//GLUT mouse callback
void Simple::OnMouse(int Button, int State, int X, int Y) {

	return;

	 if (m_MouseState == GLUT_DOWN) {
        m_Buttons |= 1<<Button;
    } else if (m_MouseState == GLUT_UP) {
        m_Buttons = 0;
    }

	m_PrevX = X;
    m_PrevY = Y;

    glutPostRedisplay();
};

//GLUT mouse move callback
void Simple::OnMouseMove(int X, int Y) {

	return;

	    float dx, dy;
	dx = X - m_PrevX;
    dy = Y - m_PrevY;

    if (m_Buttons & 1) {
		m_RotateX += dy * 0.2;
		m_RotateY += dx * 0.2;
    } 
	m_PrevX = X;
	m_PrevY = Y;

    // set view matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glTranslatef(0.0, 0.0, m_TranslateZ);
	glRotatef(m_RotateY, 0.0, 1.0, 0.0);
	glRotatef(m_RotateX, 1.0, 0.0, 0.0);
	glTranslatef(-0.5, -0.5, -0.5);
    glutPostRedisplay();

};

void Simple::OnIdle(double Time, float ElapsedTime) {

	return;

		//move camera?
	if(m_KeyboardMask['w'])
		m_TranslateZ += 2.f * ElapsedTime;
	if(m_KeyboardMask['s'])
		m_TranslateZ -= 2.f * ElapsedTime;

	if(m_TranslateZ > 0)
		m_TranslateZ = 0;
	
	//set the modelview matrix
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, m_TranslateZ);
    glRotatef(m_RotateY, 0.0, 1.0, 0.0);
    glRotatef(m_RotateX, 1.0, 0.0, 0.0);
	glTranslatef(-0.5, -0.5, -0.5);
    glutPostRedisplay();
};

void Simple::OnWindowResized(int Width, int Height) {

	return;
	    // viewport
    glViewport(0, 0, Width, Height);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)Width / (GLfloat) Height, 0.1, 10.0);

};
