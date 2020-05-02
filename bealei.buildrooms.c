#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#define NUM_ROOMS 10
#define STR_BUF 30

struct Room{
    char name[STR_BUF];
    struct Room* connections[6];
    char roomtype[STR_BUF];
	int num_connections;
	int next_open;
};

char* createDirectory(void){
    pid_t pid = getpid();
    int file_descriptor;
    char path_buffer[STR_BUF];
    memset(path_buffer, '\0', sizeof(path_buffer));
    sprintf(path_buffer, "bealei.rooms.%d/", pid);
    mkdir(path_buffer, 0755);
	return path_buffer;
}


int createRooms(struct Room* allrooms[7]){
    char roomnames[NUM_ROOMS][STR_BUF] = {
        "one",
        "two",
        "three",
        "four",
        "five",
        "six",
        "seven",
        "eight",
        "nine",
        "ten"
    };
    shufflenames(roomnames);
    int i = 0;
	int j = 0;
    for (i = 0; i < 7; i++){
        allrooms[i] = (struct Room*) malloc(sizeof(struct Room));
		for(j=0;j<7;j++){
			allrooms[i]->connections[j] = NULL;
		}
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

int areconnected(struct Room* r1, struct Room* r2){
	int i =0;
	for (i;i<6; i++){
		int j = 0;
		for (j; j<6; j++){
			if((r1->connections[i]->name == r2->name)&&(r2->connections[j]->name == r1->name)&&r1->connections[i]->name!=NULL){
				return 1;
			}
		}
	}
	return 0;
}

int connectionsarenotfull(struct Room* r){
	if(r->num_connections < 6){
		return 1;
	}
	else{
		return 0;
	}
}

void connectrooms(struct Room* r1, struct Room* r2){
	if (connectionsarenotfull(r1) && connectionsarenotfull(r2)){
		r1->connections[r1->next_open] = r2;
		r2->connections[r2->next_open] = r1;
		r1->next_open++;
		r2->next_open++;
		r1->num_connections++;
		r2->num_connections++;
	}
}


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

void connectallrooms(struct Room* allrooms[7]){
	int num_connections;
	int i =0;
	int j = 0;
	int arr[7] = {0,1,2,3,4,5,6};
	for(i=0;i<7;i++){
		num_connections = (rand()%3) + 3;
		shufflearr(arr);
		for(j=num_connections-allrooms[i]->num_connections;j>0;j--){
			if(i==arr[j]){
				j--;
				if(j < 0){
					j=6;
				}
			}
			if(!(areconnected(allrooms[i],allrooms[arr[j]]))){
				connectrooms(allrooms[i],allrooms[arr[j]]);
			}
			
		}
	}
}


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
	//

	// FILE *fp;
	// fp = fopen("")
}


int main()
{
	int i;
	struct Room* allrooms[7];
    srand(time(NULL));

    createRooms(allrooms);
	connectallrooms(allrooms);
	char path[STR_BUF];
	strcpy(path,createDirectory());
	makefile(path, allrooms);
	for (i = 0; i < 7; i++){
		free(allrooms[i]);
	}

}
