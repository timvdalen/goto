/*
 * Video subsystem -- see header file
 */

#if defined _WIN32
	#include <gl\freeglut.h>
#elif defined __APPLE__
	#include <GLUT/glut.h>
	#include <ApplicationServices/ApplicationServices.h>
	#define glutLeaveMainLoop(); exit(0);
#else
	#include <GL/freeglut.h>
#endif

#include <cstdio>
#include <set>

#include "objects.h"
#include "video.h"
#include "game.h"
#include "structures.h"
#include "world.h"
#include "collision.h"

#define CALL(x, ...) { if (x) (x)(__VA_ARGS__); }

#define PRIV(T,x) T *x = (T *) data;

namespace Video {

using namespace std;
using namespace Base::Alias;

void(*OnFrame) () = NULL;

double GameSpeed = 60.0;
FPS fps;

//------------------------------------------------------------------------------

void Initialize(int* argc, char *argv[])
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
}

//------------------------------------------------------------------------------

void Terminate()
{
	Window::windows.clear();
}

//------------------------------------------------------------------------------

void StartEventLoop()
{
	glutDisplayFunc(Window::display);
	glutReshapeFunc(Window::resize);
	glutTimerFunc((int) (1000 / GameSpeed), Window::timer, 0);

	glutMainLoop();
}

//------------------------------------------------------------------------------

void StopEventLoop()
{
    if(glutGameModeGet(GLUT_GAME_MODE_ACTIVE)){
        glutLeaveGameMode();
    }
	glutLeaveMainLoop();
}

//------------------------------------------------------------------------------

dword ElapsedTime()
{
	return glutGet(GLUT_ELAPSED_TIME);
}

//------------------------------------------------------------------------------

double CurrentFPS()
{
	return fps;
}

//==============================================================================

struct Plane{
	Vd normal;//Pointing to the outside of the plane
	Pd origin;
};

struct ViewVolume{
	Plane* p;
	ViewVolume() : p(NULL){}
	
};
ViewVolume vv;

//------------------------------------------------------------------------------
void loadViewVolume(Point<double>* p){
	clearViewVolume();
	Plane* planes = new Plane[6];
	//Ordened in such a way that the earlier planes will likely cull more
	//front plane
	planes[0].normal = ~(((Vd)(p[2]-p[0]))*((Vd)(p[1]-p[0])));
	planes[0].origin = p[0];
	//left plane
	planes[1].normal = ~(((Vd)(p[4]-p[0]))*((Vd)(p[3]-p[0])));
	planes[1].origin = p[0];
	
	//right plane
	planes[2].normal = ~(((Vd)(p[2]-p[1]))*((Vd)(p[5]-p[1])));
	planes[2].origin = p[1];
	
	//down plane
	planes[3].normal = ~(((Vd)(p[1]-p[0]))*((Vd)(p[4]-p[0])));
	planes[3].origin = p[0];
	
	//top plane
	planes[4].normal = ~(((Vd)(p[7]-p[3]))*((Vd)(p[2]-p[3])));
	planes[4].origin = p[3];
	
	//back
	planes[5].normal = ~(((Vd)(p[5]-p[4]))*((Vd)(p[7]-p[4])));
	planes[5].origin = p[4];
/*	
	printf("(%f, %f, %f) :: (%f, %f, %f)\n", planes[0].normal.x,planes[0].normal.y,planes[0].normal.z,
			planes[0].origin.x,planes[0].origin.y, planes[0].origin.z);
			
	printf("(%f, %f, %f) :: (%f, %f, %f)\n", planes[1].normal.x,planes[1].normal.y,planes[1].normal.z,
			planes[1].origin.x,planes[1].origin.y, planes[1].origin.z);
			
	printf("(%f, %f, %f) :: (%f, %f, %f)\n", planes[2].normal.x,planes[2].normal.y,planes[2].normal.z,
			planes[2].origin.x,planes[2].origin.y, planes[2].origin.z);
			
	printf("(%f, %f, %f) :: (%f, %f, %f)\n", planes[3].normal.x,planes[3].normal.y,planes[3].normal.z,
			planes[3].origin.x,planes[3].origin.y, planes[3].origin.z);
			
	printf("(%f, %f, %f) :: (%f, %f, %f)\n", planes[4].normal.x,planes[4].normal.y,planes[4].normal.z,
			planes[4].origin.x,planes[4].origin.y, planes[4].origin.z);
	
	printf("(%f, %f, %f) :: (%f, %f, %f)\n", planes[5].normal.x,planes[5].normal.y,planes[5].normal.z,
			planes[5].origin.x,planes[5].origin.y, planes[5].origin.z);
	printf("---\n");*/
	vv.p = planes;
}

//------------------------------------------------------------------------------

