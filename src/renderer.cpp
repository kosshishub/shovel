// Standard
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <renderer.hpp>

#include <terrainmesher.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_STATIC
#include <GLFW\glfw3.h>

#include <windows.h>

#include "../build/res.h"

#define PI 3.14159265


// kosshis renderer
namespace krdr {

	GLuint createShader(const char vertSrc[], const char fragSrc[]){

		int InfoLogLength = 0;

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vertSrc, NULL);
		glCompileShader(vs);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fragSrc, NULL);
		glCompileShader(fs);

		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			char * ProgramErrorMessage = (char*)malloc(InfoLogLength+1);
			glGetShaderInfoLog(vs, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("Vertex shader failed to compile! LOG:\n%s\n", &ProgramErrorMessage[0]);
			MessageBox(0, "Vertex shader failed to compile!", "Error!", 0);
		}
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			char * ProgramErrorMessage = (char*)malloc(InfoLogLength+1);
			glGetShaderInfoLog(fs, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("Fragment shader failed to compile! LOG:\n%s\n", &ProgramErrorMessage[0]);
			MessageBox(0, "Fragment shader failed to compile!", "Error!", 0);
		}

		GLuint shader_program = glCreateProgram();
		glAttachShader(shader_program, fs);
		glAttachShader(shader_program, vs);
		glLinkProgram(shader_program);

		glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			char * ProgramErrorMessage = (char*)malloc(InfoLogLength+1);
			glGetProgramInfoLog(shader_program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			printf("SHADERPROGRAM LINK FAILURE! LOG:\n%s\n", &ProgramErrorMessage[0]);
			MessageBox(0, "Shader link failure!", "Error!", 0);
		}

		return shader_program;
	}

	void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		printf("%i %i\n", width, height);
		glViewport(0, 0, width, height);
	}



	GLFWwindow* window;

	GLuint shader;
	GLuint shader_uMP;
	GLuint shader_uMV;
	GLuint shader_uFogColor;
	GLuint shader_aVertex;
	GLuint shader_aColor;


	GLuint textShader;
	GLuint textShader_aCoord;

	GLuint textShader_uTex;

	GLuint textShader_uColor;

	GLuint text_vbo;
	GLuint text_tex;


	FT_Library ft;
	FT_Face face;
	FT_GlyphSlot g;

	static void error_callback(int error, const char* description){
	    MessageBox(0, description, "Error!", 0);
	}


	GLFWwindow* getWindow(){
		return window;
	}

	int init(){
		if (!glfwInit())
			return -1;

		glfwSetErrorCallback(error_callback);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);

		int width = 640, height = 480;
		window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);

		if (!window) {
			printf("Window creation failed!\n");
			glfwTerminate();
			return -1;
		}

		glfwMakeContextCurrent(window);
		glewExperimental = GL_TRUE;
		GLenum game_glewinit = glewInit();

		if(game_glewinit){
			printf("Glew Failed: %s\n", glewGetErrorString(game_glewinit));
			MessageBox(0, "GLEW failed!", "Error!", 0);
			return -1;
		}


		if(FT_Init_FreeType(&ft)) {
			fprintf(stderr, "Could not init freetype library\n");
			MessageBox(0, "FreeType failed to initialize.", "Error!", 0);
			return -1;
		}

		if(FT_New_Face(ft, "Consolas.ttf", 0, &face)) {
			fprintf(stderr, "Could not open font\n");
			MessageBox(0, "Could not open font!", "Error!", 0);
			return -1;
		}

		FT_Set_Pixel_Sizes(face, 0, 48);
		
		if(FT_Load_Char(face, 'X', FT_LOAD_RENDER)) {
			fprintf(stderr, "Could not load character 'X'\n");
			MessageBox(0, "FreeType is not working!", "Error!", 0);
			return -1;
		}
		g = face->glyph;


		const GLubyte* renderer = glGetString(GL_RENDERER); 
		const GLubyte* version = glGetString(GL_VERSION); 
		printf("Renderer: %s\n", renderer);
		printf("OpenGL %s\n", version);

		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS); 


		// SHADER
		shader = createShader( chunkVertSrc, chunkFragSrc );
		glUseProgram(shader);
		shader_aVertex = 		glGetAttribLocation (shader, "aVertex");
		shader_aColor =  		glGetAttribLocation (shader, "aColor");
		shader_uMP =     		glGetUniformLocation(shader, "uMP");
		shader_uMV =     		glGetUniformLocation(shader, "uMV");
		shader_uFogColor =      glGetUniformLocation(shader, "uFogColor");

		textShader = createShader( textVertSrc, textFragSrc );
		glUseProgram(textShader);
		textShader_aCoord =  glGetAttribLocation (textShader, "aCoord");
		textShader_uTex = glGetUniformLocation(textShader, "uTex");
		textShader_uColor =  glGetUniformLocation(textShader, "uColor");

		GLuint text_tex;
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &text_tex);
		glBindTexture(GL_TEXTURE_2D, text_tex);
		glUniform1i(textShader_uTex, 0);
		
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenBuffers(1, &text_vbo);
		glEnableVertexAttribArray(textShader_aCoord);
		glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
		glVertexAttribPointer(textShader_aCoord, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glDisableVertexAttribArray(textShader_aCoord);

		return 0;
	}

	bool wireFrame = false;

	void toggleWireframe(){
		wireFrame = !wireFrame;
			glPolygonMode( GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL );

	}

	glm::vec3 screenToWorldSpaceVector(int x, int max_x, int y, int max_y, glm::mat4 pRotMat){
		glm::vec4 ray_clip = {
			 (x/(float)max_x) * 2.0f - 1.0f,
			-(y/(float)max_y) * 2.0f + 1.0f,
			-1.0f, 1.0f
		};

		glm::vec4 ray = glm::inverse(pRotMat) * ray_clip;

		return glm::normalize(ray);
	}

	void meshTick(){
		while(1){
			TerrainMesher::MeshBuffer* mesh = TerrainMesher::getMesh();
			if(mesh == NULL) break;

			int i = mesh->chunk;
			TerrainMesher::ChunkList* chunks = TerrainMesher::getChunks();

			// Check if buffers have ever been generated for this chunk, if no, gen!
			if( chunks->state[i] & 0b10 ){
				glGenBuffers(1, &chunks->vertexBuffer[i] );
				glGenBuffers(1, &chunks->indexBuffer [i] );
				glGenBuffers(1, &chunks->colorBuffer [i] );
			}
			
			chunks->state[i] = chunks->state[i] & 0b1;	

			glBindBuffer(GL_ARRAY_BUFFER, chunks->vertexBuffer[i]);
			int size = mesh->vertexCount * sizeof(float);
			glBufferData(GL_ARRAY_BUFFER, size, mesh->vertexBuffer, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, chunks->colorBuffer[i]);
			size = mesh->vertexCount * sizeof(float);
			glBufferData(GL_ARRAY_BUFFER, size, mesh->colorBuffer, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunks->indexBuffer[i]);
			size = mesh->indexCount * sizeof(int);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, mesh->indexBuffer, GL_STATIC_DRAW);

			chunks->items[i] = mesh->indexCount;


			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	0);
			glBindBuffer(GL_ARRAY_BUFFER, 			0);

			TerrainMesher::meshUsed();
		}
	}

	void draw(glm::mat4 modelview, glm::mat4 projection){
		glUseProgram(shader);
		glEnableVertexAttribArray(shader_aVertex);
		glEnableVertexAttribArray(shader_aColor);

		glEnable(GL_CULL_FACE); 
		glCullFace(GL_BACK);

		glUniformMatrix4fv(shader_uMV, 1, GL_FALSE, &modelview[0][0]);
		glUniformMatrix4fv(shader_uMP, 1, GL_FALSE, &projection[0][0]);

		meshTick();

		TerrainMesher::ChunkList* chunks = TerrainMesher::getChunks();

		glEnableVertexAttribArray(shader_aVertex);
		glEnableVertexAttribArray(shader_aColor);


		int count = 0;
		for (int i = 0; i < chunks->count; i++) {
			count++;
			if ( chunks->items[i] > 0 ) {

				glBindBuffer(GL_ARRAY_BUFFER, 			chunks->colorBuffer[i] );
				// glVertexAttribPointer(shader_aColor, 3, GL_BYTE, GL_TRUE, 0, NULL);
				glVertexAttribPointer(shader_aColor, 3, GL_FLOAT, GL_FALSE, 0, NULL);

				glBindBuffer(GL_ARRAY_BUFFER, 			chunks->vertexBuffer[i]);
				glVertexAttribPointer(shader_aVertex, 3, GL_FLOAT, GL_FALSE, 0, NULL);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	chunks->indexBuffer [i]);

				glDrawElements(GL_TRIANGLES, chunks->items[i], GL_UNSIGNED_INT, 0);	
			}
		}


		glDisableVertexAttribArray(shader_aVertex);
		glDisableVertexAttribArray(shader_aColor);

		glDisable(GL_CULL_FACE); 
	}

	void drawText(const char *text, float x, float y, float sx, float sy, int fontSize) {
		glUseProgram(textShader);
		glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
		glEnableVertexAttribArray(textShader_aCoord);

		FT_Set_Pixel_Sizes(face, 0, fontSize);
		GLfloat black[4] = {0, 0, 0, 1};
		glUniform4fv(textShader_uColor, 1, black);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, text_tex);
		glUniform1i(textShader_uTex, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		const char *p;

		for(p = text; *p; p++) {
			if(FT_Load_Char(face, *p, FT_LOAD_RENDER))
					continue;
	 	
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer
			);
	 
			float x2 = x + g->bitmap_left * sx;
			float y2 = -y - g->bitmap_top * sy;
			float w = g->bitmap.width * sx;
			float h = g->bitmap.rows * sy;
	 
			GLfloat box[4][4] = {
					{x2,     -y2    , 0, 0},
					{x2 + w, -y2    , 1, 0},
					{x2,     -y2 - h, 0, 1},
					{x2 + w, -y2 - h, 1, 1},
			};

			glVertexAttribPointer(textShader_aCoord, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	 
			x += (g->advance.x/64) * sx;
			y += (g->advance.y/64) * sy;
		}

		glDisableVertexAttribArray(textShader_aCoord);

		GLenum err;
		while((err = glGetError()) != GL_NO_ERROR)
		{
			printf("OPENGL ERROR %i\n", err);
		}
	}

	void setFogColor(float r, float g, float b, float a){

		glClearColor(r, g, b, a);

		glUseProgram(shader);

		glUniform3f(shader_uFogColor, r,g,b);
	}

	double getTime(){
		return glfwGetTime();
	}

	int windowShouldClose(){
		return glfwWindowShouldClose(window);
	}

	void startFrame(){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void swapBuffers(){
		glfwSwapBuffers(window);
	}


	void getWindowSize(int *width, int *height){
		glfwGetWindowSize(window, width, height);
	}

	void setWindowTitle(char titlebuffer[]){
		glfwSetWindowTitle(window, titlebuffer);
	}


} // end namespace