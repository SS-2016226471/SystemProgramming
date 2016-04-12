#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

int dostat(char *);

int main(int ac,char *av[]){
	// file_name1, file_name2 is receive arguments string.
	char* file_name1 = (char *)malloc(BUFSIZ);
	char* file_name2 = (char *)malloc(BUFSIZ);
	// If the number of arguments is greater than 1
	if(ac > 1){ 
		strcpy(file_name1,*(++av)); // Copy the first argument to file_name1
		strcpy(file_name2,*(++av)); // Copy the second argument to file_name2
		// If the same file_name1 and file_name2
		if(!strcmp(file_name1,file_name2)){
			// Print error and exit.
			printf("\'%s\' and \'%s\' area the same file\n",file_name1,file_name2);
			return 0;
		}
		// Change the filename and return value stored on the result variable.
		int result = rename(file_name1,file_name2);
		// rename function is error	
		if(result != 0){
			if(dostat(file_name2)){ // If file_name2 is directory
				strcat(file_name2,"/"); // make new path
				strcat(file_name2,file_name1);
				result = link(file_name1,file_name2); // move file_name1 to file_name2
				if(result != 0){ // link function is error
					printf("\'%s\' does not exits of directory\n",file_name1); // file_name1 does not exist of directory.
				}else{ // link function is not error
					unlink(file_name1); // remove file_name1
				}
			}
			else { // If file_name2 is file
				printf("\'%s\' does not exits of directory\n",file_name1); // file_name1 does not exist of directory.
			}
		}
	}
	return 0; // Program exit.
}

// file_name2 is checked whether directory.
int dostat(char *filename){
	struct stat info; 
	if(stat(filename,&info) == -1){ // Not read file info. 
		perror(filename);
	}
	else {  // read file info
		if(S_ISDIR(info.st_mode)) { // file is a directory?
			return 1;
		}
		else return 0;
	}
}
	
