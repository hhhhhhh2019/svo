#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H


#include <vector.h>


typedef struct {
	int* vertexes;
	int* tex_vertexes;
	int* normals;

	int count;
} obj_face;

typedef struct {
	int vertexes_count;
	int tex_vertexes_count;
	int normals_count;
	int faces_count;

	vec3f* vertexes;
	vec3f* tex_vertexes;
	vec3f* normals;
	obj_face* faces;
} ObjParsedFile;


ObjParsedFile parse_file(char*);


#endif // OBJ_PARSER_H
