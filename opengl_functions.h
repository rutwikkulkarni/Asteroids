#ifndef OPENGL_FUNCTIONS
#define OPENGL_FUNCTIONS

#define GLLoadFunc(name, name_caps) static PFN##name_caps##PROC name;

#define ALL_GL_FUNCTIONS \
GLLoadFunc(wglChoosePixelFormatARB, WGLCHOOSEPIXELFORMATARB); \
GLLoadFunc(wglCreateContextAttribsARB, WGLCREATECONTEXTATTRIBSARB); \
GLLoadFunc(glGenBuffers, GLGENBUFFERS); \
GLLoadFunc(glBindBuffer, GLBINDBUFFER); \
GLLoadFunc(glGenVertexArrays, GLGENVERTEXARRAYS); \
GLLoadFunc(glBindVertexArray, GLBINDVERTEXARRAY); \
GLLoadFunc(glBufferData, GLBUFFERDATA); \
GLLoadFunc(glVertexAttribPointer, GLVERTEXATTRIBPOINTER); \
GLLoadFunc(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY); \
GLLoadFunc(glCreateShader, GLCREATESHADER); \
GLLoadFunc(glUseProgram, GLUSEPROGRAM); \
GLLoadFunc(glCompileShader, GLCOMPILESHADER); \
GLLoadFunc(glCreateProgram, GLCREATEPROGRAM); \
GLLoadFunc(glLinkProgram, GLLINKPROGRAM); \
GLLoadFunc(glShaderSource, GLSHADERSOURCE); \
GLLoadFunc(glGetShaderiv, GLGETSHADERIV); \
GLLoadFunc(glAttachShader, GLATTACHSHADER); \
GLLoadFunc(glGetUniformLocation, GLGETUNIFORMLOCATION); \
GLLoadFunc(glUniformMatrix4fv, GLUNIFORMMATRIX4FV); \
GLLoadFunc(glBufferSubData, GLBUFFERSUBDATA); \
GLLoadFunc(wglSwapIntervalEXT, WGLSWAPINTERVALEXT); \
GLLoadFunc(glGenFramebuffers, GLGENFRAMEBUFFERS); \
GLLoadFunc(glBindFramebuffer, GLBINDFRAMEBUFFER); \
GLLoadFunc(glFramebufferTexture2D, GLFRAMEBUFFERTEXTURE2D); \
GLLoadFunc(glCheckFramebufferStatus, GLCHECKFRAMEBUFFERSTATUS); \
GLLoadFunc(glGenRenderbuffers, GLGENRENDERBUFFERS); \
GLLoadFunc(glBindRenderbuffer, GLBINDRENDERBUFFER); \
GLLoadFunc(glRenderbufferStorage, GLRENDERBUFFERSTORAGE); \
GLLoadFunc(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER); \
GLLoadFunc(glGetShaderInfoLog, GLGETSHADERINFOLOG);


ALL_GL_FUNCTIONS

void LoadOpenGLFunctions(){
#define GLLoadFunc(name, name_caps) name = (PFN##name_caps##PROC)wglGetProcAddress(#name)
    ALL_GL_FUNCTIONS
}

#endif /* OPENGL_FUNCTIONS */
