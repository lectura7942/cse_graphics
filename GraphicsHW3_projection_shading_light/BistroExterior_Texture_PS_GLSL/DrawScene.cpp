//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS; // handle to shader program

GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 3 
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_texture;
GLint loc_flag_texture_mapping;
GLint loc_u_flag_fog;
GLint loc_u_flag_blending, loc_u_fragment_alpha;
GLint loc_u_flag_blind;
GLint loc_u_flag_change, loc_u_change_factor;

int flag_blending = 0;
float cow_alpha = 0.5f;
int flag_fog = 0;
float change_factor = 0.0f;

// Gouraud Shading
GLuint h_ShaderProgram_G;
GLint loc_global_ambient_color_G;
loc_light_Parameters loc_light_G[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material_G;
GLint loc_ModelViewProjectionMatrix_G, loc_ModelViewMatrix_G, loc_ModelViewMatrixInvTrans_G;

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp > // affineInverse
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

int light_on[3] = { 0,0,0 };
glm::vec4 world_light_WC = { 0.0f, 0.0f, 3000.0f, 1.0f };

void find_world_light_EC() {
	glm::vec4 world_light_EC = ViewMatrix * world_light_WC;
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform4f(loc_light[0].position, world_light_EC[0], world_light_EC[1], world_light_EC[2], 1.0f);
	glUseProgram(h_ShaderProgram_G);
	glUniform4f(loc_light_G[0].position, world_light_EC[0], world_light_EC[1], world_light_EC[2], 1.0f);
	glUseProgram(0);
}

/*********************************  START: my data 20201551*********************************/
glm::mat4 ModelMatrix_tiger, ModelViewMatrix_tiger, ModelMatrix_tiger_eye, ModelMatrix_tiger_behind;
int timestamp_tiger;
glm::mat4 ModelMatrix_ironman;
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

// texture stuffs
#define N_TEXTURES_USED 2
#define TEXTURE_ID_FLOOR 0
#define TEXTURE_ID_TIGER 1
GLuint texture_names[N_TEXTURES_USED];
//int flag_texture_mapping;

void find_tiger_light_EC(glm::mat4 ModelViewMatrix_tiger) {
	glUseProgram(h_ShaderProgram_TXPS);
	//glm::vec3 tiger_light_MC = glm::vec3(0.0f, -88.0f, 62.0f);
	glm::vec4 tiger_light_pos = ModelViewMatrix_tiger * glm::vec4(0.0f, -88.0f, 62.0f, 1.0f);
	glm::vec4 tiger_light_dir = ModelViewMatrix_tiger * glm::vec4(0.0f, -1.0f, 0.0f, 0.0f); // chk
	glUniform4f(loc_light[2].position, tiger_light_pos[0], tiger_light_pos[1], tiger_light_pos[2], 1.0f);
	glUniform3f(loc_light[2].spot_direction, tiger_light_dir[0], tiger_light_dir[1], tiger_light_dir[2]);
	glUseProgram(0);
}

void My_glTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);
}

typedef enum {
	UAXIS,
	VAXIS,
	NAXIS
} AXIS;

#define TUNIT 100.0f // 카메라 이동 단위
#define RUNIT 0.1f // 카메라 회전 단위
//#define ZUNIT 0.1f // 카메라 줌 단위

int prevx, prevy; // 이전 마우스 WdC 위치 저장

int cur_frame_tiger = 0, cur_frame_ben = 0, cur_frame_wolf, cur_frame_spider = 0;
int animation_tiger = 1; // 호랑이 이동/멈춤.
int timestamp = 0;
/*********************************  END: my data 20201551*********************************/

/*********************************  START: object*********************************/

// tiger object
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];

Material_Parameters material_tiger;

// ben object
#define N_BEN_FRAMES 30
GLuint ben_VBO, ben_VAO;
int ben_n_triangles[N_BEN_FRAMES];
int ben_vertex_offset[N_BEN_FRAMES];
GLfloat* ben_vertices[N_BEN_FRAMES];

Material_Parameters material_ben;

// spider object
#define N_SPIDER_FRAMES 16
GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLfloat* spider_vertices[N_SPIDER_FRAMES];

Material_Parameters material_spider;

// dragon object
GLuint dragon_VBO, dragon_VAO;
int dragon_n_triangles;
GLfloat* dragon_vertices;

Material_Parameters material_dragon;

// cow object
GLuint cow_VBO, cow_VAO;
int cow_n_triangles;
GLfloat* cow_vertices;

Material_Parameters material_cow;

// godzilla object
GLuint godzilla_VBO, godzilla_VAO;
int godzilla_n_triangles;
GLfloat* godzilla_vertices;

Material_Parameters material_godzilla;

// ironman object
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;

Material_Parameters material_ironman;

// tank object
GLuint tank_VBO, tank_VAO;
int tank_n_triangles;
GLfloat* tank_vertices;

Material_Parameters material_tank;

int read_geometry(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);

	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	fclose(fp);

	return n_triangles;
}

