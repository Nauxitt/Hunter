#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include "userdata.h"
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include "hunter.h"
#include <string.h>


char __data_dir[128];

char * dataDir(){
	/*
	   Returns a string denoting the data directory and creates it if it doesn't exist.

	   Returns NULL on error
   */

	// Exit if the calculations below have already been performed
	if(__data_dir[0])
		return (char*) __data_dir;

	// Get home directory, error if can't
	char * home = getenv("HOME");
	if(home == NULL)
		return NULL;

	// Store data directory
	sprintf((char*) &__data_dir, "%s/.hunter", home);

	// Create data directory if it doesn't exist
	DIR * dir = opendir((char*) &__data_dir);
	if(dir){
		closedir(dir);
		return (char*) &__data_dir;
	}
	else if (ENOENT == errno){
		// Make directory
#ifdef _WIN32
		mkdir((char*) &__data_dir);
#else
		mkdir((char*) &__data_dir, 0755);
#endif
	}
	return NULL;
}

char * dataPath(char * dest, char * sub_path){
	sprintf(dest, "%s/%s", dataDir(), sub_path);
	return dest;
}

void hunterSaveAt(Hunter * hunter, char * fpath){
	/*
		Saves hunter data at a specified path.
	*/

	char buffer[256];
	encodeHunter(hunter, (char*) &buffer);
	
	// Get data length
	char * seek = (char*) &buffer;
	
	// Seek NULL byte, ending the header
	while(*seek++ != 0);
	
	// Decode length from characters
	uint32_t length;
	length  = *seek++ << 24;
	length |= *seek++ << 16;
	length |= *seek++ <<  8;
	length |= *seek++;
	
	// Save hunter data
	FILE * fd = fopen(fpath, "wb");
	// TODO: error check
	
	fwrite(&buffer, length, 1, fd);
	fclose(fd);
}

void hunterSave(Hunter * hunter){
	/*
	   Saves hunter in $DATA/hunters/$HUNTER_NAME.hunter
	*/

	char fpath[256];

	// Get hunter filepath
	sprintf(
			(char*) &fpath,
			"%s/hunters/%s.hunter",
			dataDir(),
			(char*) hunter->name
		);

	hunterSaveAt(hunter, (char*) &fpath);
}
