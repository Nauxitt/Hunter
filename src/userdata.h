#ifndef __userdata_h
#define __userdata_h

#include "hunter.h"

extern char __data_dir[128];

char * dataDir();
char * dataPath(char * dest, char * sub_path);

void hunterSaveAt(Hunter * hunter, char * fpath);
void hunterSave(Hunter * hunter);

int userdataMainSave();
int usermain();

#endif
