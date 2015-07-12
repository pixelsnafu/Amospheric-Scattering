/**
 * Auxillary function to set up a GLSL shader program.  
 * requires the name of a vertex program and a fragment
 * program.  Returns a handle to the created GLSL program
 *
 * vert - Name of source file for vertex program
 * frag - Name of source file for fragment program
 */

#ifndef __shaderSetup__
#define __shaderSetup__

GLuint setUpAShader (char *vert, char *frag);

#endif