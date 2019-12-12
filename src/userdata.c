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

void hunterSaveAt(Hunter * hunter, char * fpath){
	/*
		Saves hunter data at a specified path.
	*/

	char buffer[256];
	encodeHunter(hunter, (char*) &buffer);
	
	// Get data length
	char * seek = (char*) &buffer;
	
	// Seek NULL byte, ending the header
	while(*seek++ != NULL);
	
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

int userdataMainSave() {
	Hunter hunters[] = {
		{	.name = "DANIEL",
		.base_stats={.atk = 2, .mov = 3, .def = 6, .max_hp=10},
		.credits = 10528,
		.type = "hunter"
		},
		{	.name = "DAVE",
		.base_stats = {.mov = 3, .atk = 4, .def = 4, .max_hp=10},
		.credits = 1,
		.type = "hunter"
		},
		{	.name = "STAN",
		.base_stats = {.atk = 2, .mov = 1, .def = 8, .max_hp=10},
		.credits = 0xFFFFFFFF,
		.type = "hunter"
		},
		{	.name = "TIM",
		.base_stats = {.atk = 9, .mov = 1, .def = 1, .max_hp=10},
		.credits = 12,
		.type = "hunter"
		}
	};

	for(int i=0; i < 4; i++){
		hunterSave(&hunters[i]);
	}
}

int usermain(){
	userdataMainSave();
	char buffer[256];

	// Linked list struct type for Hunter reading
	struct HunterFileLinkedList {
		char fpath[128];
		struct stat attr;
		
		Hunter hunter;
		
		// double-linked list
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

		// If root node is empty, set it to the current node and continue to next node
		if (hunters == NULL) {
			hunters = node;
			continue;
		}

		// If the new node's modify time is greatest, make it the new root and continue
		time_t node_time = node->attr.st_mtim.tv_sec;

		if (node_time >= hunters->attr.st_mtim.tv_sec) {
			node->next = hunters;
			hunters = node;
			continue;
		}

		// Search for sorted insert point for node
		struct HunterFileLinkedList * iter = hunters;

		while (iter->next != NULL) {
			time_t iter_time = iter->attr.st_mtim.tv_sec;
			time_t next_time = iter->next->attr.st_mtim.tv_sec;

			if(iter_time >= node_time && node_time >= next_time)
				break;

			iter = iter->next;
		}

		node->next = iter->next;
		iter->next = node;
	}
	closedir(dir);

	struct HunterFileLinkedList * iter = hunters;
	for(; iter; iter = iter->next){
		// Load hunter file contents
		FILE * fd = fopen((char*) &iter->fpath, "rb");
		fread((char*) &buffer, sizeof(buffer), 1, fd);
		fclose(fd);

		// Decode hunter data and print
		decodeHunter(&iter->hunter, (char*) &buffer);
		printHunter(&iter->hunter);
		printf("\n");
	}

	return 0;
}