void loadOrthogonalVolume(double left, double right, double bottom, double top, double depth, double overSizing){    
	double left_x		= left + 0.5*(right -left)*overSizing;
	double right_x		= right + 0.5*(right - left)*overSizing;
	double bottom_y		= bottom + 0.5*(top - bottom)*overSizing;
	double top_y		= top + 0.5*(top - bottom)*overSizing;
	double near_z		= depth*(-overSizing*0.5);
	double far_z		= depth*(1+overSizing*0.5);
	Point<double>* p	= new Point<double>[8];
	p[0]				= Point<double>(left_x, bottom_y, near_z);
	p[1]				= Point<double>(right_x, bottom_y, near_z);
	p[2]				= Point<double>(right_x, top_y, near_z);
	p[3]				= Point<double>(left_x, top_y, near_z);
	p[4]				= Point<double>(left_x, bottom_y, far_z);
	p[5]				= Point<double>(right_x, bottom_y, far_z);
	p[6]				= Point<double>(right_x, top_y, far_z);
	p[7]				= Point<double>(left_x, top_y, far_z);
	loadViewVolume(p);
	delete[] p;
}

//------------------------------------------------------------------------------

void loadPerspectiveVolume(double fovy, double aspect, double depth, double overSizing){
	//Bounding volume:
	//         _________/ |
	// y       /       __/|
	/// \     /      /    |
	// |   /|    /|       |
	// | ov |  oc |       |
	// |   \|    \|       |
	// |     \      \     |
	// |       \      \ __|
	// |         \ _____ \|
	// |                 \|
	//  --------------------> z
	//     |nv    |nc    |farplane
	// ov = viewing origin, nv = viewing nearplane
	// oc = camera origin, nc = camera nearplane
	// angles are given by fovy
	double o;
	if(overSizing > 1.0/100000.0){
		o				= -depth*overSizing;
	}else{
		o				= -depth/100000.0;
	}
	double near_z		= -depth*overSizing*0.5;
	double near_top_y	= (near_z-o)*tan(fovy/2.0);
	double near_bot_y	= -near_top_y;
	double near_right_x	= near_top_y*aspect;
	double near_left_x 	= -near_right_x;
	double far_z		= depth*(overSizing*0.5+1);
	double far_top_y	= (far_z-o)*tan(fovy/2.0);
	double far_bot_y	= -far_top_y;
	double far_right_x	= far_top_y*aspect;
	double far_left_x 	= -far_right_x;
	Point<double>* p	= new Point<double>[8];
	p[0]				= Point<double>(near_left_x	,near_bot_y	,near_z);//left, bottom, near
	p[1]				= Point<double>(near_right_x,near_bot_y	,near_z);//right, bottom, near
	p[2]				= Point<double>(near_right_x,near_top_y	,near_z);//right, top, near
	p[3]				= Point<double>(near_left_x	,near_top_y	,near_z);//left, top, near
	p[4]				= Point<double>(far_left_x	,far_bot_y	,far_z);//left, bottom, far
	p[5]				= Point<double>(far_right_x	,far_bot_y	,far_z);//right, bottom, far
	p[6]				= Point<double>(far_right_x	,far_top_y	,far_z);//right , top, far
	p[7]				= Point<double>(far_left_x	,far_top_y	,far_z);//left, top, far
	loadViewVolume(p);
	delete[] p;
}

//------------------------------------------------------------------------------

void clearViewVolume(){
	delete[] vv.p;
	vv.p = NULL;
}
//------------------------------------------------------------------------------

