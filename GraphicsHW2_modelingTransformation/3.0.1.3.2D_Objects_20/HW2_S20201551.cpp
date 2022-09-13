#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

														  // include glm/*.hpp only if necessary
														  //#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0

int win_height, win_width;

// draw my object - ufo
#define UFO_TOP 0
#define UFO_BOTTOM 1
#define UFO_MIDDLE 2
#define UFO_WINDOW 3

GLfloat ufo_behind_left[3][2] = { {-30.0, 0.0}, {-20.0, 0.0}, {-18.0, 6.0} };
GLfloat ufo_behind_right[3][2] = { {30.0, 0.0}, {20.0, 0.0}, {18.0, 6.0} };
GLfloat ufo_top[180][2];
GLfloat ufo_bottom[31][2];
GLfloat ufo_middle[4][2] = { {-20.0, 0.0}, {-20.0, 5.0}, {20.0, 5.0}, {20.0, 0.0} };
GLfloat ufo_window_middle[4][2] = { {-3.0, 7.0}, {-3.0, 13.0}, {3.0, 13.0}, {3.0, 7.0} };
GLfloat ufo_window_left[7][2] = { {-14.0, 7.0},{-11.0, 13.0},{-10.0, 13.0},{-4.0, 13.0}, {-4.0, 7.0},{-10.0, 7.0}, {-11.0, 7.0} };
GLfloat ufo_window_right[7][2] = { {4.0, 7.0}, {4.0, 13.0}, {10.0, 13.0}, {11.0, 13.0},{14.0, 7.0},{11.0, 7.0},{10.0, 7.0} };

GLfloat ufo_color[4][3] = {
	{0 / 255.0f, 0 / 255.0f, 0 / 255.0f},  //top
	{ 0 / 255.0f,   255 / 255.0f,   213 / 255.0f }, // bottom
	{0 / 255.0f, 0 / 255.0f, 0 / 255.0f}, // middle
	{233 / 255.0f, 255 / 255.0f, 252 / 255.0f} // window
};

GLuint VBO_ufo, VAO_ufo;

