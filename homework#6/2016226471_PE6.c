#include <stdio.h>
#include<signal.h>

#define MAXARGS 20
#define ARGLEN 100
main(){
	void e(int);
	char *arglist[MAXARGS+1];
	int numargs;
	char argbuf[ARGLEN];
	char *makestring();
	numargs = 0;
	while(numargs < MAXARGS){
		signal(SIGINT,e); // Interrupt Signal , push  Ctrl + C , Program exit
		printf("Arg[%d]? ",numargs);
		if(fgets(argbuf,ARGLEN,stdin)&& *argbuf != '\n')
			arglist[numargs++] = makestring(argbuf);
		else{
			if(numargs > 0){
				arglist[numargs]=NULL;
				execute(arglist);
				numargs = 0;
			}
		}
	}
	return 0;
}

execute(char *arglist[]){
	int pid,exitstatus;
	void f(int);
	signal(SIGINT,f); // Interrupt signal, stop for execute process
	pid = fork();
	switch(pid){
		case -1: 
			perror("fork failed");
			exit(1);
		case 0 :
			execvp(arglist[0],arglist);
			perror("execvp failed");
			exit(1);
		default:
			while(wait(&exitstatus) != pid);
	}
}
void f(int signum){ // used to execute function in the Ctrl+C interrupt handling
	printf("\n");
}
void e(int signum){ // used to main in the Ctrl+C interrupt handling
	printf("\n");
	exit(1);
}
char *makestring(char *buf){
	char *cp,*malloc();
	buf[strlen(buf)-1] = '\0';
	cp = malloc(strlen(buf)+1);
	if(cp==NULL){
		fprintf(stderr,"no memory\n");
		exit(1);
	}
	strcpy(cp,buf);
	return cp;
}

