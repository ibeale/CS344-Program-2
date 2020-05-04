// Created by: Isaac Beale
// for OSU CS344 SP'19


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define NUM_ROOMS 10
#define STR_BUF 30

// Room struct that works like a linked list graph
struct Room{
    char name[STR_BUF];
    struct Room* connections[6];
    char roomtype[STR_BUF];
	int num_connections;
	int next_open;
};


// This function creates the directory with the PID of the current process and the correct permissions
char* createDirectory(char path[STR_BUF]){
    pid_t pid = getpid();
    int file_descriptor;
    char path_buffer[STR_BUF];
    memset(path_buffer, '\0', sizeof(path_buffer));
    sprintf(path_buffer, "bealei.rooms.%d/", pid);
    mkdir(path_buffer, 0755);
	strcpy(path, path_buffer);
}


// createRooms takes in an array of allrooms and populates it with
// malloc(). It first shuffles the names, and arbitrarily assigns the start, end and middle rooms
int createRooms(struct Room* allrooms[7]){
    char roomnames[NUM_ROOMS][STR_BUF] = {
        "Washington",
        "Oregon",
        "California",
        "Colorado",
        "Arizona",
        "Utah",
        "Montana",
        "Idaho",
        "New Mexico",
        "Nevada"
    };
    shufflenames(roomnames);
    int i = 0;
	int j = 0;
    for (i = 0; i < 7; i++){
        allrooms[i] = (struct Room*) malloc(sizeof(struct Room));
		// Initialize each connection to NULL
		for(j=0;j<7;j++){
			allrooms[i]->connections[j] = NULL;
		}
		// Initialize other values
		allrooms[i]->num_connections = 0;
		allrooms[i]->next_open = 0;
        strcpy(allrooms[i]->name,roomnames[i]);
		if(i==0){
			strcpy(allrooms[i]->roomtype, "START_ROOM");
		}
		else if(i==6){
			strcpy(allrooms[i]->roomtype, "END_ROOM");
		}
		else{
			strcpy(allrooms[i]->roomtype, "MID_ROOM");
		}
    }
}


// Shuffles an array of strings, used to ranomize the names of the rooms
int shufflenames(char roomnames[NUM_ROOMS][STR_BUF]){
    int i = 0;
    char temp[STR_BUF];
    for (i = 0; i < NUM_ROOMS; i++){
        strcpy(temp, roomnames[i]);
        int idx = rand()%NUM_ROOMS;
        strcpy(roomnames[i], roomnames[idx]);
        strcpy(roomnames[idx], temp);

    }
}


// Checks if two rooms are connected
int areconnected(struct Room* r1, struct Room* r2){
	int i =0;
	for (i;i<6; i++){
		int j = 0;
		for (j; j<6; j++){
			// If the first is connected to the second, and the second is connected to the first we return 1
			if((r1->connections[i]->name == r2->name)&&(r2->connections[j]->name == r1->name)&&r1->connections[i]->name!=NULL){
				return 1;
			}
		}
	}
	// if we get through the above loop, then they are not connected and return 0.
	return 0;
}


// checks to see if we are able to add more connections to the room
int connectionsarenotfull(struct Room* r){
	if(r->num_connections < 6){
		return 1;
	}
	else{
		return 0;
	}
}


// Connect the two given rooms and increment their values accordingly.
void connectrooms(struct Room* r1, struct Room* r2){
	// Make sure the two rooms have capacity for connecting.
	if (connectionsarenotfull(r1) && connectionsarenotfull(r2)){
		r1->connections[r1->next_open] = r2;
		r2->connections[r2->next_open] = r1;
		r1->next_open++;
		r2->next_open++;
		r1->num_connections++;
		r2->num_connections++;
	}
}

// Another shuffle function, this time for a list of ints.
void shufflearr(int arr[7]){
	int i;
	int temp;
	int idx;
	for(i=0;i<7;i++){
		idx = rand()%7;
		temp = arr[i];
		arr[i] = arr[idx];
		arr[idx] = temp;
	}

}


// Use the shufflearr function to randomize the connections
// and for every room, connect to 3-6 other rooms.
void connectallrooms(struct Room* allrooms[7]){
	int num_connections;
	int i =0;
	int j = 0;
	// this array will be shuffled to ensure that we don't try to connect two rooms twice
	// this is done to uphold the rule for number of connections
	int arr[7] = {0,1,2,3,4,5,6};
	for(i=0;i<7;i++){
		// Generates a number [3,6]
		num_connections = (rand()%4) + 3;
		shufflearr(arr);
		// This loop is here to make sure we are connecting a valid room, and not connecting a room to itself
		// I subtract the number of connections the room has from the random number [3-6] so that 
		// if there are, for example, 2 existing connections and we decide to add 3, the final number of connections
		// will be 3 instead of 5.
		for(j=num_connections-allrooms[i]->num_connections;j>0;j--){
			if(i==arr[j]){
				j--;
				if(j < 0){
					j=6;
				}
			}
			// If the rooms aren't connected, then connect them.
			if(!(areconnected(allrooms[i],allrooms[arr[j]]))){
				connectrooms(allrooms[i],allrooms[arr[j]]);
			}
			
		}
	}
}

// This takes in the array of room structs and produces the file to specification.
void makefile(char path[STR_BUF], struct Room* allrooms[7]){
	char name[STR_BUF];
	char content[STR_BUF];
	int i;
	int j;
	int k;
	for(i=0;i<7;i++){
		memset(name, '\0', sizeof(name));
		strcpy(name, path);
		strcat(name, allrooms[i]->name);
		strcat(name, "_room");
		FILE* fp;
		fp = fopen(name, "w");
        fprintf(fp,"ROOM NAME: %s\n", allrooms[i]->name);
		for(j=0;j<allrooms[i]->num_connections;j++){
			fprintf(fp, "CONNECTION %d: %s\n",j+1, allrooms[i]->connections[j]->name);
		}
		fprintf(fp,"ROOM_TYPE: %s\n", allrooms[i]->roomtype);
		fclose(fp);
	}
}


int main()
{
	int i;
	struct Room* allrooms[7];
    srand(time(NULL));

    createRooms(allrooms);
	connectallrooms(allrooms);
	char path[STR_BUF];
	createDirectory(path);
	makefile(path, allrooms);
	for (i = 0; i < 7; i++){
		free(allrooms[i]);
	}

}
