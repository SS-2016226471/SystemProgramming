#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>


#define BACKLOG 1
#define HOSTLEN 256

main(int ac, char *av[]){
	int sock, fd;
	FILE *fpin;
	char request[BUFSIZ];
	if(ac == 1){
		fprintf(stderr,"usage:ws portnum\n");
		exit(1);
	}
	sock = make_server_socket(atoi(av[1]));
	printf(">> Socket ID : %d\n",sock);
	if(sock == -1) exit(2);
	/* main loop hear */
	while(1){
		/* take a call and buffer it */
		printf(">> loop start\n");
		fd = accept(sock,NULL,NULL);
		printf(">> File Discripter : %d\n",fd);
		fpin = fdopen(fd,"r");

		/* read request */
		fgets(request,BUFSIZ,fpin);
		printf("got a call: request = %s", request);
		read_til_crnl(fpin);

		/* do what client asks */
		process_rq(request,fd);
		fclose(fpin);
	}
}
/*
	make_server_socket(portnum)
	make_server_socket(portnum,backlog)
		return a serversocket or -1 if error		*/
int make_server_socket(int portnum){
	return make_server_socket_q(portnum,BACKLOG);
}
int make_server_socket_q(int portnum, int backlog){
	struct sockaddr_in saddr;		/* build our address heare	*/
	struct hostent *hp;			/* this is part of our		*/
	char hostname[HOSTLEN];			/* address			*/
	int sock_id;				/* the socket			*/

	sock_id = socket(PF_INET, SOCK_STREAM,0);
	if(sock_id == -1)
		return -1;
	

	/* build address and bind it to socket */
	bzero((void *)&saddr,sizeof(saddr));	/* clear out struct 	*/
	gethostname(hostname,HOSTLEN);		/* where am I		*/
	hp = gethostbyname(hostname);		/* get info about host	*/
						/* fill in host part	*/
	bcopy((void *)hp->h_addr,(void *)&saddr.sin_addr,hp->h_length);
	saddr.sin_port = htons(portnum);	/* fill in socket port	*/
	saddr.sin_family = AF_INET;		/* fill in addr family	*/
	if(bind(sock_id,(struct sockaddr *)&saddr,sizeof(saddr))!=0)
		return -1;
	
	/* arrange for incoming calls */
	if(listen(sock_id,backlog)!=0)
		return -1;
	return sock_id;
}

/*
	read_til_crnl(FILE *)
	skip over all request info until a CRNL is seen		*/

read_til_crnl(FILE *fp){
	char buf[BUFSIZ];
	printf(">>read_til_crnl start\n");
	while(fgets(buf,BUFSIZ,fp) != NULL && strcmp(buf,"\r\n") != 0)
		;
	printf(">>read_til_crnl close\n");
}

/*
	process_rq(char *rq,int fd)
	do what the request asks for and write reply to fd
	handles request in a new process
	rq is HTTP command: GET /foo/bar.html HTTP/1.0		*/
process_rq(char *rq, int fd){
	char cmd[BUFSIZ], arg[BUFSIZ];
	
	printf(">>process_rq start\n");
	/* create a new process and return if not the child */
	if(fork() != 0)
		return;
	strcpy(arg,"./"); /* precede args with ./ */
	printf(">>arg : %s\n",arg);
	if(sscanf(rq,"%s%s",cmd,arg+2) != 2)
		return;
	printf(">>cmd : %s\n",cmd);
	printf(">>arg : %s\n",arg);

	if(strcmp(cmd,"GET") != 0){
		if(strcmp(cmd,"HEAD") == 0){
			do_head(arg,fd);
		}
		else cannot_do(fd);
	}
	else if(not_exist(arg))
		do_404(arg,fd);
	else if(isadir(arg))
		do_ls(arg,fd);
	else if(ends_in_cgi(arg))
		do_exec(arg,fd);
	else
		do_cat(arg,fd);

	printf(">>process_rq close\n");
}

/*
	the reply header thing: all functions need one
	if content_type is NULL then don't send content type	*/