void prepare_ufo() {

	for (int i = 0; i < 180; i++) {
		ufo_top[i][0] = 20.0f * cosf(i * TO_RADIAN);
		ufo_top[i][1] = 20.0f * sinf(i * TO_RADIAN);
	}
	for (int i = 0; i < 31; i++) {
		float x = i - 15;
		ufo_bottom[i][0] = x;
		ufo_bottom[i][1] = 2.0f / 45.0f * x * x - 5.0f;
	}

	GLsizeiptr buffer_size = sizeof(ufo_top) + sizeof(ufo_bottom) + sizeof(ufo_middle) + sizeof(ufo_behind_left) + sizeof(ufo_behind_right) + sizeof(ufo_window_middle) + sizeof(ufo_window_left) + sizeof(ufo_window_right);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_ufo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_ufo);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ufo_behind_left), ufo_behind_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ufo_behind_left), sizeof(ufo_behind_right), ufo_behind_right);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ufo_behind_left) + sizeof(ufo_behind_right), sizeof(ufo_top), ufo_top);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ufo_behind_left) + sizeof(ufo_behind_right) + sizeof(ufo_top), sizeof(ufo_bottom), ufo_bottom);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ufo_behind_left) + sizeof(ufo_behind_right) + sizeof(ufo_top) + sizeof(ufo_bottom), sizeof(ufo_middle), ufo_middle);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ufo_behind_left) + sizeof(ufo_behind_right) + sizeof(ufo_top) + sizeof(ufo_bottom) + sizeof(ufo_middle), sizeof(ufo_window_left), ufo_window_left);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ufo_behind_left) + sizeof(ufo_behind_right) + sizeof(ufo_top) + sizeof(ufo_bottom) + sizeof(ufo_middle) + sizeof(ufo_window_left), sizeof(ufo_window_middle), ufo_window_middle);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(ufo_behind_left) + sizeof(ufo_behind_right) + sizeof(ufo_top) + sizeof(ufo_bottom) + sizeof(ufo_middle) + sizeof(ufo_window_left) + sizeof(ufo_window_middle), sizeof(ufo_window_right), ufo_window_right);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_ufo);
	glBindVertexArray(VAO_ufo);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_ufo);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_ufo() {
	glBindVertexArray(VAO_ufo);

	// ufo_behind_left
	glUniform3fv(loc_primitive_color, 1, ufo_color[UFO_MIDDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	// ufo_behind_right
	glUniform3fv(loc_primitive_color, 1, ufo_color[UFO_MIDDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 6);

	glUniform3fv(loc_primitive_color, 1, ufo_color[UFO_TOP]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 180);

	glUniform3fv(loc_primitive_color, 1, ufo_color[UFO_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 186, 31);

	glUniform3fv(loc_primitive_color, 1, ufo_color[UFO_MIDDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 217, 4);

	glUniform3fv(loc_primitive_color, 1, ufo_color[UFO_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 221, 7); // window_left

	glDrawArrays(GL_TRIANGLE_FAN, 228, 4); // window_middle

	glDrawArrays(GL_TRIANGLE_FAN, 232, 7); // window_right

	glBindVertexArray(0);
}

#define AIRPLANE_BIG_WING 0
#define AIRPLANE_SMALL_WING 1
#define AIRPLANE_BODY 2
#define AIRPLANE_BACK 3
#define AIRPLANE_SIDEWINDER1 4
#define AIRPLANE_SIDEWINDER2 5
#define AIRPLANE_CENTER 6
GLfloat big_wing[6][2] = { { 0.0, 0.0 },{ -20.0, 15.0 },{ -20.0, 20.0 },{ 0.0, 23.0 },{ 20.0, 20.0 },{ 20.0, 15.0 } };
GLfloat small_wing[6][2] = { { 0.0, -18.0 },{ -11.0, -12.0 },{ -12.0, -7.0 },{ 0.0, -10.0 },{ 12.0, -7.0 },{ 11.0, -12.0 } };
GLfloat body[5][2] = { { 0.0, -25.0 },{ -6.0, 0.0 },{ -6.0, 22.0 },{ 6.0, 22.0 },{ 6.0, 0.0 } };
GLfloat back[5][2] = { { 0.0, 25.0 },{ -7.0, 24.0 },{ -7.0, 21.0 },{ 7.0, 21.0 },{ 7.0, 24.0 } };
GLfloat sidewinder1[5][2] = { { -20.0, 10.0 },{ -18.0, 3.0 },{ -16.0, 10.0 },{ -18.0, 20.0 },{ -20.0, 20.0 } };
GLfloat sidewinder2[5][2] = { { 20.0, 10.0 },{ 18.0, 3.0 },{ 16.0, 10.0 },{ 18.0, 20.0 },{ 20.0, 20.0 } };
GLfloat center[1][2] = { { 0.0, 0.0 } };
GLfloat airplane_color[7][3] = {
	{ 150 / 255.0f, 129 / 255.0f, 183 / 255.0f },  // big_wing
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // small_wing
	{ 111 / 255.0f,  85 / 255.0f, 157 / 255.0f },  // body
	{ 150 / 255.0f, 129 / 255.0f, 183 / 255.0f },  // back
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder1
	{ 245 / 255.0f, 211 / 255.0f,   0 / 255.0f },  // sidewinder2
	{ 255 / 255.0f,   0 / 255.0f,   0 / 255.0f }   // center
};

GLuint VBO_airplane, VAO_airplane;

int airplane_clock = 0;
float airplane_s_factor = 1.0f;

void prepare_airplane() {

	GLsizeiptr buffer_size = sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back) + sizeof(sidewinder1) + sizeof(sidewinder2) + sizeof(center);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(big_wing), big_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing), sizeof(small_wing), small_wing);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing), sizeof(body), body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body), sizeof(back), back);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back),
		sizeof(sidewinder1), sidewinder1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1), sizeof(sidewinder2), sidewinder2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(big_wing) + sizeof(small_wing) + sizeof(body) + sizeof(back)
		+ sizeof(sidewinder1) + sizeof(sidewinder2), sizeof(center), center);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_airplane);
	glBindVertexArray(VAO_airplane);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_airplane);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_airplane() { // Draw airplane in its MC.
	glBindVertexArray(VAO_airplane);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BIG_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SMALL_WING]);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_BACK]);
	glDrawArrays(GL_TRIANGLE_FAN, 17, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER1]);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_SIDEWINDER2]);
	glDrawArrays(GL_TRIANGLE_FAN, 27, 5);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_CENTER]);
	glPointSize(5.0);
	glDrawArrays(GL_POINTS, 32, 1);
	glPointSize(1.0);

	glUniform3fv(loc_primitive_color, 1, airplane_color[AIRPLANE_CENTER]);
	glPointSize(5.0);
	glDrawArrays(GL_POINTS, 0, 1);
	glPointSize(1.0);
	glBindVertexArray(0);
}

