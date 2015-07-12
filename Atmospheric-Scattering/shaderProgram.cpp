// ShaderSetup.cpp
//
// simple reading and writing for text files
//
// www.lighthouse3d.com
//
// You may use these functions freely.
// they are provided as is, and no warranties, either implicit,
// or explicit are given
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <OPENGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif


char *textFileRead(char *fn) {


	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	else {
		fprintf (stderr, "Error reading %s\n", fn);
	}
	return content;
}

int textFileWrite(char *fn, char *s) {

	FILE *fp;
	int status = 0;

	if (fn != NULL) {
		fp = fopen(fn,"w");

		if (fp != NULL) {
			
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}




void printShaderInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    GLint infologLength = 0;
    GLsizei charsWritten  = 0;
    char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}


/**
 * Auxillary function to set up a GLSL shader program.  requires the name of a vertex program and a fragment
 * program.  Returns a handle to the created GLSL program
 *
 * vert - Name of source file for vertex program
 * frag - Name of source file for fragment program
 */

GLuint setUpAShader (char *vert, char *frag)
{
	// Read in shader source
	char *vs = NULL,*fs = NULL;
	
	// Create the shader 
	GLuint the_vert = glCreateShader(GL_VERTEX_SHADER);
	GLuint the_frag = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Read in shader source
	vs = textFileRead(vert);
	if (!vs) {
		fprintf (stderr, "Error reading vertex shader file %s\n", vert);
		return 0;
	}
	fs = textFileRead(frag);
	if (!fs) {
		fprintf (stderr, "Error reading fragment shader file %s\n", frag);
		return 0;
	}

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(the_vert, 1, &vv,NULL);
	glShaderSource(the_frag, 1, &ff,NULL);

	free(vs);free(fs);
	
	// Compile the shader
	glCompileShader(the_vert);
	glCompileShader(the_frag);
	printShaderInfoLog(the_vert);
	printShaderInfoLog(the_frag);
	
	// Create the program -- attaching your shaders
	GLuint the_program = glCreateProgram();
	glAttachShader(the_program, the_vert);
	glAttachShader(the_program, the_frag);
	printProgramInfoLog(the_program);
	
	// Link the program
	glLinkProgram(the_program);
	printProgramInfoLog(the_program);
	
	return the_program;

}
 


