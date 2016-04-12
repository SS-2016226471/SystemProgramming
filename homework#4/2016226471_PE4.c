#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>


int main(int ac, char *av[]){
	char* file_name1 = (char *)malloc(BUFSIZ);
	char* file_name2 = (char *)malloc(BUFSIZ);
	if(ac > 1){
		strcpy(file_name1,*(++av));
		strcpy(file_name2,*(++av));
		if(!strcmp(file_name1,file_name2)){
			printf("\'%s\' and \'%s\' area the same file\n",file_name1,file_name2);
			return 0;
		}
		int result = rename(file_name1,file_name2);
		if(result != 0){
			strcat(file_name2,"/");
			strcat(file_name2,file_name1);
			result = link(file_name1,file_name2);
			if(result != 0){
				printf("\'%s\' does not exist\n",file_name1);
				return 0;
			}
			else{
				unlink(file_name1);
			}
		}
	}
	return 0;
}
