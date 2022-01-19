#include <stdio.h>
#include <windows.h>

#include <GL/freeglut.h>

#define _USE_MATH_DEFINES

#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

typedef struct {
	double x;
	double y;
	double z;
} dPoint3D;

typedef struct {
	int t1;
	int t2;
	int t3;
} dPolyEl3D;

vector<dPoint3D> vertices;
vector<dPolyEl3D> polygons;
dPoint3D polygon_center = {0.0, 0.0, 0.0};
dPoint3D e;
dPoint3D s = { 0.0, 0.0, 1.0 };
dPoint3D os;

vector<dPoint3D> control_polygon;
vector<dPoint3D> bSpline;
vector<dPoint3D> bTan;

int step = 0;

GLuint window;
GLuint width = 800, height = 800;

void draw_bSpline() 
{
	//crta krivulju
	//glBegin(GL_POINTS);
	glBegin(GL_LINE_STRIP);
	{
		for (int i = 0; i < bSpline.size() - 1; i++)
			glVertex3d(bSpline[i].x, bSpline[i].y, bSpline[i].z);
	}
	glEnd();
	return;
}

void draw_shape()
{
	//crta model
	glBegin(GL_LINE_LOOP);
	{
		for (int i = 0; i < polygons.size(); i++) {
			glVertex3d(vertices[polygons[i].t1].x, vertices[polygons[i].t1].y, vertices[polygons[i].t1].z);
			glVertex3d(vertices[polygons[i].t2].x, vertices[polygons[i].t2].y, vertices[polygons[i].t2].z);
			glVertex3d(vertices[polygons[i].t3].x, vertices[polygons[i].t3].y, vertices[polygons[i].t3].z);
		}
	}
	glEnd();
	return;
}

void reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);           
	glLoadIdentity();  

	gluPerspective(40.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);         
	glLoadIdentity();                    

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);		
	glClear(GL_COLOR_BUFFER_BIT);				
	glPointSize(1.0);							
	glColor3f(0.0f, 0.0f, 0.0f);				

	return;
}

dPoint3D norm(dPoint3D v)
{
	double k = pow(pow(s.x, 2.0) + pow(s.y, 2.0) + pow(s.z, 2.0), 0.5);
	v.x /= k;
	v.y /= k;
	v.z /= k;

	return v;
}

void display()
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glTranslated(-15.0, -15.0, -100.0);
	draw_bSpline();

	//translacija sredista tijela u tocku krivulje
	glTranslated(bSpline[step].x, bSpline[step].y, bSpline[step].z);

 	e.x = bTan[step + 1].x - bTan[step].x;
	e.y = bTan[step + 1].y - bTan[step].y;
	e.z = bTan[step + 1].z - bTan[step].z;

	os.x = s.y * e.z - s.z * e.y;
	os.y = -s.x * e.z + s.z * e.x;
	os.z = s.x * e.y - s.y * e.x;

	//računanje kuta rotacije
	
	double fi = acos((s.x * e.x + s.y * e.y + s.z * e.z) / pow(pow(s.x, 2.0) + pow(s.y, 2.0) + pow(s.z, 2.0), 0.5) *
		pow(pow(e.x, 2.0) + pow(e.y, 2.0) + pow(e.z, 2.0), 0.5));
	fi = fi / (2 * M_PI) * 360;

	/*transformacije: 1. translacije tijela u ishodiste
					  2. skaliranje tijela za proizvoljnu konstantu
					  3. rotacija oko krivulje
	 */
	glRotated(fi, os.x, os.y, os.z);
	glScaled(5.0, 5.0, 5.0);
	glTranslated(-polygon_center.x, -polygon_center.y, -polygon_center.z);

	draw_shape();

	step++;
	if (step == bSpline.size() - 1)
		step = 0;

	glutSwapBuffers();
	return;
}

void timer(int t)
{
	glutPostRedisplay();
	glutTimerFunc(20, timer, 0);
	return;
}

