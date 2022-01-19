#include <iostream>
#include <vector>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/freeglut.h>
#include <glm/glm.hpp>

//#define GLM_FORCE_RADIANS
//#include <glm/gtc/matrix_transform.hpp>

#define PARTICLES_NUM 1000

void reshape(int w, int h);
void display();
void keyboard(unsigned char theKey, int mouseX, int mouseY);
void timer(int t);
double unif();

GLuint window;
GLuint width = 500, height = 500;
struct Particle 
{
	glm::dvec2 position;

	glm::dvec4 color;
	double size;
	double life;

	Particle()
		: position(glm::dvec2(unif() / 10, unif() / 10)), color(glm::dvec4(0.0f, 0.0f, 0.0f, 1.0f)), size(unif() / 50), life(0.0f) {}
};

glm::dvec2 source = glm::dvec2(0.0f, 0.0f);
std::vector<Particle> particles = std::vector<Particle>(PARTICLES_NUM);

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);

	window = glutCreateWindow("Cestice");

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	srand(time(NULL));
	timer(0);
	glutMainLoop();

	return 0;
}

void reshape(int w, int h)
{
	width = w; height = h;
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(1.0);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void display()
{
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 3.0f);

	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 3.0f), 
		glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3(0.0f, 1.0f, 0.0f)  
	);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 mvp = projection * view * model;
	*/

	for (int i = 0; i < PARTICLES_NUM; i++) {	
		if (particles[i].life > 0.0f) {
			double size = particles[i].size;
			double x = particles[i].position.x;
			double y = particles[i].position.y;

			glColor3f(particles[i].color.x, particles[i].color.y, particles[i].color.z);
			glBegin(GL_QUADS);
			{
				glVertex2d(x - size, y - size);
				glVertex2d(x + size, y - size);
				glVertex2d(x + size, y + size);
				glVertex2d(x - size, y + size);
			}
			glEnd();
		}
	}

	glutSwapBuffers();
}

void keyboard(unsigned char theKey, int mouseX, int mouseY)
{
	switch (theKey)
	{
	case 'i':
		source.x = unif() / 4;
		source.y = unif() / 4;
		int created = 0;
		for (int j = 0; j < PARTICLES_NUM; j++) {
			if (created > 10) break;

			if (particles[j].life == 0) {
				particles[j].position += source;
				particles[j].color[2] = 1.0f;
				particles[j].life = 1.0f;
				created++;
			}

			if ((j + 1) == PARTICLES_NUM)
				j = 0;
		}
		break;
	}

	glutPostRedisplay();
	return;
}

void timer(int t)
{
	for (int i = 0; i < PARTICLES_NUM; i++) {
		if (particles[i].life > 0) {
				particles[i].position.x += unif()/5;
				particles[i].life -= 0.01;
				particles[i].color -= 0.01;
		}
	}

	glutPostRedisplay();
	glutTimerFunc(20, timer, 0); 
	return;
}

double unif()
{
	double num = (rand() % 1000 - (double)500) / 500;
	num = num * num *num + num;
	return num;
}