bool outsideViewingVolume(BoundingBox b){
	if(!vv.p){
		printf("not loaded!\n");
		return false;
	}
	//transform boundingbox from object-space to world space
	double mv_m[4][4];
	glGetDoublev(GL_MODELVIEW_MATRIX, mv_m[0]);
	/** /for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			printf("%f\t", mv_m[j][i]);
		}
		printf("|\n|");
	}/**/
	//don't just transform all 8 points, transform the vectors between the vertices
	//gives faster performance
	double dx			= b.rth.x - b.lbl.x;
	Vd vdx				= Vd(dx*mv_m[0][0],dx*mv_m[0][1],-dx*mv_m[0][2]);
	double dy			= b.rth.y - b.lbl.y;
	Vd vdy				= Vd(dy*mv_m[1][0],dy*mv_m[1][1],-dy*mv_m[1][2]);
	double dz			= b.rth.z - b.lbl.z;
	Vd vdz				= Vd(dz*mv_m[2][0],dz*mv_m[2][1],-dz*mv_m[2][2]);
	Pd	p_org			= Pd(	b.lbl.x * mv_m[0][0] + b.lbl.y*mv_m[1][0]	+ b.lbl.z*mv_m[2][0] + mv_m[3][0],
								b.lbl.x * mv_m[0][1] + b.lbl.y*mv_m[1][1]	+ b.lbl.z*mv_m[2][1] + mv_m[3][1],
								-b.lbl.x * mv_m[0][2] - b.lbl.y*mv_m[1][2]	- b.lbl.z*mv_m[2][2] - mv_m[3][2]);
	/** /printf("Points are:\n (%f, %f, %f): (%f, %f, %f), (%f, %f, %f), (%f, %f, %f)", p_org.x, p_org.y, p_org.z, vdx.x,vdx.y,
				vdx.z, vdy.x,vdy.y,	vdy.z, vdz.x,vdz.y,	vdz.z);/**/
	for(int i = 0; i < 6; i++){
		//use the addition properties of the dot-product to construct 
		//	a point which has the most chance to be inside the plane
		Vd v 				= p_org - vv.p[i].origin;
		double extreme_dot	= (v^vv.p[i].normal)			+ 
								fmin(0.0,vdx^vv.p[i].normal)+
								fmin(0.0,vdy^vv.p[i].normal)+
								fmin(0.0,vdz^vv.p[i].normal);
		if(0 < extreme_dot){
//			printf("not drawn!\n");
			return true;
		}
	}
//	printf("just fails the test!\n");
	return false;
}

//==============================================================================

    
std::set<Window *> Window::windows;

struct WindowData
{
	uword width, height;
	double aspect;
	int wid;
    bool fullscreen;
};

//------------------------------------------------------------------------------

Window::Window(uword width, uword height, const char *title,bool fullscreen,
               word xpos, word ypos) : resized(true)
{
	data = (void *) new WindowData;

	PRIV(WindowData, wd);
	wd->width = width;
	wd->height = height;
	wd->aspect = (double) width / (double) height;
    wd->fullscreen = fullscreen;
    if(fullscreen){
	//	glutGameModeString("800x600");
	    wd->wid = glutEnterGameMode();
	}else{
		glutInitWindowPosition(xpos, ypos);
		glutInitWindowSize(width, height);
	    wd->wid = glutCreateWindow(title);
	}

	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //Clearing screen to get a black screen while loading..
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glutSwapBuffers();
	windows.insert(this);
}

//------------------------------------------------------------------------------

Window::~Window()
{
	windows.erase(this);

	PRIV(WindowData, wd)
	delete wd;
}

//------------------------------------------------------------------------------

void Window::select()
{
	PRIV(WindowData, wd)
	glutSetWindow(wd->wid);
}

//------------------------------------------------------------------------------

void Window::render()
{
	PRIV(WindowData, wd)

	select();

	glClear(GL_COLOR_BUFFER_BIT);

	vector<Viewport *>::iterator vit;
	for (vit = viewports.begin(); vit != viewports.end(); ++vit)
	{
		(*vit)->select(this);
		(*vit)->render();
	}

	glutSwapBuffers();
}

//------------------------------------------------------------------------------

void Window::size(uword &width, uword &height)
{
	PRIV(WindowData, wd)

	select();

	if (width || height)
	{
		if (!width)  width = wd->width;
		if (!height) height = wd->height;
		glutReshapeWindow(width, height);
		return;
	}

	width = wd->width;
	height = wd->height;
}

//------------------------------------------------------------------------------

void Window::display()
{
	set<Window *>::iterator wit;
	for (wit = windows.begin(); wit != windows.end(); ++wit)
		(*wit)->render();
}

//------------------------------------------------------------------------------

void Window::resize(int width, int height)
{
	// Currently only sets the first window
	if (windows.empty()) return;

	void *data = (*windows.begin())->data;
	PRIV(WindowData, wd)

	wd->width = width;
	wd->height = height;
	wd->aspect = (double) width / (double) height;
	(*windows.begin())->resized = true;
	(*windows.begin())->render();
}

//------------------------------------------------------------------------------

void Window::timer(int state)
{
	fps();
	CALL(OnFrame);
	for (set<Window *>::iterator it = windows.begin(); it != windows.end(); ++it)
		(*it)->resized = false;

	glutTimerFunc((int) (1000 / GameSpeed), Window::timer, 0);
}

//==============================================================================

void Camera::lookAt(const Point<double> &target)
{
    Vd dir = ~Vd(target-origin);

    double len = sqrt(dir.x * dir.x + dir.y * dir.y);
    double pitch = atan2(dir.z, len);
    double pan = atan2(dir.x, dir.y);

    objective = Qd(Rd(-pitch, Vd(1, 0, 0))) * Qd(Rd(pan, Vd(0, 0, 1)));

}


//==============================================================================

struct ViewportData
{
	double x, y;
	double w, h;
	double f, a;
};