//house
#define HOUSE_ROOF 0
#define HOUSE_BODY 1
#define HOUSE_CHIMNEY 2
#define HOUSE_DOOR 3
#define HOUSE_WINDOW 4

GLfloat roof[3][2] = { { -12.0, 0.0 },{ 0.0, 12.0 },{ 12.0, 0.0 } };
GLfloat house_body[4][2] = { { -12.0, -14.0 },{ -12.0, 0.0 },{ 12.0, 0.0 },{ 12.0, -14.0 } };
GLfloat chimney[4][2] = { { 6.0, 6.0 },{ 6.0, 14.0 },{ 10.0, 14.0 },{ 10.0, 2.0 } };
GLfloat door[4][2] = { { -8.0, -14.0 },{ -8.0, -8.0 },{ -4.0, -8.0 },{ -4.0, -14.0 } };
GLfloat window[4][2] = { { 4.0, -6.0 },{ 4.0, -2.0 },{ 8.0, -2.0 },{ 8.0, -6.0 } };

GLfloat house_color[5][3] = {
	{ 200 / 255.0f, 39 / 255.0f, 42 / 255.0f },
	{ 235 / 255.0f, 225 / 255.0f, 196 / 255.0f },
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 233 / 255.0f, 113 / 255.0f, 23 / 255.0f },
	{ 44 / 255.0f, 180 / 255.0f, 49 / 255.0f }
};

GLuint VBO_house, VAO_house;
void prepare_house() {
	GLsizeiptr buffer_size = sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door)
		+ sizeof(window);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(roof), roof);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof), sizeof(house_body), house_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body), sizeof(chimney), chimney);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney), sizeof(door), door);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(roof) + sizeof(house_body) + sizeof(chimney) + sizeof(door),
		sizeof(window), window);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_house);
	glBindVertexArray(VAO_house);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_house);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_house() {
	glBindVertexArray(VAO_house);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_ROOF]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 3);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_CHIMNEY]);
	glDrawArrays(GL_TRIANGLE_FAN, 7, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_DOOR]);
	glDrawArrays(GL_TRIANGLE_FAN, 11, 4);

	glUniform3fv(loc_primitive_color, 1, house_color[HOUSE_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glBindVertexArray(0);
}

//draw car2
#define CAR2_BODY 0
#define CAR2_FRONT_WINDOW 1
#define CAR2_BACK_WINDOW 2
#define CAR2_FRONT_WHEEL 3
#define CAR2_BACK_WHEEL 4
#define CAR2_LIGHT1 5
#define CAR2_LIGHT2 6

GLfloat car2_body[8][2] = { { -18.0, -7.0 },{ -18.0, 0.0 },{ -13.0, 0.0 },{ -10.0, 8.0 },{ 10.0, 8.0 },{ 13.0, 0.0 },{ 18.0, 0.0 },{ 18.0, -7.0 } };
GLfloat car2_front_window[4][2] = { { -10.0, 0.0 },{ -8.0, 6.0 },{ -2.0, 6.0 },{ -2.0, 0.0 } };
GLfloat car2_back_window[4][2] = { { 0.0, 0.0 },{ 0.0, 6.0 },{ 8.0, 6.0 },{ 10.0, 0.0 } };
GLfloat car2_front_wheel[8][2] = { { -11.0, -11.0 },{ -13.0, -8.0 },{ -13.0, -7.0 },{ -11.0, -4.0 },{ -7.0, -4.0 },{ -5.0, -7.0 },{ -5.0, -8.0 },{ -7.0, -11.0 } };
GLfloat car2_back_wheel[8][2] = { { 7.0, -11.0 },{ 5.0, -8.0 },{ 5.0, -7.0 },{ 7.0, -4.0 },{ 11.0, -4.0 },{ 13.0, -7.0 },{ 13.0, -8.0 },{ 11.0, -11.0 } };
GLfloat car2_light1[3][2] = { { -18.0, -1.0 },{ -17.0, -2.0 },{ -18.0, -3.0 } };
GLfloat car2_light2[3][2] = { { -18.0, -4.0 },{ -17.0, -5.0 },{ -18.0, -6.0 } };

GLfloat car2_color[7][3] = {
	{ 100 / 255.0f, 141 / 255.0f, 159 / 255.0f },
	{ 235 / 255.0f, 219 / 255.0f, 208 / 255.0f },
	{ 235 / 255.0f, 219 / 255.0f, 208 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f },
	{ 249 / 255.0f, 244 / 255.0f, 0 / 255.0f }
};

GLuint VBO_car2, VAO_car2;
void prepare_car2() {
	GLsizeiptr buffer_size = sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel) + sizeof(car2_light1) + sizeof(car2_light2);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(car2_body), car2_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body), sizeof(car2_front_window), car2_front_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window), sizeof(car2_back_window), car2_back_window);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window), sizeof(car2_front_wheel), car2_front_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel),
		sizeof(car2_back_wheel), car2_back_wheel);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel), sizeof(car2_light1), car2_light1);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(car2_body) + sizeof(car2_front_window) + sizeof(car2_back_window) + sizeof(car2_front_wheel)
		+ sizeof(car2_back_wheel) + sizeof(car2_light1), sizeof(car2_light2), car2_light2);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_car2);
	glBindVertexArray(VAO_car2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_car2);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_car2() {
	glBindVertexArray(VAO_car2);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WINDOW]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_FRONT_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_BACK_WHEEL]);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 8);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT1]);
	glDrawArrays(GL_TRIANGLE_FAN, 32, 3);

	glUniform3fv(loc_primitive_color, 1, car2_color[CAR2_LIGHT2]);
	glDrawArrays(GL_TRIANGLE_FAN, 35, 3);

	glBindVertexArray(0);
}

