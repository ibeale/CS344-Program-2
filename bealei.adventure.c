#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>


#define STR_BUF 30

int getRecentDir(){
	char buf[STR_BUF];
	memset(buf,'\0', STR_BUF);
	strcpy(buf,"./");
	struct dirent *de;

	DIR *dr = opendir(buf);
	struct stat file_info;
	if (dr == NULL){
		printf("Could not open current directory");
		return 0;
	}
	de = readdir(dr);
	while (de != NULL){
		if(strcmp(de->d_name, ".") && strcmp(de->d_name, "..")){
			memset(buf,'\0', STR_BUF);
			strcpy(buf,"./");
			strcat(buf,de->d_name);
			stat(buf, &file_info);
			if(S_ISDIR(file_info.st_mode)){
				printf("%s\n", de->d_name);
			}
		}
		de = readdir(dr);
	}
	closedir(dr);
}

int main(){
	getRecentDir();
}
