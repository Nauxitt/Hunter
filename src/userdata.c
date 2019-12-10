#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include "userdata.h"
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include "hunter.h"


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
		mkdir((char*) &__data_dir, 0755);
	}
	else {
		return NULL;
	}
}

char * dataPath(char * dest, char * sub_path){
	sprintf(dest, "%s/%s", dataDir(), sub_path);
	return dest;
}

int usermain(){
	// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
	
	char buffer[256];

	// Linked list struct type for Hunter reading
	struct HunterFileLinkedList {
		char fpath[128];
		struct stat attr;
		
		Hunter hunter;
		
		// double-linked list
		struct HunterFileLinkedList * prev;
		struct HunterFileLinkedList * next;
	};

	// Iterate over hunters directory
	DIR * dir;
	struct dirent * ent;
	dir = opendir(dataPath((char*) &buffer, "hunters"));
	// TODO: opendir errors

	struct HunterFileLinkedList * hunters = NULL;

	while ((ent = readdir(dir)) != NULL) {
		char * fname = ent->d_name;

		// Skip hidden files, as well as current and parent directories
		if (fname[0] == '.')
			continue;

		// Get the absolute path of the hunter file
		sprintf(
				(char*) &buffer, "%s/%s",
				dataPath((char*) &buffer, "hunters"),
				ent->d_name
			);
		
		struct HunterFileLinkedList * node = (struct HunterFileLinkedList*) calloc(sizeof(struct HunterFileLinkedList), 1);
		strcpy((char*) &node->fpath, (char*) &buffer);

		// Stat hunter
		if(stat((char*) &buffer, &node->attr) != 0){
			// TODO: error
		}

		// Sort hunters by modified date, newest first
		// Just set root node if empty, and continue
		if (hunters == NULL) {
			hunters = node;
			continue;
		}
		
		struct HunterFileLinkedList * iter = hunters;
		struct HunterFileLinkedList * prev = NULL;

		// Find node which was modified earlier, then break search
		while(iter->next && iter->next->attr.st_mtime > node->attr.st_mtime){
			prev = iter;
			iter = iter->next;
		}

		// insert new hunter, sorted
		if(prev == NULL)
			hunters = node;
		else
			prev->next = node;

		node->next = iter;
	}
	closedir(dir);

	struct HunterFileLinkedList * iter = hunters;
	for(; iter; iter = iter->next){
		printf("%s\n", (char*) &iter->fpath);
		printf("Modified time: %s\n", ctime(&iter->attr.st_mtime));
	}

	return 0;
}