// cake
#define CAKE_FIRE 0
#define CAKE_CANDLE 1
#define CAKE_BODY 2
#define CAKE_BOTTOM 3
#define CAKE_DECORATE 4

GLfloat cake_fire[4][2] = { { -0.5, 14.0 },{ -0.5, 13.0 },{ 0.5, 13.0 },{ 0.5, 14.0 } };
GLfloat cake_candle[4][2] = { { -1.0, 8.0 } ,{ -1.0, 13.0 },{ 1.0, 13.0 },{ 1.0, 8.0 } };
GLfloat cake_body[4][2] = { { 8.0, 5.0 },{ -8.0, 5.0 } ,{ -8.0, 8.0 },{ 8.0, 8.0 } };
GLfloat cake_bottom[4][2] = { { -10.0, 1.0 },{ -10.0, 5.0 },{ 10.0, 5.0 },{ 10.0, 1.0 } };
GLfloat cake_decorate[4][2] = { { -10.0, 0.0 },{ -10.0, 1.0 },{ 10.0, 1.0 },{ 10.0, 0.0 } };

GLfloat cake_color[5][3] = {
	{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 204 / 255.0f, 0 / 255.0f },
{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
{ 255 / 255.0f, 102 / 255.0f, 255 / 255.0f },
{ 102 / 255.0f, 51 / 255.0f, 0 / 255.0f }
};

GLuint VBO_cake, VAO_cake;

void prepare_cake() {
	int size = sizeof(cake_fire);
	GLsizeiptr buffer_size = sizeof(cake_fire) * 5;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, size, cake_fire);
	glBufferSubData(GL_ARRAY_BUFFER, size, size, cake_candle);
	glBufferSubData(GL_ARRAY_BUFFER, size * 2, size, cake_body);
	glBufferSubData(GL_ARRAY_BUFFER, size * 3, size, cake_bottom);
	glBufferSubData(GL_ARRAY_BUFFER, size * 4, size, cake_decorate);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cake);
	glBindVertexArray(VAO_cake);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cake);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_cake() {
	glBindVertexArray(VAO_cake);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_FIRE]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_CANDLE]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_BOTTOM]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	glUniform3fv(loc_primitive_color, 1, cake_color[CAKE_DECORATE]);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);

	glBindVertexArray(0);
}

// sword

