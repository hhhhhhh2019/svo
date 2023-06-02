#include <obj_parser.h>
#include <utils.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>



ObjParsedFile parse_file(char* filename) {
	int size;
	char* file_data;

	read_file(filename, &file_data, &size);


	ObjParsedFile result = {0,0,0,0, malloc(0),malloc(0),malloc(0),malloc(0)};

	char* str = calloc(1,1);

	char is_comment = 0;
	char is_vertex = 0;
	char is_tex_vertex = 0;
	char is_normal = 0;
	char is_face = 0;

	int data_id = 0;

	for (int q = 0; q < size; q++) {
		char c = file_data[q];

		if (c == '\n' || c == ' ' || c == '\t' || c == '\r' || c == '#' || c == 0) {
			if (strlen(str) == 0 || is_comment) {}
			else if (strcmp(str, "v") == 0) {
				is_vertex = 1;
				is_tex_vertex = 0;
				is_normal = 0;
				is_face = 0;
				data_id = 0;

				result.vertexes_count += 1;
				result.vertexes = realloc(result.vertexes, sizeof(vec3f) * result.vertexes_count);
			} else if (strcmp(str, "vt") == 0) {
				is_vertex = 0;
				is_tex_vertex = 1;
				is_normal = 0;
				is_face = 0;
				data_id = 0;

				result.tex_vertexes_count += 1;
				result.tex_vertexes = realloc(result.tex_vertexes, sizeof(vec2f) * result.tex_vertexes_count);
			} else if (strcmp(str, "vn") == 0) {
				is_vertex = 0;
				is_tex_vertex = 0;
				is_normal = 1;
				is_face = 0;
				data_id = 0;

				result.normals_count += 1;
				result.normals = realloc(result.normals, sizeof(vec3f) * result.normals_count);
			} else if (strcmp(str, "f") == 0) {
				is_vertex = 0;
				is_tex_vertex = 0;
				is_normal = 0;
				is_face = 1;
				data_id = 0;

				result.faces_count++;
				result.faces = realloc(result.faces, sizeof(obj_face) * result.faces_count);
				result.faces[result.faces_count-1].count = 0;
				result.faces[result.faces_count-1].vertexes = malloc(0);
				result.faces[result.faces_count-1].tex_vertexes = malloc(0);
				result.faces[result.faces_count-1].normals = malloc(0);
			} else {
				if (is_vertex) {
					result.vertexes[result.vertexes_count-1].data[data_id++] = atof(str);
				} else if (is_tex_vertex) {
					result.tex_vertexes[result.tex_vertexes_count-1].data[data_id++] = atof(str);
				} else if (is_normal) {
					result.normals[result.normals_count-1].data[data_id++] = atof(str);
				} else if (is_face) {
					int ids[3] = {0,0,0};
					int fid = 0;
					int fst = 0;

					for (int i = 0; i < strlen(str)+1; i++) {
						if (str[i] == 0) {
							ids[fid++] = atoi(str+fst)-1;
							fst = i+1;
						}

						if (str[i] == '/') {
							str[i] = 0;
							ids[fid++] = atoi(str+fst)-1;
							str[i] = '/';
							fst = i+1;
						}
					}

					obj_face* face = &result.faces[result.faces_count-1];

					face->count++;
					face->vertexes = realloc(face->vertexes, sizeof(int) * face->count);
					face->tex_vertexes = realloc(face->tex_vertexes, sizeof(int) * face->count);
					face->normals = realloc(face->normals, sizeof(int) * face->count);

					face->vertexes[face->count-1] = ids[0];
					face->tex_vertexes[face->count-1] = ids[1];
					face->normals[face->count-1] = ids[2];
				}
			}

			free(str);
			str = calloc(1,1);
		}

		if (c == '#') {
			is_comment = 1;
			is_vertex = 0;
			is_tex_vertex = 0;
			is_normal = 0;
			is_face = 0;
		}

		if (c == '\n')
			is_comment = 0;

		if (is_comment)
			continue;

		if (c == '\n' || c == ' ' || c == '\t' || c == '\r' || c == '#' || c == 0)
			continue;

		int len = strlen(str);
		str = realloc(str,len+2);
		str[len] = c;
		str[len+1] = 0;
	}


	free(file_data);
	free(str);

	return result;
}
