#include <stdio.h>
#include <unistd.h>

#define oops(m,x) { perror(m); exit(x); }

main(int ac, char **av){
	int thepipe[2],thepipe2[2],newfd,pid;
	char buf[1000];
	if(ac != 4){
		fprintf(stderr,"usage: pipe cmd1 cmd2 cmd3\n");
		exit(1);
	}
	
	if(pipe(thepipe) == -1){
		oops("Cannot get a pipe",1);
	}
	if(pipe(thepipe2) == -1){
		oops("Cannot get a pipe2",1);	
	}
	if((pid = fork()) == -1){
		oops("Cannot fork",2);
	}
	if(pid > 0){
		if(dup2(thepipe2[0],0) == -1){
			oops("could not redirect stdin",3);
		}
		close(thepipe[0]);
		close(thepipe[1]);
		close(thepipe2[0]);
		close(thepipe2[1]);
		execlp(av[3],av[3],NULL);
	}
	else if(pid == 0){
		if((pid = fork()) == -1){
			oops("Cannot fork",2);
                }
		if(pid>0){
			if(dup2(thepipe[0],0)==-1){
				oops("could not redirect stdin",3);
			}
			if(dup2(thepipe2[1],1)==-1){
				oops("could not redirect stdout",4);
			}
			close(thepipe[0]);
			close(thepipe[1]);
			close(thepipe2[0]);
			close(thepipe2[1]);
			execlp(av[2],av[2],NULL);
		}
		else if(pid == 0){
			if(dup2(thepipe[1],1) == -1){
				oops("could not redirect stdout",4);
			}
			close(thepipe[0]);
			close(thepipe[1]);
			close(thepipe2[0]);
			close(thepipe2[1]);
			execlp(av[1],av[1],NULL);
		}
	}
}