header(FILE *fp, char *content_type){
	fprintf(fp,"HTTP/1.0 200 OK\r\n");
	if(content_type)
		fprintf(fp,"Content-type: %s\r\n",content_type);
}

/*
	simple functions first:
		connot_do(fd)		unimplemented HTTP command
		and do_404(item,fd)	no such object		*/
cannot_do(int fd){
	FILE *fp = fdopen(fd,"w");
	fprintf(fp,"HTTP/1.0 501 Not Implemented\r\n");
	fprintf(fp,"Content-type: text/plain\r\n");
	fprintf(fp,"\r\n");
	fprintf(fp,"That command is not yet implemented\r\n");
	fclose(fp);
}

do_404(char *item, int fd){
	FILE *fp = fdopen(fd,"w");
	fprintf(fp,"HTTP/1.0 404 Not Found\r\n");
	fprintf(fp,"Content-type: text/plain\r\n");
	fprintf(fp,"\r\n");
	fprintf(fp,"The item you requested: %s\r\nis not found\r\n",item);
	fclose(fp);
}

/*
	the directory listing section
	isadir() uses stat, not_exist() uses stat
	do_ls runs ls, It should not				*/
isadir(char *f){
	struct stat info;
	return(stat(f,&info) != -1 && S_ISDIR(info.st_mode));
}
not_exist(char *f){
	struct stat info;
	return(stat(f,&info) == -1);
}

do_ls(char *dir, int fd){
	FILE *fp;
	fp = fdopen(fd,"w");
	header(fp,"text/plain");
	fprintf(fp,"\r\n");
	fflush(fp);
	
	dup2(fd,1);
	dup2(fd,2);
	close(fd);
	execlp("ls","ls","-l",dir,NULL);
	perror(dir);
	exit(1);
}

/*
	the cgi stuff. function to check extension and
	one to run the program.					*/
char * file_type(char *f){
	char *cp;
	if((cp = strrchr(f,'.')) != NULL)
		return cp+1;
	return "";
}

ends_in_cgi(char *f){
	return(strcmp(file_type(f),"cgi")==0);
}

do_exec(char *prog, int fd){
	FILE *fp;
	fp = fdopen(fd,"w");
	header(fp, NULL);
	fflush(fp);
	dup2(fd,1);
	dup2(fd,2);
	close(fd);
	execl(prog,prog,NULL);
	perror(prog);
}

/*
	do_cat(filename,fd)
	sends back contents after a header			*/
do_cat(char *f,int fd){
	char *extension = file_type(f);
	char *content = "text/plain";
	FILE *fpsock, *fpfile;
	int c;

	if(strcmp(extension,"html") == 0)
		content = "text/html";
	else if(strcmp(extension, "gif") == 0)
		content = "image/gif";
	else if(strcmp(extension,"jpg") == 0)
		content = "image/jpeg";
	else if(strcmp(extension,"jpeg") == 0)
		content = "image/jpeg";

	fpsock = fdopen(fd,"w");
	fpfile = fopen(f,"r");
	if(fpsock != NULL && fpfile != NULL){
		header(fpsock,content);
		fprintf(fpsock, "\r\n");
		while((c = getc(fpfile) ) != EOF)
			putc(c,fpsock);
		fclose(fpfile);
		fclose(fpsock);
	}
	exit(0);
}

do_head(char *f, int fd){
	char *extension = file_type(f);
	char *content = "text/plain";
	FILE *fpsock;
	int c;
	if(strcmp(extension,"html") == 0)
		content = "text/html";
	else if(strcmp(extension,"gif") == 0)
		content = "image/gif";
	else if(strcmp(extension,"jpg") == 0)
		content = "image/jpeg";
	else if(strcmp(extension,"jpeg") == 0)
		content = "image/jpeg";
	fpsock = fdopen(fd,"w");
	if(fpsock != NULL){
		header(fpsock,content);
		fclose(fpsock);
	}
	exit(0);
}
