#ifndef _INC_APP_HPP_
#define _INC_APP_HPP_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


#define LOGD(mes) std::cout << "[DEBUG] " << mes << std::endl
#define LOGE(mes) std::cerr << "[ERROR] " << mes << std::endl


class App {
public:
  App() {
    buffer = (GLubyte*)calloc(N * N * 4, sizeof(GLubyte));
  }
  ~App() {
    free(buffer);
  }
  int initGL(const int width, const int height);
  void draw();
  void updateFrame();
private:
  GLuint compileShader(GLenum type, const char *source);
  GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader, const char * vertexPositionName);
    
  struct _uniform { GLint tex; };
  GLuint program;
  SDL_Surface* screen;
  _uniform uniform;
  GLubyte* buffer;
  const int N = 512;
};

#define BUF(x, y, i) buffer[((x) * N + (y)) * 4 + i]

static double x = 0.0, y = 0.0, e = 0.0, f = 1.0;
static double a = -1.4, b = 1.6, c = 1.0, d = 0.7;

void App::updateFrame() {
  glClear(GL_COLOR_BUFFER_BIT); 
  //memset(buffer, 0, N * N * 4);
  for(int i = 0; i < N * N * 4; i++) buffer[i] *= 0.9;
  a = -1.4 + sin(e) / 2;
  b = -1.6 + sin(f) / 2;
  c = 1.0 + cos(f) / 2;
  d = 0.7 + cos(e) / 2;
  e = e > 3.1415 * 2 ? 0 : e + 0.01;
  f = f > 3.1415 * 2 ? 0 : f + 0.011;
  for(int i = 0; i < 100000; ++i) {
    double nx = sin(a * y) + c * cos(a * x);
    double ny = sin(b * x) + d * cos(b * y);
    x = nx;
    y = ny;
    int sx = N / 2 - x * (N / 6);
    int sy = N / 2 + y * (N / 6);
    sx = std::min(N, std::max(0, sx));
    sy = std::min(N, std::max(0, sy));
    const int div = N / 256;
    BUF(sy, sx, 0) = sx / div;
    BUF(sy, sx, 1) = sy / div;
    BUF(sy, sx, 2) = (sx * sy) / (div * div);
  }
  draw();
}

int App::initGL(const int width, const int height) {
	//initialise SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    LOGE("SDL_Init" << SDL_GetError());
    return 0;
  }
  screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL);
  if (screen == NULL) {
	  LOGE("SDL_SetVideoMode" << SDL_GetError());
		return 0;
	}
	
	//SDL initialised successfully, now load shaders and geometry
	const char vs_src[] = R"(
		attribute vec4 vPos;
		void main() {
		   gl_Position = vPos;
    })";

	const char fs_src[] = R"(
		precision mediump float;
    uniform sampler2D tex;
    void main() {
      vec4 color = texture2D(tex, gl_FragCoord.xy / 512.0);
      color.a = 0.001;
		  gl_FragColor = vec4(color);
		})";

	//load vertex and fragment shaders
	auto vs = compileShader(GL_VERTEX_SHADER, vs_src);
	auto fs = compileShader(GL_FRAGMENT_SHADER, fs_src);
	linkProgram(vs, fs, "vPos");

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glViewport(0, 0, width, height);

  uniform.tex = glGetUniformLocation(program, "tex");  
  GLuint texid;
  glGenTextures(1, &texid);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  
  glBindTexture(GL_TEXTURE_2D, texid);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  return 1;
}

void App::draw() {
  glUseProgram(program);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  
  static GLfloat vert[] = {
    -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 0.0f
  };
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vert);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


GLuint App::compileShader(GLenum type, const char *source) {
	auto shader = glCreateShader(type);
	if (shader == 0) {
		LOGE("glCreateShader");
		return 0;
	}

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(!status) {
    LOGE("glCompileShader");
    
    GLsizei bufSize;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &bufSize);
    if (bufSize > 1) {
      GLchar *infoLog = (GLchar *)malloc(bufSize);
      if (infoLog != NULL) {
        GLsizei length;
        glGetShaderInfoLog(shader, bufSize, &length, infoLog);
        LOGE(infoLog);
        free(infoLog);
      }         
    }
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint App::linkProgram(GLuint vs, GLuint fs, const char* vPosName) {
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glBindAttribLocation(program, 0, vPosName);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		LOGE("glLinkProgram");
		glDeleteProgram(program);
		return 0;
  }
  
  return 1;
}

#endif