void prepare_tiger(void) { // vertices enumerated clockwise
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/tiger/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_tiger.ambient_color[0] = 0.24725f;
	material_tiger.ambient_color[1] = 0.1995f;
	material_tiger.ambient_color[2] = 0.0745f;
	material_tiger.ambient_color[3] = 1.0f;

	material_tiger.diffuse_color[0] = 0.75164f;
	material_tiger.diffuse_color[1] = 0.60648f;
	material_tiger.diffuse_color[2] = 0.22648f;
	material_tiger.diffuse_color[3] = 1.0f;

	material_tiger.specular_color[0] = 0.728281f;
	material_tiger.specular_color[1] = 0.655802f;
	material_tiger.specular_color[2] = 0.466065f;
	material_tiger.specular_color[3] = 1.0f;

	material_tiger.specular_exponent = 51.2f;

	material_tiger.emissive_color[0] = 0.1f;
	material_tiger.emissive_color[1] = 0.1f;
	material_tiger.emissive_color[2] = 0.0f;
	material_tiger.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	strcpy(filename, "Data/dynamic_objects/tiger/tiger_tex2.jpg");
	My_glTexImage2D_from_file(filename);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//	// Set camera behind tiger
	ModelMatrix_tiger_behind = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 220.0f, 100.0f));
	ModelMatrix_tiger_behind= glm::rotate(ModelMatrix_tiger_behind, TO_RADIAN * (180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_tiger_behind= glm::rotate(ModelMatrix_tiger_behind, TO_RADIAN * (90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
}

void prepare_ben(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ben_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_BEN_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/ben/ben_vn%d%d.geom", i / 10, i % 10);
		ben_n_triangles[i] = read_geometry(&ben_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		ben_n_total_triangles += ben_n_triangles[i];

		if (i == 0)
			ben_vertex_offset[i] = 0;
		else
			ben_vertex_offset[i] = ben_vertex_offset[i - 1] + 3 * ben_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &ben_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glBufferData(GL_ARRAY_BUFFER, ben_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_BEN_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, ben_vertex_offset[i] * n_bytes_per_vertex,
			ben_n_triangles[i] * n_bytes_per_triangle, ben_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_BEN_FRAMES; i++)
		free(ben_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &ben_VAO);
	glBindVertexArray(ben_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ben_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	sprintf(filename, "Data/dynamic_objects/tiger/tiger_tex2.jpg");
	My_glTexImage2D_from_file(filename);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
}
void prepare_spider(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, spider_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_SPIDER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/spider/spider_vnt_%d%d.geom", i / 10, i % 10);
		spider_n_triangles[i] = read_geometry(&spider_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		spider_n_total_triangles += spider_n_triangles[i];

		if (i == 0)
			spider_vertex_offset[i] = 0;
		else
			spider_vertex_offset[i] = spider_vertex_offset[i - 1] + 3 * spider_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &spider_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glBufferData(GL_ARRAY_BUFFER, spider_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, spider_vertex_offset[i] * n_bytes_per_vertex,
			spider_n_triangles[i] * n_bytes_per_triangle, spider_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_SPIDER_FRAMES; i++)
		free(spider_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &spider_VAO);
	glBindVertexArray(spider_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	sprintf(filename, "Data/dynamic_objects/tiger/tiger_tex2.jpg");
	My_glTexImage2D_from_file(filename);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void prepare_dragon(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, dragon_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/dragon_vnt.geom");
	dragon_n_triangles = read_geometry(&dragon_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	dragon_n_total_triangles += dragon_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &dragon_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glBufferData(GL_ARRAY_BUFFER, dragon_n_total_triangles * 3 * n_bytes_per_vertex, dragon_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(dragon_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &dragon_VAO);
	glBindVertexArray(dragon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	sprintf(filename, "Data/dynamic_objects/tiger/tiger_tex2.jpg");
	My_glTexImage2D_from_file(filename);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
}

void prepare_cow(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, cow_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/cow_vn.geom");
	cow_n_triangles = read_geometry(&cow_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	cow_n_total_triangles += cow_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &cow_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glBufferData(GL_ARRAY_BUFFER, cow_n_total_triangles * 3 * n_bytes_per_vertex, cow_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(cow_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &cow_VAO);
	glBindVertexArray(cow_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void prepare_godzilla(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, godzilla_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/godzilla_vnt.geom");
	godzilla_n_triangles = read_geometry(&godzilla_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	godzilla_n_total_triangles += godzilla_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &godzilla_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glBufferData(GL_ARRAY_BUFFER, godzilla_n_total_triangles * 3 * n_bytes_per_vertex, godzilla_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(godzilla_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &godzilla_VAO);
	glBindVertexArray(godzilla_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, godzilla_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	sprintf(filename, "Data/dynamic_objects/tiger/tiger_tex2.jpg");
	My_glTexImage2D_from_file(filename);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
}
void prepare_ironman(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ironman_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	sprintf(filename, "Data/dynamic_objects/tiger/tiger_tex2.jpg");
	My_glTexImage2D_from_file(filename);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
}
void prepare_tank(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tank_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/tank_vnt.geom");
	tank_n_triangles = read_geometry(&tank_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	tank_n_total_triangles += tank_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &tank_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glBufferData(GL_ARRAY_BUFFER, tank_n_total_triangles * n_bytes_per_triangle, tank_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(tank_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &tank_VAO);
	glBindVertexArray(tank_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	/*glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);*/

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	sprintf(filename, "Data/dynamic_objects/tiger/tiger_tex2.jpg");
	My_glTexImage2D_from_file(filename);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
}

struct material {
	float ambient_color[4], diffuse_color[4], specular_color[4], emissive_color[4];
	float specular_exponent;
};

material material_ruby = {
	{0.1745f, 0.01175f, 0.01175f, 1.0f},
	{0.61424f, 0.04136f, 0.04136f, 1.0f},
	{0.727811f, 0.626959f, 0.626959, 1.0f},
	{0.1745f, 0.01175f, 0.01175f, 1.0f},
	76.8f
};

material material_brass = {
	{0.329412f, 0.223529f, 0.027451f,1.0f},
	{0.780392f, 0.568627f, 0.113725f, 1.0f},
	{0.992157f, 0.941176f, 0.807843f, 1.0f},
	{0.329412f, 0.223529f, 0.027451f,1.0f},
	27.9f
};

void set_material_ruby_TXPS(void) {
		// assume ShaderProgram_TXPS is used
		glUniform4fv(loc_material.ambient_color, 1, material_ruby.ambient_color);
		glUniform4fv(loc_material.diffuse_color, 1, material_ruby.diffuse_color);
		glUniform4fv(loc_material.specular_color, 1, material_ruby.specular_color);
		glUniform1f(loc_material.specular_exponent, material_ruby.specular_exponent);
		glUniform4fv(loc_material.emissive_color, 1, material_ruby.emissive_color);
}

void set_material_brass(int shader) {
	if (h_ShaderProgram_G) {
		// assume h_ShaderProgram_G is used
		glUniform4fv(loc_material_G.ambient_color, 1, material_brass.ambient_color);
		glUniform4fv(loc_material_G.diffuse_color, 1, material_brass.diffuse_color);
		glUniform4fv(loc_material_G.specular_color, 1, material_brass.specular_color);
		glUniform1f(loc_material_G.specular_exponent, material_brass.specular_exponent);
		glUniform4fv(loc_material_G.emissive_color, 1, material_brass.emissive_color);
	}
	if (h_ShaderProgram_TXPS) {
		glUniform4fv(loc_material.ambient_color, 1, material_brass.ambient_color);
		glUniform4fv(loc_material.diffuse_color, 1, material_brass.diffuse_color);
		glUniform4fv(loc_material.specular_color, 1, material_brass.specular_color);
		glUniform1f(loc_material.specular_exponent, material_brass.specular_exponent);
		glUniform4fv(loc_material.emissive_color, 1, material_brass.emissive_color);
	}
}

void set_material_tiger(void) {
	glUniform4fv(loc_material.ambient_color, 1, material_tiger.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_tiger.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_tiger.specular_color);
	glUniform1f(loc_material.specular_exponent, material_tiger.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_tiger.emissive_color);
}

void draw_tiger(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);
}

void draw_ben(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(ben_VAO);
	glDrawArrays(GL_TRIANGLES, ben_vertex_offset[cur_frame_ben], 3 * ben_n_triangles[cur_frame_ben]);
	glBindVertexArray(0);
}

void draw_spider(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(spider_VAO);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);
}
void draw_dragon(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(dragon_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);
}

void draw_cow(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(cow_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * cow_n_triangles);
	glBindVertexArray(0);
}

void draw_godzilla(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(godzilla_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * godzilla_n_triangles);
	glBindVertexArray(0);
}

void draw_ironman(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(ironman_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);
}

void draw_tank(void) {
	glFrontFace(GL_CW);

	glBindVertexArray(tank_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tank_n_triangles);
	glBindVertexArray(0);
}
/*********************************  END: object *********************************/

/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_0, //dragon+tank top view
	CAMERA_1, // ironman + spider view
	CAMERA_2, // ben + cow view
	CAMERA_3, //godzilla + tiger view
	CAMERA_M, // dragon + tank front view, 세상 이동 카메라
	CAMERA_T, // 호랑이 관점 카메라
	CAMERA_G, // 호랑이 관찰 카메라
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis; // 회전할 때만 move = 1
} Camera;

Camera camera_info[NUM_CAMERAS];
Camera current_camera;
int mode; // 카메라 모드

using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
}

// 호랑이 관점/관찰 카메라
void set_ViewMatrix_for_tiger() {
	glm::mat4 Matrix_camera_inverse;
	if (mode == CAMERA_T) {
		// Set tiger eye camera in mc
		ModelMatrix_tiger_eye = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -88.0f, 62.0f));
		float angle = 0.3f / 6.0f;
		if (cur_frame_tiger < 6) {
			ModelMatrix_tiger_eye = glm::rotate(ModelMatrix_tiger_eye, TO_RADIAN * (angle * cur_frame_tiger), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else {
			ModelMatrix_tiger_eye = glm::rotate(ModelMatrix_tiger_eye, TO_RADIAN * (12.0f * angle - angle * cur_frame_tiger), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		ModelMatrix_tiger_eye = glm::rotate(ModelMatrix_tiger_eye, TO_RADIAN * (180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_tiger_eye = glm::rotate(ModelMatrix_tiger_eye, TO_RADIAN * (90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		Matrix_camera_inverse = ModelMatrix_tiger * ModelMatrix_tiger_eye;
	}
	else if (mode == CAMERA_G) {
		Matrix_camera_inverse = ModelMatrix_tiger * ModelMatrix_tiger_behind;
	}
	
	ViewMatrix = glm::affineInverse(Matrix_camera_inverse);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

}

void set_current_camera(int camera_num) {
	mode = camera_num;
	if (camera_num < CAMERA_T) {
		Camera* pCamera = &camera_info[camera_num];
		memcpy(&current_camera, pCamera, sizeof(Camera));
		set_ViewMatrix_from_camera_frame();
		find_world_light_EC();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	}
}

void initialize_camera(void) {
	//CAMERA_1 : ironman + spider view
	Camera* pCamera = &camera_info[CAMERA_1];
	pCamera->pos[0] = -1012.0f; pCamera->pos[1] = 384.0f; pCamera->pos[2] = 265.0f;
	pCamera->uaxis[0] = -0.604633f; pCamera->uaxis[1] = -0.796480f; pCamera->uaxis[2] = -0.005047;
	pCamera->vaxis[0] = -0.071083f; pCamera->vaxis[1] = 0.047647f; pCamera->vaxis[2] = 0.996327f;
	pCamera->naxis[0] = 0.793316f; pCamera->naxis[1] = -0.602774f; pCamera->naxis[2] = 0.085425f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	// CAMERA_T: 호랑이 관점 카메라
	pCamera = &camera_info[CAMERA_T];
	pCamera->pos[0] = pCamera->pos[1] = pCamera->pos[2] = 0.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 1.0f;  pCamera->vaxis[2] = 0.0f;
	pCamera->naxis[0] = pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = -1.0f;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 10000.0f;

	//CAMERA_2 : ben + cow view
	pCamera = &camera_info[CAMERA_2];
	pCamera->pos[0] = -1602.0f; pCamera->pos[1] = 694.0f; pCamera->pos[2] = 498.0f;
	pCamera->uaxis[0] = -0.532990f; pCamera->uaxis[1] = -0.839770f; pCamera->uaxis[2] = 0.103445f;
	pCamera->vaxis[0] = 0.263861f; pCamera->vaxis[1] = -0.048805f; pCamera->vaxis[2] = 0.963315f;
	pCamera->naxis[0] = -0.803916f; pCamera->naxis[1] = 0.540735f; pCamera->naxis[2] = 0.247596f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_3 : godzilla + tiger view
	pCamera = &camera_info[CAMERA_3];
	pCamera->pos[0] = 1829.293579f; pCamera->pos[1] = -1426.412720f; pCamera->pos[2] = 48.108929f;
	pCamera->uaxis[0] = -0.355628f; pCamera->uaxis[1] = -0.934606f; pCamera->uaxis[2] = 0.003367f;
	pCamera->vaxis[0] = -0.281420f; pCamera->vaxis[1] = 0.110517f; pCamera->vaxis[2] = 0.953173f;
	pCamera->naxis[0] = -0.891238f; pCamera->naxis[1] = 0.338034f; pCamera->naxis[2] = -0.302325f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_0 : dragon+tank top view
	pCamera = &camera_info[CAMERA_0];
	pCamera->pos[0] = 800.0f; pCamera->pos[1] = 2700.0f; pCamera->pos[2] = 5900.0f; 
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 1.0f; pCamera->vaxis[2] = 0.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 1.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * (scene.camera.fovy), pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_M : dragon + tank front view, 세상 이동 카메라
	pCamera = &camera_info[CAMERA_M];
	pCamera->pos[0] = -1120.232788f; pCamera->pos[1] = -273.042023f; pCamera->pos[2] = 774.767578f;
	pCamera->uaxis[0] = 0.829477f; pCamera->uaxis[1] = -0.556484f; pCamera->uaxis[2] = -0.047773f;
	pCamera->vaxis[0] = 0.082245f; pCamera->vaxis[1] = 0.037096f; pCamera->vaxis[2] = 0.995908f;
	pCamera->naxis[0] = -0.552440f; pCamera->naxis[1] = -0.830015f; pCamera->naxis[2] = 0.076539f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	set_current_camera(CAMERA_0);
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_u_flag_blind, 1);
	glUseProgram(0);
}

void renew_cam_position(int axis, float dist) {
	// Move camera {dist} distance in {axis} direction
	switch (axis)
	{
	case UAXIS:
		current_camera.pos[0] += dist * current_camera.uaxis[0];
		current_camera.pos[1] += dist * current_camera.uaxis[1];
		current_camera.pos[2] += dist * current_camera.uaxis[2];
		break;
	case VAXIS:
		current_camera.pos[0] += dist * current_camera.vaxis[0];
		current_camera.pos[1] += dist * current_camera.vaxis[1];
		current_camera.pos[2] += dist * current_camera.vaxis[2];
		break;
	case NAXIS:
		current_camera.pos[0] -= dist * current_camera.naxis[0];
		current_camera.pos[1] -= dist * current_camera.naxis[1];
		current_camera.pos[2] -= dist * current_camera.naxis[2];
		break;
	default:
		break;
	}
}

void renew_cam_orientation(int axis, int angle) {
	glm::mat3 RotationMatrix;
	glm::vec3 direction;

	switch (axis) {
	case UAXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), RUNIT * TO_RADIAN * angle, glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2])));
		direction = RotationMatrix * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
		current_camera.vaxis[0] = direction.x; current_camera.vaxis[1] = direction.y; current_camera.vaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
		current_camera.naxis[0] = direction.x; current_camera.naxis[1] = direction.y; current_camera.naxis[2] = direction.z;
		break;
	case VAXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), RUNIT * TO_RADIAN * angle, glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2])));
		direction = RotationMatrix * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
		current_camera.uaxis[0] = direction.x; current_camera.uaxis[1] = direction.y; current_camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
		current_camera.naxis[0] = direction.x; current_camera.naxis[1] = direction.y; current_camera.naxis[2] = direction.z;
		break;
	case NAXIS:
		RotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), RUNIT * TO_RADIAN * angle, glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2])));
		direction = RotationMatrix * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
		current_camera.uaxis[0] = direction.x; current_camera.uaxis[1] = direction.y; current_camera.uaxis[2] = direction.z;
		direction = RotationMatrix * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
		current_camera.vaxis[0] = direction.x; current_camera.vaxis[1] = direction.y; current_camera.vaxis[2] = direction.z;
		break;
	}
}
/*********************************  END: camera *********************************/

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) {
	char string[256];
	
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_TXPS[3] = {
	{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
	{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_G[3] = {
	{ GL_VERTEX_SHADER, "Shaders/G.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/G.frag" },
	{ GL_NONE, NULL }
	};
	
	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");

	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");

	loc_u_flag_blending = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_blending");
	loc_u_fragment_alpha = glGetUniformLocation(h_ShaderProgram_TXPS, "u_fragment_alpha");

	loc_u_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");

	loc_u_flag_blind = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_blind");

	loc_u_flag_change = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_change");
	loc_u_change_factor = glGetUniformLocation(h_ShaderProgram_TXPS, "u_change_factor");

	// Gouraud shading
	h_ShaderProgram_G = LoadShaders(shader_info_G);
	loc_ModelViewProjectionMatrix_G = glGetUniformLocation(h_ShaderProgram_G, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_G= glGetUniformLocation(h_ShaderProgram_G, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_G = glGetUniformLocation(h_ShaderProgram_G, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color_G = glGetUniformLocation(h_ShaderProgram_G, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light_G[i].light_on = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light_G[i].position = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light_G[i].ambient_color = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light_G[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light_G[i].specular_color = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light_G[i].spot_direction = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light_G[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light_G[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_G, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light_G[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_G, string);
	}

	loc_material_G.ambient_color = glGetUniformLocation(h_ShaderProgram_G, "u_material.ambient_color");
	loc_material_G.diffuse_color = glGetUniformLocation(h_ShaderProgram_G, "u_material.diffuse_color");
	loc_material_G.specular_color = glGetUniformLocation(h_ShaderProgram_G, "u_material.specular_color");
	loc_material_G.emissive_color = glGetUniformLocation(h_ShaderProgram_G, "u_material.emissive_color");
	loc_material_G.specular_exponent = glGetUniformLocation(h_ShaderProgram_G, "u_material.specular_exponent");

}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

//axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {

	//set grid vertices
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//bistro_exterior
GLuint* bistro_exterior_VBO;
GLuint* bistro_exterior_VAO;
int* bistro_exterior_n_triangles;
int* bistro_exterior_vertex_offset;
GLfloat** bistro_exterior_vertices;
GLuint* bistro_exterior_texture_names;

bool* flag_texture_mapping;

void initialize_lights(void) { // follow OpenGL conventions for initialization
	int i;

	find_world_light_EC(); // set loc_light[0].position, loc_light_G[0].position

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);

	// world light
	glUniform1i(loc_light[0].light_on, 0);
	glUniform4f(loc_light[0].ambient_color, 0.13f, 0.13f, 0.13f, 1.0f);
		glUniform4f(loc_light[0].diffuse_color, 0.5f, 0.5f, 0.5f, 0.5f);
		glUniform4f(loc_light[0].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform3f(loc_light[0].spot_direction, 0.0f, 0.0f, -1.0f);
	glUniform1f(loc_light[0].spot_exponent, 0.0f); // [0.0, 128.0]
	glUniform1f(loc_light[0].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
	glUniform4f(loc_light[0].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation

	// moving camera light
	glUniform4f(loc_light[1].position,0.0f, 0.0f, 0.0f,1.0f);
	glUniform1i(loc_light[1].light_on, 0);
	glUniform4f(loc_light[1].ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_light[1].diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_light[1].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform3f(loc_light[1].spot_direction, 0.0f, 0.0f, -1.0f);
	glUniform1f(loc_light[1].spot_exponent, 27.0f); // [0.0, 128.0]
	glUniform1f(loc_light[1].spot_cutoff_angle, 10.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
	glUniform4f(loc_light[1].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation

	// tiger light
	glUniform4f(loc_light[2].position, 0.0f, 0.0f, 0.0f, 1.0f); // will be changed
	glUniform1i(loc_light[2].light_on, 0);
	glUniform4f(loc_light[2].ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_light[2].diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_light[2].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform3f(loc_light[2].spot_direction, 0.0f, 0.0f, -1.0f); // will be changed
	glUniform1f(loc_light[2].spot_exponent, 27.0f); // [0.0, 128.0]
	glUniform1f(loc_light[2].spot_cutoff_angle, 10.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
	glUniform4f(loc_light[2].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation

	/* Gouraud Shader */
	glUseProgram(h_ShaderProgram_G);

	glUniform4f(loc_global_ambient_color_G, 0.2f, 0.2f, 0.2f, 1.0f);

	// world light
	glUniform1i(loc_light_G[0].light_on, 0);
	glUniform4f(loc_light_G[0].ambient_color, 0.13f, 0.13f, 0.13f, 1.0f);
	glUniform4f(loc_light_G[0].diffuse_color, 0.5f, 0.5f, 0.5f, 0.5f);
	glUniform4f(loc_light_G[0].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform3f(loc_light_G[0].spot_direction, 0.0f, 0.0f, -1.0f);
	glUniform1f(loc_light_G[0].spot_exponent, 0.0f); // [0.0, 128.0]
	glUniform1f(loc_light_G[0].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
	glUniform4f(loc_light_G[0].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation

	// moving camera light
	glUniform4f(loc_light_G[1].position, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1i(loc_light_G[1].light_on, 0);
	glUniform4f(loc_light_G[1].ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_light_G[1].diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_light_G[1].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform3f(loc_light_G[1].spot_direction, 0.0f, 0.0f, -1.0f);
	glUniform1f(loc_light_G[1].spot_exponent, 27.0f); // [0.0, 128.0]
	glUniform1f(loc_light_G[1].spot_cutoff_angle, 10.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
	glUniform4f(loc_light_G[1].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation

	glUniform1i(loc_light_G[2].light_on, 0);

	glUseProgram(0);
}

bool readTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_bistro_exterior(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	bistro_exterior_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	bistro_exterior_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	bistro_exterior_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	bistro_exterior_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

	// vertices
	bistro_exterior_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		bistro_exterior_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		bistro_exterior_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			bistro_exterior_vertex_offset[materialIdx] = 0;
		else
			bistro_exterior_vertex_offset[materialIdx] = bistro_exterior_vertex_offset[materialIdx - 1] + 3 * bistro_exterior_n_triangles[materialIdx - 1];

		glGenBuffers(1, &bistro_exterior_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, bistro_exterior_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			bistro_exterior_vertices[materialIdx], GL_STATIC_DRAW);

		// As the geometry data exists now in graphics memory, ...
		free(bistro_exterior_vertices[materialIdx]);

		// Initialize vertex array object.
		glGenVertexArrays(1, &bistro_exterior_VAO[materialIdx]);
		glBindVertexArray(bistro_exterior_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", scene.n_materials);

	// textures
	bistro_exterior_texture_names = (GLuint*)malloc(sizeof(GLuint) * scene.n_textures);
	glGenTextures(scene.n_textures, bistro_exterior_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded bistro exterior textures into graphics memory.\n\n");
	
	free(bistro_exterior_vertices);
}

void draw_bistro_exterior(void) {
	glUseProgram(h_ShaderProgram_TXPS);

	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		glUniform4fv(loc_material.ambient_color, 1, scene.material_list[materialIdx].shading.ph.ka);
		glUniform4fv(loc_material.diffuse_color, 1, scene.material_list[materialIdx].shading.ph.kd);
		glUniform4fv(loc_material.specular_color, 1, scene.material_list[materialIdx].shading.ph.ks);
		glUniform1f(loc_material.specular_exponent, scene.material_list[materialIdx].shading.ph.spec_exp);
		glUniform4fv(loc_material.emissive_color, 1, scene.material_list[materialIdx].shading.ph.kr);

		int texId = scene.material_list[materialIdx].diffuseTexId;
		glUniform1i(loc_texture, texId);
		glUniform1i(loc_flag_texture_mapping, flag_texture_mapping[texId]);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);

		glBindVertexArray(bistro_exterior_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * bistro_exterior_n_triangles[materialIdx]);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}

void prepare_objects_20201551(void) {
	prepare_tiger();
	prepare_ben();
	prepare_spider();
	prepare_dragon();
	prepare_cow();
	prepare_godzilla();
	prepare_ironman();
	prepare_tank();
}

int tank_g = 1; // initially Gouraud Shading

void draw_objects_20201551(void) {
	glUseProgram(h_ShaderProgram_TXPS);

	glFrontFace(GL_CW);

	float angle[3], dist[4]; // angle in rad
	angle[0] = atanf(1.0f / 2.0f);
	dist[0] = 1200.0f / cosf(angle[0]);
	dist[1] = 800.0f + 400.0f / cosf(angle[0]);
	angle[1] = acosf(800.0f / dist[1]);
	angle[2] = 90.0f * TO_RADIAN - angle[1];
	dist[2] = dist[1] - 800.0f * cosf(angle[1]);
	dist[3] = 800.0f * sinf(angle[1]);
	int t[8] = { 200, 20, 150, 600, 150, 20, 200, 20 }; // t = time length
	int clock_tiger = timestamp_tiger % 1360;

	ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(1800.0f + dist[0] + dist[1], -2600.0f, 0.0f));
	ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -angle[0], glm::vec3(0.0f, 0.0f, 1.0f));
	if (clock_tiger<200) { // 직선
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-dist[0]-dist[1]+dist[0]/200.0f * clock_tiger, 0.0f, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_tiger < 220) { // 방향 전환
		clock_tiger -= 200;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-dist[1], 0.0f, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 90.0f * TO_RADIAN + angle[2]/20.0f*clock_tiger, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_tiger < 370) { // 원으로 진입
		clock_tiger -= 220;
		float d = dist[1] * sinf(angle[1]) / 2 / sinf(angle[2] / 2);
		float theta= 90.0f * TO_RADIAN - angle[2] / 2;
		float a = dist[1] - d * cosf(theta);
		float b = d * sinf(theta);
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-dist[1] + dist[2] / 150.0f * clock_tiger, dist[3] / 150.0f * clock_tiger, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 90.0f * TO_RADIAN + angle[2], glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_tiger < 970) { // 원 둘레 회전
		clock_tiger -= 370;
		float a = 360.0f * TO_RADIAN - 2 * angle[1];

		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -angle[1]-a/600.0f*clock_tiger, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-800.0f, 0.0f, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 180.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_tiger < 1120) { // 원 빠져나옴
		clock_tiger -= 970;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-800.0f * cosf(angle[1]) - dist[2] / 150.0f * clock_tiger, -800.0f * sinf(angle[1]) + dist[3] / 150.0f * clock_tiger, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -90.0f * TO_RADIAN - angle[2], glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_tiger < 1140) { // 방향 전환
		clock_tiger -= 1120;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-dist[1],0.0f, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -90.0f * TO_RADIAN - angle[2]+angle[2]/20.0f *clock_tiger, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_tiger < 1340) { // 직선
		clock_tiger -= 1140;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(- dist[1] - dist[0] / 200.0f * clock_tiger, 0.0f, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else { // 방향 180도 회전
		clock_tiger -= 1340;
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(-dist[1] - dist[0], 0.0f, 0.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, (- 90.0f+180.0f/20.0f*clock_tiger) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(0.0f, 0.0f, 20.0f));

	if (mode == CAMERA_T || mode==CAMERA_G) {
		set_ViewMatrix_for_tiger();
	}
	ModelViewMatrix = ViewMatrix * ModelMatrix_tiger;
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix_tiger;

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	if (mode == CAMERA_T || mode == CAMERA_G) {
		find_world_light_EC();
	}
	find_tiger_light_EC(ModelViewMatrix); // glUseProgram(0) 들어있음

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform1i(loc_flag_texture_mapping, true);
	set_material_tiger();
	glUniform1i(loc_texture, TEXTURE_ID_TIGER);
	draw_tiger();

	//$begin draw spider
	float rotation_spider;
	int clock_spider = timestamp % 340;
	float x;
	float PI = 3.14159265359f;

	ModelViewMatrix = ViewMatrix;
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-2000.0f, 1100.0f, 50.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -45.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	
	if (clock_spider < 100) { // sin 그래프
		x = -720.0f + 1440.0f / 100.0f * clock_spider;
		rotation_spider = atanf(-PI*250.0f/ 720.0f * cosf(PI / 720.0f * x));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(x, -250.0f * sinf(PI / 720.0f * x), 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN + rotation_spider, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_spider < 120) { // 방향 변환
		clock_spider -= 100;
		float a =  atanf(250.0f * PI / 720.0f);
		rotation_spider = -90.0f * TO_RADIAN + a + (180.0f * TO_RADIAN - a) / 20.0f * clock_spider;
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(720.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_spider, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_spider < 320) { // 직선 200
		clock_spider -= 120;
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(720.0f-1440.0f/200.0f*clock_spider, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else { // 방향 변환 20
		clock_spider -= 320;
		float a = 180.0f*TO_RADIAN - atanf(250.0f * PI / 720.0f);
		rotation_spider = 90.0f * TO_RADIAN - a / 20.0f * clock_spider;
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-720.0f, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, rotation_spider, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));

	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	draw_spider();

	// $begin draw ben
	int clock_ben = timestamp % 220;
	glUniform1i(loc_texture, TEXTURE_ID_TIGER);

	ModelViewMatrix = ViewMatrix;
	if (clock_ben < 100) { // 직선 <10
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -TO_RADIAN * 130.0f/100.0f* clock_ben, glm::vec3(0.0f, 0.0f, 1.0f)); // 130도 원 둘레
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0f, -800.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (clock_ben < 110) {
		clock_ben -= 100;
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-800.0f * cosf(40.0f * TO_RADIAN), 800.0f * sinf(40.0f * TO_RADIAN), 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (140.0f - 180.0f/10.0f*clock_ben)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f)); // 180도 돌기
	}
	else if (clock_ben < 210) {
		clock_ben -= 110;

		ModelViewMatrix = glm::rotate(ModelViewMatrix, TO_RADIAN * (50.0f+ 130.0f / 100.0f*clock_ben), glm::vec3(0.0f, 0.0f, 1.0f)); // 130도 원 둘레
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0f, 800.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		clock_ben -= 210;
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0f, -800.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90.0f + 180.0f/10.0f * clock_ben) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0f, 0.0f, 10.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(300.0f, 300.0f, 300.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	draw_ben();

	//$begin draw godzilla
	glUniform1i(loc_texture, TEXTURE_ID_TIGER);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(4000.0f, -2000.0f, 30.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(7.0f, 7.0f, 7.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, TO_RADIAN * (-80.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, TO_RADIAN * (90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_godzilla();

	//$begin draw ironman
	glUniform1i(loc_texture, TEXTURE_ID_TIGER);

	ModelMatrix_ironman = glm::mat4(1.0f);
	ModelMatrix_ironman = glm::translate(ModelMatrix_ironman, glm::vec3(-2700.0f, 1700.0f, 20.0f));
	ModelMatrix_ironman = glm::scale(ModelMatrix_ironman, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelMatrix_ironman = glm::rotate(ModelMatrix_ironman, TO_RADIAN * (90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix_ironman = glm::rotate(ModelMatrix_ironman, TO_RADIAN * (90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	ModelViewMatrix = ViewMatrix * ModelMatrix_ironman;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_ironman();

	//$begin draw cow

	int clock_cow= timestamp % 360;

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-900.0f, -400.0f, 120.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	//ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, clock_cow * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(300.0f, 300.0f, 300.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	if (flag_blending) {
		glEnable(GL_BLEND);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1i(loc_u_flag_blending, 1);
		glEnable(GL_CULL_FACE);

		glUniform1f(loc_u_fragment_alpha, cow_alpha);
		glCullFace(GL_BACK);
		draw_cow();
		glCullFace(GL_FRONT);
		draw_cow();

		glUniform1f(loc_u_fragment_alpha, 1.0f);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glUniform1i(loc_u_flag_blending, 0);
	}
	else {
		draw_cow();
	}
	// $begin Draw dragon

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(1000.0f, 3000.0f, 20.0f)); // x800 4000
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -110.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	glUniform1i(loc_flag_texture_mapping, false);
	set_material_ruby_TXPS();
	draw_dragon();

	//$begin draw tank
	if (!tank_g) {
		glUseProgram(h_ShaderProgram_TXPS);

		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f, 2000.0f, 100.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 140.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f)); // 180
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(70.0f, 70.0f, 70.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

		set_material_brass(h_ShaderProgram_TXPS);
	}
	else {
		glUseProgram(h_ShaderProgram_G);

		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f, 2000.0f, 100.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 140.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f)); // 180
		ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(70.0f, 70.0f, 70.0f));
		ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
		ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));

		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_G, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_G, 1, GL_FALSE, &ModelViewMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_G, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

		set_material_brass(h_ShaderProgram_G);
	}
	draw_tank();
	glUseProgram(0);
}
/*****************************  END: geometry setup *****************************/

/********************  START: callback function definitions *********************/
void timer_20201551(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	if (animation_tiger) {
		timestamp_tiger = (timestamp_tiger + 1) % UINT_MAX;
		cur_frame_tiger = timestamp_tiger % N_TIGER_FRAMES;
	}
	cur_frame_spider = timestamp % N_SPIDER_FRAMES;
	cur_frame_ben = timestamp % N_BEN_FRAMES;
	glutPostRedisplay();
	glutTimerFunc(10, timer_20201551, 0);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	draw_bistro_exterior();

	draw_objects_20201551();

	glutSwapBuffers();
}

void keyboard_20201551(unsigned char key, int x, int y) {
	switch (key) {
	case '7': // switch between Gouraud and Phong shading for tank
		tank_g = 1 - tank_g;
		printf("Tank: %s Shading\n", tank_g ? "Gouraud" : "Phong");
		glutPostRedisplay();
		break;
	case '1':
		set_current_camera(CAMERA_1);

		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_flag_fog, 1);
		glUseProgram(0);

		glutPostRedisplay();
		break;
	case '2':
		set_current_camera(CAMERA_2);
		glutPostRedisplay();
		break;
	case '3':
		set_current_camera(CAMERA_3);
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_flag_change, 1);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case '0':
		set_current_camera(CAMERA_0);
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_flag_blind, 1);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'm': // 세상 이동 카메라 모드로 변환
		if (mode == CAMERA_M) {
			// 광원 on/off
			light_on[1] = 1 - light_on[1];
			glUseProgram(h_ShaderProgram_TXPS);
			glUniform1i(loc_light[1].light_on, light_on[1]);
			glUseProgram(h_ShaderProgram_G);
			glUniform1i(loc_light_G[1].light_on, light_on[1]);
			glUseProgram(0);
		}
		else {
			set_current_camera(CAMERA_M);
		}
		glutPostRedisplay();
		break;
	case 't': // 호랑이 관점 카메라 모드
		set_current_camera(CAMERA_T);
		glutPostRedisplay();
		break;
	case 'g': // 호랑이 관찰 카메라 모드
		set_current_camera(CAMERA_G);
		glutPostRedisplay();
		break;
	case 's': // 세상 이동 카메라 -u축 이동 이동
		if (mode == CAMERA_M) {
			renew_cam_position(UAXIS, -TUNIT);
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'f': // 세상 이동 카메라 u축 이동
		if (mode == CAMERA_M) {
			renew_cam_position(UAXIS, TUNIT);
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'e': // 세상 이동 카메라 -n축 이동
		if (mode == CAMERA_M) {
		renew_cam_position(NAXIS, TUNIT);
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		}
		break;
	case 'd': // 세상 이동 카메라 n축 이동
		if (mode == CAMERA_M) {
		renew_cam_position(NAXIS, -TUNIT);
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		}
		break;
	case 'q': // 세상 이동 카메라 v축 이동
			if (mode == CAMERA_M) {
			renew_cam_position(VAXIS, TUNIT);
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
			}
		break;
	case 'a': // 세상 이동 카메라 -v 축 이동
			if (mode == CAMERA_M) {
			renew_cam_position(VAXIS, -TUNIT);
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
			}
		break;
	case ' ': // 호랑이 이동/멈춤
		animation_tiger = 1 - animation_tiger;
		break;
	case 'w': // 세상 광원 on/off
		light_on[0] = 1 - light_on[0];
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_light[0].light_on, light_on[0]);
		glUseProgram(h_ShaderProgram_G);
		glUniform1i(loc_light_G[0].light_on, light_on[0]);
		glUseProgram(0);
		break;
	case 'b': // 호랑이 광원 on/off
		glUseProgram(h_ShaderProgram_TXPS);
		light_on[2] = 1 - light_on[2];
		glUniform1i(loc_light[2].light_on, light_on[2]);
		glUseProgram(0);
		break;
	case 'z':
		flag_blending = 1 - flag_blending;
		printf("Cow: blending mode %s\n", flag_blending ? "ON" : "OFF");
		break;
	}

	if (mode != CAMERA_M && light_on[1]) {
		// 광원 off
		light_on[1] = 0;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_light[1].light_on, 0);
		glUseProgram(h_ShaderProgram_G);
		glUniform1i(loc_light_G[1].light_on, 0);
		glUseProgram(0);
	}
	if (mode != CAMERA_1) {
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_flag_fog, 0);
		glUseProgram(0);
	}
	if (mode != CAMERA_0) {
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_flag_blind, 0);
		glUseProgram(0);
	}
	if (mode != CAMERA_3) {
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_flag_change, 0);
		glUseProgram(0);
	}
}

void special_20201551(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
	case GLUT_KEY_DOWN:
		if (flag_blending) {
			cow_alpha -= 0.05f;
			if (cow_alpha < 0.0f) cow_alpha = 0.0f;
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_UP:
	case GLUT_KEY_RIGHT:
		if (flag_blending) {
			cow_alpha += 0.05f;
			if (cow_alpha > 1.0f) cow_alpha = 1.0f;
			glutPostRedisplay();
		}
		break;
	}
}

void mouse_20201551(int button, int state, int x, int y) {
	if (mode != CAMERA_M) return;

	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			current_camera.move = 1;
			current_camera.rotation_axis = UAXIS; // u 또는 v axis 중심으로 회전
			prevx = x; prevy = y;
		}
		else if (state == GLUT_UP) {
			current_camera.move = 0;
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {
			current_camera.move = 1;
			current_camera.rotation_axis = NAXIS; // n axis 중심으로 회전
			prevx = x; prevy = y;
		}
		else if (state == GLUT_UP) {
			current_camera.move = 0;
		}
		break;
	}
}

void wheel_20201551(int wheel, int direction, int x, int y) {
	if (glutGetModifiers() != GLUT_ACTIVE_SHIFT) return;
	// When shift key is pressed

	if (direction > 0) { // Zoom in
		current_camera.fovy -= TO_RADIAN;
		change_factor -= 1.0f;
		ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
	}
	else { // Zoom out
		current_camera.fovy += TO_RADIAN;
		change_factor += 1.0f;
		ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
	}
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1f(loc_u_change_factor, change_factor);
	glUseProgram(0);
}

void motion_20201551(int x, int y) {
	if (!current_camera.move) return;

	if (current_camera.rotation_axis == NAXIS) {
		renew_cam_orientation(NAXIS, y - prevy); // 시계/반시계 회전
	}
	else {
		renew_cam_orientation(UAXIS, y - prevy);  // 위/아래 바라보기
		renew_cam_orientation(VAXIS, x - prevx);  // 왼쪽/오른쪽 바라보기

	}

	prevx = x; prevy = y;

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void reshape(int width, int height) {
	float aspect_ratio;

	glViewport(0, 0, width, height);

	current_camera.aspect_ratio = (float)width / height; // chk

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, bistro_exterior_VAO);
	glDeleteBuffers(scene.n_materials, bistro_exterior_VBO);

	glDeleteVertexArrays(1, &tiger_VAO);
	glDeleteBuffers(1, &tiger_VBO);

	glDeleteVertexArrays(1, &ben_VAO);
	glDeleteBuffers(1, &ben_VBO);

	glDeleteVertexArrays(1, &spider_VAO);
	glDeleteBuffers(1, &spider_VBO);

	glDeleteVertexArrays(1, &dragon_VAO);
	glDeleteBuffers(1, &dragon_VBO);

	glDeleteVertexArrays(1, &tank_VAO);
	glDeleteBuffers(1, &tank_VBO);

	glDeleteVertexArrays(1, &cow_VAO);
	glDeleteBuffers(1, &cow_VBO);

	glDeleteVertexArrays(1, &ironman_VAO);
	glDeleteBuffers(1, &ironman_VBO);

	glDeleteVertexArrays(1, &godzilla_VAO);
	glDeleteBuffers(1, &godzilla_VBO);

	glDeleteTextures(scene.n_textures, bistro_exterior_texture_names);

	free(bistro_exterior_n_triangles);
	free(bistro_exterior_vertex_offset);

	free(bistro_exterior_VAO);
	free(bistro_exterior_VBO);

	free(bistro_exterior_texture_names);
	free(flag_texture_mapping);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard_20201551);
	glutSpecialFunc(special_20201551);
	glutMotionFunc(motion_20201551);
	glutMouseFunc(mouse_20201551);
	glutMouseWheelFunc(wheel_20201551);
	glutReshapeFunc(reshape);
	glutTimerFunc(10, timer_20201551, 0);
	glutCloseFunc(cleanup);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
}

void prepare_scene(void) {
	prepare_bistro_exterior();
	prepare_objects_20201551();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 9
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Bistro Exterior Scene";
	char messages[N_MESSAGE_LINES][256] = { 
		"    - Keys used:",
		"		'f' : draw x, y, z axes and grid",
		"		'1' : set the camera for original view",
		"		'2' : set the camera for bistro view",
		"		'3' : set the camera for tree view",	
		"		'4' : set the camera for top view",
		"		'5' : set the camera for front view",
		"		'6' : set the camera for side view",
		"		'ESC' : program close",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
