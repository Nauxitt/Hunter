#ifndef __userdata_h
#define __userdata_h

extern char __data_dir[128];

char * dataDir();
char * dataPath(char * dest, char * sub_path);
int usermain();

#endif