int main(int argc, char** argv)
{
	ifstream in_file_1(argv[1]);
	ifstream in_file_2(argv[2]);
	string line;

	if (!in_file_1) exit(1);
	if (!in_file_2) exit(1);

	//citanje kontrolnog poligona
	while (in_file_1 >> line) {
		dPoint3D point;
		in_file_1 >> point.x >> point.y >> point.z;
		control_polygon.push_back(point);
	}

	//citanje modela
	while (in_file_2 >> line) {
		switch (*line.c_str()) {
		case 'v':
			dPoint3D vertex;
			in_file_2 >> vertex.x >> vertex.y >> vertex.z;
			polygon_center.x += vertex.x;
			polygon_center.y += vertex.y;
			polygon_center.z += vertex.z;
			vertices.push_back(vertex);
			break;
		case 'f':
			dPolyEl3D polygon;
			in_file_2 >> polygon.t1 >> polygon.t2 >> polygon.t3;
			polygon.t1--; polygon.t2--; polygon.t3--;
			polygons.push_back(polygon);
			break; 
		}
	}

	in_file_1.close();
	in_file_2.close();

	polygon_center.x /= vertices.size();
	polygon_center.y /= vertices.size();
	polygon_center.z /= vertices.size();

	for (int i = 3; i < control_polygon.size(); i++) {
		//priprema podatkovnih strukturi za racunanje krivulje
		double B[4][4] =
		{
				{-1./6, 1./2, -1./2, 1./6},
				{ 1./2,  -1.,  1./2,   0.},
				{-1./2,   0.,  1./2,   0.},
				{ 1./6, 2./3,  1./6,   0.}
		};

		dPoint3D R[4] = { control_polygon[i-3],  control_polygon[i-2], control_polygon[i-1], control_polygon[i] };

		for (float t = 0; t < 1; t += 0.01) {
			double T[4] = {pow(t, 3), pow(t, 2), pow(t, 1), pow(t, 0)};
			double k1 = T[0] * B[0][0] + T[1] * B[1][0] + T[2] * B[2][0] + T[3] * B[3][0];
			double k2 = T[0] * B[0][1] + T[1] * B[1][1] + T[2] * B[2][1] + T[3] * B[3][1];
			double k3 = T[0] * B[0][2] + T[1] * B[1][2] + T[2] * B[2][2] + T[3] * B[3][2];
			double k4 = T[0] * B[0][3] + T[1] * B[1][3] + T[2] * B[2][3] + T[3] * B[3][3];
			 
			//tocka krivulje
			dPoint3D element;
			element.x = k1 * R[0].x + k2 * R[1].x + k3 * R[2].x + k4 * R[3].x;
			element.y = k1 * R[0].y + k2 * R[1].y + k3 * R[2].y + k4 * R[3].y;
			element.z = k1 * R[0].z + k2 * R[1].z + k3 * R[2].z + k4 * R[3].z;

			bSpline.push_back(element);

			//tangenta na tocku krivulje
			double T2[4] = { 3 * pow(t, 2), 2 * pow(t, 1), 1, 0 };
			k1 = T[0] * B[0][0] + T[1] * B[1][0] + T[2] * B[2][0] + T[3] * B[3][0];
			k2 = T[0] * B[0][1] + T[1] * B[1][1] + T[2] * B[2][1] + T[3] * B[3][1];
			k3 = T[0] * B[0][2] + T[1] * B[1][2] + T[2] * B[2][2] + T[3] * B[3][2];
			k4 = T[0] * B[0][3] + T[1] * B[1][3] + T[2] * B[2][3] + T[3] * B[3][3];

			element.x = k1 * R[0].x + k2 * R[1].x + k3 * R[2].x + k4 * R[3].x;
			element.y = k1 * R[0].y + k2 * R[1].y + k3 * R[2].y + k4 * R[3].y;
			element.z = k1 * R[0].z + k2 * R[1].z + k3 * R[2].z + k4 * R[3].z;

			bTan.push_back(element);
		}
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);

	window = glutCreateWindow("Putanje");

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);

	timer(0);
	glutMainLoop();
	return 0;
}