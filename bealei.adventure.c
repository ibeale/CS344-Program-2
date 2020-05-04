// Created by: Isaac Beale
// for OSU CS344 SP'19

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


#define STR_BUF 30


// Similar struct to the one used in buildrooms, except the connections are just strings now instead of pointers
// because it was too annoying to try and recreate the original struct.
struct Room{
    char name[STR_BUF];
    char connections[6][STR_BUF];
    char roomtype[STR_BUF];
	int num_connections;
};


// Finds the most recent directory (not including . and ..) and returns the path to it.
// Adapted from https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
void getRecentDir(char path[STR_BUF]){
	char buf[STR_BUF];
	memset(buf,'\0', STR_BUF);
	memset(path, '\0', STR_BUF);
	strcpy(buf,"./");
	struct dirent *de;

	DIR *dr = opendir(buf);
	struct stat file_info;
	if (dr == NULL){
		printf("Could not open current directory");
		return;
	}
	de = readdir(dr);
	time_t recent = 0;
	while (de != NULL){
		if(strcmp(de->d_name, ".") && strcmp(de->d_name, "..")){
			memset(buf,'\0', STR_BUF);
			strcpy(buf,"./");
			strcat(buf,de->d_name);
			stat(buf, &file_info);
			if(S_ISDIR(file_info.st_mode)){
				if(file_info.st_mtime > recent){
					recent = file_info.st_mtime;
					memset(path, '\0', STR_BUF);
					strcpy(path, buf);
				}
			}
		}
		de = readdir(dr);
	}
	closedir(dr);

}


// Helper function to remove the leading white character on a string because strtok is difficult.
char* removeFirst(char string[STR_BUF]){
	char new_str[STR_BUF];
	memset(new_str, '\0', STR_BUF);
	int i = 1;
	while(string[i]){
		new_str[i-1] = string[i];
		i++;
	}
	memset(string, '\0', STR_BUF);
	strcpy(string, new_str);
}


// Iterate through each file inside of a directory
void iterateFiles(char* path, struct Room* allrooms[7]){
	char buf[STR_BUF];
	DIR *dr = opendir(path);
	struct dirent *de;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	char content[STR_BUF];
	FILE* fp;
	if (dr == NULL){
		printf("Could not open specified directory\n");
		return;
	}
	de = readdir(dr);
	char delim[] = ":\n";
	int i = 0;

	// Iterate through the files that are not named ".", "..", or "currentTime.txt"
	while (de != NULL && i<7){
		if(strcmp(de->d_name, ".") && strcmp(de->d_name, "..") && strcmp(de->d_name, "currentTime.txt")){
			// Create a new room struct and initialize its variables.
			allrooms[i] = (struct Room*) malloc(sizeof(struct Room));
			int j = 0;
			for(j; j < 6; j++){
				memset(allrooms[i]->connections[j], '\0', STR_BUF);
			}
			memset(buf,'\0', STR_BUF);
			strcpy(buf,path);
			strcat(buf, "/");
			strcat(buf, de->d_name);
			fp = fopen(buf, "r");
			if (fp == NULL){
				printf("Couldnt open file %s\n", buf);
				return;
			}
			int k = 0;
			// adapted from https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
			while ((read = getline(&line, &len, fp)) != -1){
				char *ptr = strtok(line, delim);
				
				while (ptr != NULL){
					if(!(strcmp(ptr, "ROOM NAME"))){
						ptr = strtok(NULL, delim);
						memset(allrooms[i]->name, '\0', STR_BUF);
						strcpy(allrooms[i]->name, removeFirst(ptr));
						ptr = strtok(NULL, delim);	
					}
					else if(!(strcmp(ptr, "ROOM_TYPE"))){
						ptr = strtok(NULL, delim);
						memset(allrooms[i]->roomtype, '\0', STR_BUF);
						strcpy(allrooms[i]->roomtype, removeFirst(ptr));
						ptr = strtok(NULL, delim);	
					}
					else{
						ptr = strtok(NULL, delim);
						strcpy(allrooms[i]->connections[k], removeFirst(ptr));
						k++;
						ptr = strtok(NULL, delim);
					}
				}
			}
			i++;
			fclose(fp);
		}
		de = readdir(dr);
		
	}
	if(line){
		free(line);
	}

	closedir(dr);
}

