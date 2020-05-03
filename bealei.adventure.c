#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


#define STR_BUF 30

struct Room{
    char name[STR_BUF];
    char connections[6][STR_BUF];
    char roomtype[STR_BUF];
	int num_connections;
};

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
				// printf("%s: Last modified at: %d\n", de->d_name, file_info.st_mtime);
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

char* removeFirst(char string[STR_BUF]){
	char new_str[STR_BUF];
	memset(new_str, '\0', STR_BUF);
	int i = 1;
	while(string[i]){
		new_str[i-1] = string[i];
		i++;
	}
	return new_str;
}

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
	while (de != NULL && i<7){
		if(strcmp(de->d_name, ".") && strcmp(de->d_name, "..") && strcmp(de->d_name, "currentTime.txt")){
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



void game(struct Room* allrooms[7]){
	int i = 0;
	int j = 0;
	int k;
	int flag;
	int path[100];
	int path_len = 0;
	size_t bufsize = STR_BUF;
	size_t characters;
	char* buffer;
	buffer = (char*) malloc(STR_BUF);
	for(i=0;i<7;i++){
		if(!(strcmp(allrooms[i]->roomtype, "START_ROOM"))){
			break;
		}
	}
	struct Room* current = allrooms[i];
	while(strcmp(current->roomtype, "END_ROOM")){
		printf("CURRENT LOCATION: %s\n", current->name);
		printf("POSSIBLE CONNECTIONS:");
		j=0;
		while(current->connections[j][0]){
			printf(" %s", current->connections[j]);
			j++;
		}
		printf(".\nWHERE TO? >");
		memset(buffer, '\0', STR_BUF);
		characters = getline(&buffer, &bufsize, stdin);
		buffer[characters-1] = '\0';
		flag = 1;
		for(i=0;i<7;i++){
			if(!(strcmp(allrooms[i]->name, buffer))){
				path[path_len] = i;
				path_len++;
				current = allrooms[i];
				flag = 0;
				printf("\n");
			}
		}
		if(flag){
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
	}
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", path_len);
	for(i=0;i<path_len; i++){
		printf("%s\n", allrooms[path[i]]->name);
	}
	free(buffer);
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

void gettime(){
	char path[STR_BUF];
	getRecentDir(path);
	strcat(path,"/currentTime.txt");
	FILE* fp = fopen(path, "w");
	char buf[STR_BUF];
	time_t cur_time;
	cur_time = time(NULL);
	strftime(buf, STR_BUF, "%I:%M%P, %A, %B %d, %Y", localtime(&cur_time));
	fputs(buf, fp);
	fclose(fp);

}

int main(){
	// int resultInt;
	// int resultInt2;
	// pthread_t threads[2];
	// resultInt = pthread_create( &threads[0], NULL, run, NULL);
	// resultInt = pthread_create( &threads[1], NULL, gettime, NULL);
	gettime();
}