//------------------------------------------------------------------------------

Viewport::Viewport(double width, double height, double xpos, double ypos,
                   double fov)
{
	data = (void*) new ViewportData;

	PRIV(ViewportData, vd)

	vd->w = width;
	vd->h = height;
	vd->x = xpos;
	vd->y = ypos;
	vd->f = fov;
	vd->a = width / height;

	if (vd->h < 1)
		vd->h = 1;

}

//------------------------------------------------------------------------------

Viewport::~Viewport()
{
	PRIV(ViewportData, vd)
	delete vd;
}

//------------------------------------------------------------------------------

void Viewport::move(double xpos, double ypos)
{
	PRIV(ViewportData, vd)
	vd->x = xpos;
	vd->y = ypos;
}

//------------------------------------------------------------------------------

void Viewport::resize(double width, double height)
{
	PRIV(ViewportData, vd)

	if ((width < 1) || (height < 1))
		return;

	vd->w = width;
	vd->h = height;
	vd->a = width / height;
}

//------------------------------------------------------------------------------

void Viewport::setfov(double fov)
{
	PRIV(ViewportData, vd)

	vd->f = fov;
}

//------------------------------------------------------------------------------

void Viewport::select(Window *w)
{
	PRIV(ViewportData, vd)
	if (!w) return;

	WindowData *wd = (WindowData *) w->data;
	glViewport((GLint) vd->x * wd->width, (GLint) vd->y * wd->height,
	           (GLint) vd->w * wd->width, (GLint) vd->h * wd->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(vd->f, vd->a * wd->aspect, 0.1f, 1000.0f);
	/** /Video::loadPerspectiveVolume(vd->f, vd->a * wd->aspect, 300.0f);
	/**/Video::loadOrthogonalVolume(-10.0, 10.0, -10.0, 10.0, 40.0);/**/
}

//------------------------------------------------------------------------------

void Viewport::render()
{
	if (!world) return;
	PRIV(ViewportData, vd)

	// Set up render
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_DEPTH_BUFFER_BIT);

	// Set up camera
	// camera: Cam.origin(x,y,z) world center(x,y,z) Z-axis(x,y,z)
	//Point<double> p = cam.origin + (~cam.objective * Vector<double>(1,0,0));
	//gluLookAt(cam.origin.x, cam.origin.y, cam.origin.z, p.x, p.y, p.z, 0, 0, 1);

	// I might as well define the matrix myself...
	{
		 // Set up xyz axes how I like them
		double m1[16] = { 1,  0,  0,  0,
		                  0,  0, -1,  0,
					      0,  1,  0,  0,
					      0,  0,  0,  1};

		glMultMatrixd(m1);

		// Camera orientation (quaternion to rotation matrix)
		const Quaternion<double> &q = -camera.objective;
		double aa, ab, ac, ad, bb, bc, bd, cc, cd, dd;
		aa = q.a*q.a; ab = q.a*q.b; ac = q.a*q.c; ad = q.a*q.d;
		              bb = q.b*q.b; bc = q.b*q.c; bd = q.b*q.d;
		                            cc = q.c*q.c; cd = q.c*q.d;
		                                          dd = q.d*q.d;

		double m2[16] =
			{aa+bb-cc-dd, 2.0*(bc-ad), 2.0*(bd+ac),           0,
			 2.0*(bc+ad), aa-bb+cc-dd, 2.0*(cd-ab),           0,
			 2.0*(bd-ac), 2.0*(cd+ab), aa-bb-cc+dd,           0,
			           0,           0,           0, aa+bb+cc+dd};

		glMultMatrixd(m2);
		// Camera position
        Point<double> p = TO(Objects::World , world)->getCorrectedOrigin(q, camera.origin);
		glTranslated(-p.x, -p.y, -p.z);
	}

	// Enable lighting
	glEnable(GL_LIGHT0);
    //set lighting in right position
    GLfloat light_pos[4] = {0,0,100,0};
    GLfloat ambientColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};//blueish ambient light
    GLfloat ambientLight[4] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat diffuseLight[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat specularLight[4] = {0,0,0,0};// Should be: {0.4f, 0.4f, 0.4f, 1.0f};//white specular but materials are configured to be to shiny...
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	// Render the world and everything in it
	world->render();
}

//==============================================================================

double FPS::operator()()
{
	frames++;

	int ct = glutGet(GLUT_ELAPSED_TIME);
	int dt = ct - time;
	if (dt > 1000)
	{
		fps = ((double)frames / (double)dt) * 1000.0;
		time = ct;
		frames = 0;
	}

	return fps;
}

//------------------------------------------------------------------------------

} // namespace Video

//------------------------------------------------------------------------------