// Will use it's own thread to print the local time to a new file named currentTime.txt
void* gettime(pthread_mutex_t* myMutex){
	// Lock the resources so that the main process doesn't run
	pthread_mutex_lock(myMutex);
	char path[STR_BUF];
	getRecentDir(path);
	strcat(path,"/currentTime.txt");
	FILE* fp = fopen(path, "w");
	char buf[STR_BUF];
	time_t cur_time;
	cur_time = time(NULL);
	strftime(buf, STR_BUF, "%-I:%M%P, %A, %B %d, %Y", localtime(&cur_time));
	fputs(buf, fp);
	fclose(fp);
	// Release the resources to allow the main program to run.
	pthread_mutex_unlock(myMutex);

}


// Prints the first line fo currentTime.txt
void printFromTimeFile(){
	char path[STR_BUF];
	char content[STR_BUF];
	getRecentDir(path);
	strcat(path,"/currentTime.txt");
	FILE* fp = fopen(path, "r");
	fgets(content, STR_BUF, fp);
	printf("\n%s\n", content);
}


void game(struct Room* allrooms[7]){
	pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&myMutex);
	pthread_t thread;
	pthread_create( &thread, NULL, (void*) gettime, &myMutex);
	int i = 0;
	int j = 0;
	int k;
	int flag = 0;
	// path will hold the integer locations in the all rooms array of the visited
	int path[100];
	int path_len = 0;
	size_t bufsize = STR_BUF;
	size_t characters;
	char* buffer;
	buffer = (char*) malloc(STR_BUF);

	//Find the location of the start room
	for(i=0;i<7;i++){
		if(!(strcmp(allrooms[i]->roomtype, "START_ROOM"))){
			break;
		}
	}
	struct Room* current = allrooms[i];
	// While we're not in the end room, repeat
	while(strcmp(current->roomtype, "END_ROOM")){
		// Flag = 2 if the time function is called. This is just to format the output correctly.
		if(flag != 2){
			printf("CURRENT LOCATION: %s\n", current->name);
			printf("POSSIBLE CONNECTIONS:");
			j=0;
			while(current->connections[j][0] && j<6){
				// If conditions are for output formatting.
				if(j==0){
					printf(" %s", current->connections[j]);
					j++;
				}
				else{
					printf(", %s", current->connections[j]);
					j++;
				}
				
			}
		}
		printf("\nWHERE TO? >");
		memset(buffer, '\0', STR_BUF);
		characters = getline(&buffer, &bufsize, stdin);
		buffer[characters-1] = '\0';
		flag = 0;
		for(i=0;i<6;i++){
			// check if the user input is connected to the current room
			if(!(strcmp(current->connections[i], buffer))){
				for(j=0;j<7;j++){
					// Find the location in the array of the user inputted room
					if(!(strcmp(allrooms[j]->name, buffer))){
						path[path_len] = j;
						path_len++;
						current = allrooms[j];
						flag = 1;
						printf("\n");
						break;
					}
				}
			}
			else if(!(strcmp("time", buffer))){
				pthread_mutex_unlock(&myMutex);
				pthread_join(thread, NULL);
				pthread_mutex_lock(&myMutex);
				pthread_create( &thread, NULL, (void*) gettime, &myMutex);
				printFromTimeFile();
				flag=2;
				break;
				
			}
		}
		// If neither of the events above triggered, output the error message and try again.
		if(flag == 0){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
	}
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", path_len);
	for(i=0;i<path_len; i++){
		printf("%s\n", allrooms[path[i]]->name);
	}
	free(buffer);
	// Release the mutex so that we can join the dormant thread
	pthread_mutex_unlock(&myMutex);
	// Join dormant thread to free memory
	pthread_join(thread, NULL);
}

void* run(){
	struct Room* allrooms[7];
	char path[STR_BUF];
	memset(path, '\0', STR_BUF);
	getRecentDir(path);
	iterateFiles(path, allrooms);
	int i = 0;
	int j = 0;
	game(allrooms);
	for(i=0;i<7;i++){
		free(allrooms[i]);
	}
	return NULL;
}

int main(){
	run();
	exit(0);
}
