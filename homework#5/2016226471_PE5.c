#include <stdio.h>
#include <signal.h>
#include <termios.h>

main(){	
	void f(int); // The interrupt handling function
	int i;	
	signal(SIGINT, f);	// Interrupt signal & handler set
	set_cr_noecho_mode();
	for (i=0;i<5;i++){
		printf("hello\n");
		sleep(1);
	}
}
void f(int signum){ // interrupt handling function
	char input;
	printf("Interrupted! OK to quit (y/n)? ");  
	printf("%c\n",	input = getchar()); // TTY state is no echo mode, So print input character
	fflush(stdout); // buffer clear
	if(input == 'y') exit(1); // Input character is 'y' then process exit
}
set_cr_noecho_mode(){ // TTY state set no echo mode
	struct termios ttystate;
	tcgetattr(0,&ttystate);	// read TTY state
	ttystate.c_lflag &= ~ICANON; // TTY state set no buffer
	ttystate.c_lflag &= ~ECHO; // TTY state set no print echo
	ttystate.c_cc[VMIN] = 1; // TTY state get 1 char at a time
	tcsetattr(0, TCSANOW, &ttystate); // TTY state install settings
}

