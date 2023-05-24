#ifndef SVO_H
#define SVO_H


#define MAX_LEVEL 16


struct ONode;

typedef struct ONode {
	struct ONode* childs[8];
	void* data;
	unsigned char data_dist; // 1 - data in this node
} ONode;

typedef struct {
	ONode* root;
	char levels;
} SVO;


SVO init_svo(char);
void free_svo(SVO);

char  add_data(SVO*, unsigned char, char[], void*);
void  del_data(SVO*, unsigned char, char[]);
void* get_data(SVO*, unsigned char, char[]);

void coords_to_ids(short,short,short, char[]);

void print_svo(SVO);

void save_svo_to_file(SVO, char*);
SVO load_svo_from_file(char*);


#endif // SVO_H
