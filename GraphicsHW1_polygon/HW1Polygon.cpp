#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

int rightbuttonpressed = 0;
int windowh, windoww; // window size
float vertexx[129], vertexy[129]; // vertex positions
int vnum = 0; // number of vertices
int iscomplete = 0; // 1: complete polygon
int isrotate = 0; // 1: rotate polygon
float centerx, centery; // centroid of polygon
float tmpx, tmpy;

void convert(int x, int y, float* glx, float* gly) {
	// convert window coordinate(x,y) to openGL coordinate(glx, gly)
	(*glx) = (float)x * 2 / windoww - 1;
	(*gly) = 1.0f - (float)y * 2 / windowh;
}

void find_centroid() {
	//calculate area
	float area = 0; // area of polygon
	for (int i = 0; i < vnum - 1; i++) {
		area += vertexx[i] * vertexy[i + 1] - vertexx[i + 1] * vertexy[i];
	}
	area += vertexx[vnum - 1] * vertexy[0] - vertexx[0] * vertexy[vnum - 1];

	area /= 2;

	centerx = centery = 0;
	// calculate centroid
	for (int i = 0; i < vnum - 1; i++) {
		centerx += (vertexx[i] + vertexx[i + 1]) * (vertexx[i] * vertexy[i + 1] - vertexx[i + 1] * vertexy[i]);
		centery += (vertexy[i] + vertexy[i + 1]) * (vertexx[i] * vertexy[i + 1] - vertexx[i + 1] * vertexy[i]);
	}
	centerx += (vertexx[vnum - 1] + vertexx[0]) * (vertexx[vnum - 1] * vertexy[0] - vertexx[0] * vertexy[vnum - 1]);
	centery += (vertexy[vnum - 1] + vertexy[0]) * (vertexx[vnum - 1] * vertexy[0] - vertexx[0] * vertexy[vnum - 1]);

	centerx = centerx / 6.0f / area;
	centery = centery / 6.0f / area;
}

void rotate(int val) {
#define RAD 0.05
	if (isrotate) {
		float nx, ny;
		for (int i = 0; i < vnum; i++) {
			nx = (vertexx[i] - centerx) * cos(RAD) - (vertexy[i] - centery) * sin(RAD);
			ny = (vertexx[i] - centerx) * sin(RAD) + (vertexy[i] - centery) * cos(RAD);
			vertexx[i] = nx + centerx; vertexy[i] = ny + centery;
		}

		glutPostRedisplay();
		glutTimerFunc(100, rotate, 0);
	}
}

void display(void) {
	glClearColor(1, 1, 1, 1); // background: white
	glClear(GL_COLOR_BUFFER_BIT);

	// draw points
	glColor3f(0, 0, 0); // black
	glPointSize(5.0);
	glBegin(GL_POINTS);
	for (int i = 0; i < vnum; i++) {
		glVertex2f(vertexx[i], vertexy[i]);
	}
	glEnd();
	glFlush();

	// draw lines
	glColor3f(1, 0, 0); // red
	if (iscomplete) {
		glBegin(GL_LINE_LOOP);
		for (int i = 1; i < vnum; i++) {
			glVertex2f(vertexx[i - 1], vertexy[i - 1]);
			glVertex2f(vertexx[i], vertexy[i]);
		}
		glEnd();
		glFlush();
	}
	else {
		glBegin(GL_LINE_STRIP);
		for (int i = 1; i < vnum; i++) {
			glVertex2f(vertexx[i - 1], vertexy[i - 1]);
			glVertex2f(vertexx[i], vertexy[i]);
		}
		glEnd();
		glFlush();
	}

	if (isrotate) { // draw centroid
		glColor3f(0, 1, 0); //green
		glBegin(GL_POINTS);
		glVertex2f(centerx, centery);
		glEnd();
		glFlush();
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'p': // close polygon if there are 3 vertices at least
		if (vnum > 2) {
			iscomplete = 1;
			glutPostRedisplay();
		}
		else {
			fprintf(stdout, "Error: p should be pressed when there are at least 3 vertices\n");
		}
		break;
	case 'c': // delete polygon
		if (!isrotate && vnum > 0) {
			vnum = 0;
			iscomplete = 0;
			isrotate = 0;
			glutPostRedisplay();
		}
		break;
	case 'r':
		if (iscomplete) {
			if (isrotate) {
				isrotate = 0;
			}
			else {
				find_centroid();

				isrotate = 1; // start rotating
				glutTimerFunc(100, rotate, 0); // roating animation

			}
			glutPostRedisplay();
		}
		break;
	case 'f':
		glutLeaveMainLoop();
		break;
	}
}

void special(int key, int x, int y) {
# define SENSITIVITY 0.01
	switch (key) {
	case GLUT_KEY_LEFT:
		if (!isrotate && iscomplete) {
			for (int i = 0; i < vnum; i++) {
				vertexx[i] -= SENSITIVITY;
			}
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_RIGHT:
		if (!isrotate && iscomplete) {
			for (int i = 0; i < vnum; i++) {
				vertexx[i] += SENSITIVITY;
			}
		}
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		if (!isrotate && iscomplete) {
			for (int i = 0; i < vnum; i++) {
				vertexy[i] -= SENSITIVITY;
			}
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_UP:
		if (!isrotate && iscomplete) {
			for (int i = 0; i < vnum; i++) {
				vertexy[i] += SENSITIVITY;
			}
			glutPostRedisplay();
		}
		break;
	}
}

void mousepress(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {

		// if shift is pressed
		if (!iscomplete && glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
			// change window coordinates to openGL coordinates
			float glx, gly;
			convert(x, y, &glx, &gly);

			// save vertex position
			vertexx[vnum] = glx; vertexy[vnum] = gly;
			vnum += 1;

			glutPostRedisplay();
		}
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		if (!isrotate && iscomplete) {
			convert(x, y, &tmpx, &tmpy);
			rightbuttonpressed = 1;
		}
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		rightbuttonpressed = 0;
	}
}

void mousemove(int x, int y) {
	static int delay = 0;
	float dx, dy, glx, gly;
	if (rightbuttonpressed) {
		convert(x, y, &glx, &gly);
		dx = glx - tmpx; dy = gly - tmpy; // how much right mouse moved

		for (int i = 0; i < vnum; i++) {
			vertexx[i] += dx;
			vertexy[i] += dy;
		}
		tmpx = glx; tmpy = gly;
		glutPostRedisplay();
	}
}

void reshape(int width, int height) {

	windoww = width;
	windowh = height;
	glViewport(0, 0, width, height); // adjust to window size, maintain size ratio

	glutPostRedisplay();
}

void close(void) {
	fprintf(stdout, "\n^^^ The control is at the close callback function now.\n\n");
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutCloseFunc(close);
}

void initialize_renderer(void) {
	register_callbacks();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = TRUE;
	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 7
void main(int argc, char* argv[]) {
	char program_name[64] = "HW1 다각형 편집 by 20201551 권지은";
	char messages[N_MESSAGE_LINES][256] = {
		"	- Draw dot: SHIFT + L-click",
		"	- Complete polygon: 'p'",
		"	- Move polygon: LEFT, RIGHT, UP, DOWN, R-click",
		"	- Rotate polygon: 'r'",
		"	- Erase polygon: 'c",
		"	- Quit program: 'f'",
		"	- Other operations: window size change"
	};

	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_RGBA);

	glutInitWindowSize(600, 600);
	glutInitWindowPosition(500, 100);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutMainLoop();
	fprintf(stdout, "^^^ The control is at the end of main function now.\n\n");
}