#define SWORD_BODY 0
#define SWORD_BODY2 1
#define SWORD_HEAD 2
#define SWORD_HEAD2 3
#define SWORD_IN 4
#define SWORD_DOWN 5
#define SWORD_BODY_IN 6

GLfloat sword_body[4][2] = { { -6.0, 0.0 },{ -6.0, -4.0 },{ 6.0, -4.0 },{ 6.0, 0.0 } };
GLfloat sword_body2[4][2] = { { -2.0, -4.0 },{ -2.0, -6.0 } ,{ 2.0, -6.0 },{ 2.0, -4.0 } };
GLfloat sword_head[4][2] = { { -2.0, 0.0 },{ -2.0, 16.0 } ,{ 2.0, 16.0 },{ 2.0, 0.0 } };
GLfloat sword_head2[3][2] = { { -2.0, 16.0 },{ 0.0, 19.46 } ,{ 2.0, 16.0 } };
GLfloat sword_in[4][2] = { { -0.3, 0.7 },{ -0.3, 15.3 } ,{ 0.3, 15.3 },{ 0.3, 0.7 } };
GLfloat sword_down[4][2] = { { -2.0, -6.0 } ,{ 2.0, -6.0 },{ 4.0, -8.0 },{ -4.0, -8.0 } };
GLfloat sword_body_in[4][2] = { { 0.0, -1.0 } ,{ 1.0, -2.732 },{ 0.0, -4.464 },{ -1.0, -2.732 } };

GLfloat sword_color[7][3] = {
	{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 155 / 255.0f, 155 / 255.0f, 155 / 255.0f },
{ 0 / 255.0f, 0 / 255.0f, 0 / 255.0f },
{ 139 / 255.0f, 69 / 255.0f, 19 / 255.0f },
{ 255 / 255.0f, 0 / 255.0f, 0 / 255.0f }
};

GLuint VBO_sword, VAO_sword;

void prepare_sword() {
	GLsizeiptr buffer_size = sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down) + sizeof(sword_body_in);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sword_body), sword_body);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body), sizeof(sword_body2), sword_body2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2), sizeof(sword_head), sword_head);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head), sizeof(sword_head2), sword_head2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2), sizeof(sword_in), sword_in);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in), sizeof(sword_down), sword_down);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sword_body) + sizeof(sword_body2) + sizeof(sword_head) + sizeof(sword_head2) + sizeof(sword_in) + sizeof(sword_down), sizeof(sword_body_in), sword_body_in);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_sword);
	glBindVertexArray(VAO_sword);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_sword);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void draw_sword() {
	glBindVertexArray(VAO_sword);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY2]);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD]);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_HEAD2]);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 3);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 15, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_DOWN]);
	glDrawArrays(GL_TRIANGLE_FAN, 19, 4);

	glUniform3fv(loc_primitive_color, 1, sword_color[SWORD_BODY_IN]);
	glDrawArrays(GL_TRIANGLE_FAN, 23, 4);

	glBindVertexArray(0);
}

#define TIME_UNIT 10
#define CAR_ROTATION_RADIUS 100.0f
#define HOUSE_ROTATION_RADIUS 100.0f
unsigned int timestamp = 0;
glm::vec3 ZAXIS = glm::vec3(0.0f, 0.0f, 1.0f);
int phase = 0;
float ufox = 0, ufoy = 0, ax = 300, ay = -300, arotate = 0.0f;
glm::vec3 adir = glm::vec3(0.0f, 1.0f, 0.0f), dir_to_ufo = glm::vec3(-300.0f, 300.0f, 0.0f);

void timer(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	glutPostRedisplay();
	glutTimerFunc(TIME_UNIT, timer, 0);
}

void cake_arc_coord(int clock, float* x, float* y) {
	*x = 200.0f - 2.0f * clock;
	*y = -1.0f / 400.0f * (*x) * (*x) + 100.0f;
}

void display(void) {
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);

	int car_clock = timestamp % 360;

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-200.0f, -200.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -car_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(CAR_ROTATION_RADIUS, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	float size;
	car_clock %= 90;
	if (car_clock < 45) {
		size = 2.0f + (float)car_clock / 45.0f;
	}
	else {
		size = 3.0f - ((float)car_clock - 45.0f) / 45.0f;
	}
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(size, size, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car2();

	int house_clock = timestamp * 2 % 2700;
	int house_cycle_cnt = house_clock / 180;
	int house_cycle = house_clock % 360;
	float line_angle = atanf(1.0f / 3.0f);

	// rotation center moves down on line y = -1/3 * x + 300
	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 300.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, -line_angle, glm::vec3(0.0f, 0.0f, 1.0f));

	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-250.0f + HOUSE_ROTATION_RADIUS / 2.0f * house_cycle_cnt, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, house_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	if (house_cycle < 180) { // bottom big circle
		float house_size = 1.0f + 4.0f / 180.0f * house_cycle;

		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-HOUSE_ROTATION_RADIUS, 0.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(house_size, house_size, 0.0f));

	}
	else { // top small circle
		float house_size = 1.0f + 4.0f / 180.0f * (360 - house_cycle);

		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-HOUSE_ROTATION_RADIUS / 2, 0.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(house_size, house_size, 0.0f));

	}

	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();

	int cake_clock = timestamp % 1000;
	int sword_clock = cake_clock;

	if (cake_clock < 350) {
		ModelMatrix = glm::mat4(1.0f);

		if (cake_clock < 100) {
			float len = (float)cake_clock * 2.0f / cosf(10.0f * TO_RADIAN);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-400.0f, 0.0f, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, 10.0f * TO_RADIAN, ZAXIS);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(len, 0.0f, 0.0f));
		}
		else if (cake_clock < 200) {
			float len = (cake_clock - 100.0f) * 2.0f / sinf(40.0f * TO_RADIAN);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-200.0f, 200.0f * tanf(10.0f * TO_RADIAN), 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, -50.0f * TO_RADIAN, ZAXIS);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(len, 0.0f, 0.0f));
		}
		else {
			float h = 200.0f / tanf(40.0f * TO_RADIAN) - 200.0f * tanf(10.0f * TO_RADIAN);
			float len = 2.0f * (cake_clock - 200.0f) * 2.0f / cosf(40.0f * TO_RADIAN);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -h, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, 40.0f * TO_RADIAN, ZAXIS);
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(len, 0.0f, 0.0f));
		}

		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 0.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_cake();

		if (100 < sword_clock) {
			ModelMatrix = glm::mat4(1.0f);

			if (sword_clock < 200) {
				float h = 200.0f / tanf(40.0f * TO_RADIAN) - 200.0f * tanf(10.0f * TO_RADIAN) + 400.0f;
				float len = h / cosf(40.0f * TO_RADIAN) * (sword_clock - 100.0f) / 100.0f;
				float xoffset = h * tanf(40.0f * TO_RADIAN);

				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-xoffset, 400.0f, 0.0f));
				ModelMatrix = glm::rotate(ModelMatrix, -50.0f * TO_RADIAN, ZAXIS);
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(len, 0.0f, 0.0f));
				ModelMatrix = glm::rotate(ModelMatrix, -100.0f * TO_RADIAN, ZAXIS);
			}
			else if (sword_clock < 280) { // 250
				float h = 200.0f / tanf(40.0f * TO_RADIAN) - 200.0f * tanf(10.0f * TO_RADIAN);

				float angle = (-330.0f + sword_clock) * TO_RADIAN;
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -h, 0.0f));
				ModelMatrix = glm::rotate(ModelMatrix, angle, ZAXIS);
			}
			else {
				float h = 200.0f / tanf(40.0f * TO_RADIAN) - 200.0f * tanf(10.0f * TO_RADIAN);
				float len = 40.0f / sinf(40.0f * TO_RADIAN) * (sword_clock - 280.0f) / 5.0f;
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -h, 0.0f));
				ModelMatrix = glm::rotate(ModelMatrix, 40.0f * TO_RADIAN, ZAXIS);
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(len, 0.0f, 0.0f));
				ModelMatrix = glm::rotate(ModelMatrix, -90.0f * TO_RADIAN, ZAXIS);
			}

			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 0.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_sword();
		}
	}
	else if (400 <= cake_clock) {
		cake_clock -= 400;
		sword_clock -= 400;
		float x, y;

		ModelMatrix = glm::mat4(1.0f);

		if (cake_clock < 200) {
			cake_arc_coord(cake_clock, &x, &y);

			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(400.0f, -50.0f, 0.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x, y, 0.0f));
		}
		else if (cake_clock < 400) {
			cake_arc_coord(cake_clock - 200, &x, &y);

			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -50.0f, 0.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x, y, 0.0f));
		}
		else if (cake_clock < 600) {
			cake_arc_coord(cake_clock - 400, &x, &y);

			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-400.0f, -50.0f, 0.0f));
			ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x, y, 0.0f));
		}

		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10.0f, 10.0f, 0.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_cake();

		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(400.0f - 2.0f * sword_clock, 0.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, 10.0f * sword_clock * TO_RADIAN, ZAXIS);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 0.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_sword();

	}

	int ufo_clock = timestamp % 220;
	if (ufo_clock == 0) {
		ufox = rand() % 700 - 350;
		ufoy = rand() % 700 - 350;
	}

	ModelMatrix = glm::mat4(1.0f);
	if (ufo_clock <= 100) {
		float size = ufo_clock * 0.02f;
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(ufox, ufoy, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(size, size, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, 3.6f * ufo_clock * TO_RADIAN, ZAXIS);
	}
	else if (ufo_clock <= 120) {
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(ufox, ufoy, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
	}
	else {
		ufo_clock -= 120;
		float size = 2.0f - ufo_clock * 2.0f / 100.0f;
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(ufox, ufoy, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(size, size, 1.0f));
		ModelMatrix = glm::rotate(ModelMatrix, -10.8f * ufo_clock * TO_RADIAN, ZAXIS);
	}

	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_ufo();

	dir_to_ufo = glm::vec3(ufox - ax, ufoy - ay, 0.0f);
	glm::vec3 res = glm::cross(adir, dir_to_ufo);
	float angle = asinf(glm::length(res) / glm::length(adir) / glm::length(dir_to_ufo));

	if (angle > 0) {
		if (res.z < 0) { // -rotate
			if (angle <= 1.0f * TO_RADIAN) {
				arotate -= angle;
			}
			else {
				arotate -= 1.0f * TO_RADIAN;
			}
			adir.x = cosf(arotate + 90.0f * TO_RADIAN); adir.y = sinf(arotate + 90.0f * TO_RADIAN);

		}
		else { // +rotate
			if (angle <= 1.0f * TO_RADIAN) {
				arotate += angle;
			}
			else {
				arotate += 1.0f * TO_RADIAN;
			}
			adir.x = cosf(arotate + 90.0f * TO_RADIAN); adir.y = sinf(arotate + 90.0f * TO_RADIAN);
		}
	}
	if (glm::length(glm::vec3(ufox - ax, ufoy - ay, 0.0f)) > 10.0f) {
		if (angle < 5.0f * TO_RADIAN) {
			ax += 2 * adir.x; ay += 2 * adir.y;
		}
		ax += adir.x; ay += adir.y;
	}

	ModelMatrix = glm::mat4(1.0f);

	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(ax, ay, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, arotate, ZAXIS);
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.5f, -1.5f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();

	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void reshape(int width, int height) {
	win_width = width, win_height = height;

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0, -win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_ufo);
	glDeleteBuffers(1, &VBO_ufo);

	glDeleteVertexArrays(1, &VAO_car2);
	glDeleteBuffers(1, &VBO_car2);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);

	glDeleteVertexArrays(1, &VAO_cake);
	glDeleteBuffers(1, &VBO_cake);

	glDeleteVertexArrays(1, &VAO_sword);
	glDeleteBuffers(1, &VBO_sword);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	// Delete others here too!!!
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutTimerFunc(TIME_UNIT, timer, 0);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//glClearColor(250 / 255.0f, 128 / 255.0f, 114 / 255.0f, 1.0f);
	glClearColor(1, 1, 1, 1);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_airplane();
	prepare_house();
	prepare_car2();
	prepare_cake();
	prepare_sword();
	prepare_ufo();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

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
	fprintf(stdout, "  PROGRAM NAME: %s\n", program_name);
	fprintf(stdout, "    by 20201551 권지은\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 2
void main(int argc, char* argv[]) {
	char program_name[64] = "HW2 2차원 모델링 변환";
	char messages[N_MESSAGE_LINES][256] = {
		"    - objects used: car2, house, cake, sword, airplane ",
		"    - oject created: ufo"
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


