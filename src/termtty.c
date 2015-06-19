//termtty : version 1.0.0

#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <pty.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "termtty.h"


typedef struct Child {
	int fd;
	pid_t pid;
	int started;
	struct Child * next;
	struct termios inputopt;
} Child;

Child * childList=NULL;

/*
Action on SIGCHLD to manage Child termination
*/
void received_sigchld_on_terminal(int sig) {
	int status,pid;
	Child * child;
	//wait for one child process ends
	pid=wait(&status);
	//search the child structure with the given pid
	child=childList;
	while ((child!=NULL) && (child->pid!=pid)) child=child->next;
	if (child!=NULL) {
		if (child->pid==pid) {
			//We end the select loop
			child->started=0;
		}
	}
}	

int winout;

/*
Action on SIGWINCH to manage TTY resizing
*/
void resizeTTY(int sig) {
    struct winsize termsize;
	ioctl(winout,TIOCGWINSZ,&termsize);
	Child * child;
	child=childList;
    while (child!=NULL) {
		ioctl(child->fd,TIOCSWINSZ,&termsize);
		child=child->next;
	}
}


void set_input_opt(Child * child, int input) {
	struct termios ttystate;

	// Backup intial TTY mode of input fd
	tcgetattr(input, &(child->inputopt));

	// Update input mode to remove ECHO and ICANON to allow transmission of character without buffering and echo
	tcgetattr(input, &ttystate);
	ttystate.c_lflag &= ~ICANON;
	ttystate.c_lflag &= ~ECHO;
	ttystate.c_cc[VMIN] = 1;
	tcsetattr(input, TCSANOW, &ttystate);
}

void restore_input_opt(Child * child, int input) {
	tcsetattr(input, TCSANOW, &(child->inputopt));
}


static void listen_sigchld() {
	struct sigaction eventSigChld;
	//Create the sigaction structure to handle SIGCHLD signal
	sigemptyset(&eventSigChld.sa_mask);
	eventSigChld.sa_flags=0;
	eventSigChld.sa_handler= received_sigchld_on_terminal;
	sigaction(SIGCHLD,&eventSigChld, NULL);
}




/*
Remove a child from the child list
*/	
void remove_child(int pid) {
	Child * child=childList;
	Child * previous=NULL;
	while ((child!=NULL) && (child->pid!=pid)) {
		previous=child;
		child=child->next;
	}
	if (child!=NULL) {
		if (child->pid==pid) {
			if (previous==NULL) {
				//first element in list
				childList=child->next;
				free(child);
			} else {
				//an element in the middle
				previous->next=child->next;
				free(child);
			}
		}
	}
}	

Child * init_child() {
	Child * child;
	child=(Child *) malloc(sizeof(Child));
	child->next=NULL;
	child->started=0;
	child->pid=-1;
	child->fd=-1;
	return child;
}


/*
Create a pty and fork the process to execute the bash --login, 
send all shell output on output FD and read input FD to send commands to the shell
*/
int start_terminal(int input, int output) {
	Child * child;
	fd_set readset;
	int result;
	sigset_t mask;
	
	char buffer[100];
	int count;
	int maxfd;

	winout=output;

	//Create the Child structure
	child=init_child();
	
	
	//Define the sigmask  to catch SIGCHLD with pselect
	sigemptyset (&mask);
	sigaddset (&mask, SIGCHLD);

	//Create the fork PTY to manage our shell
	child->pid = forkpty(&(child->fd), NULL, NULL, NULL);
	if (child->pid==-1) {
		return -1;
	} else if (child->pid == 0) {
		//here we are the child process
		char *argv[]={ "/bin/bash","--login", 0};
		execv(argv[0], argv);
		//child process ends here
	} else {
		//Create the sigaction structure to handle SIGWINCH signal
	    struct sigaction eventWindowResize;
		sigemptyset(&eventWindowResize.sa_mask);
		eventWindowResize.sa_flags=0;
		eventWindowResize.sa_handler= resizeTTY;

		//Change TTY size to the actual size of the terminal
		struct winsize termsize;
		ioctl(output,TIOCGWINSZ,&termsize);
		ioctl(child->fd,TIOCSWINSZ,&termsize);

	
		//here we are in the parent process
		child->next=childList;
		childList=child;
	
		//Start handling signal
		listen_sigchld();

		// Update input mode to remove ECHO and ICANON to allow transmission of character without buffering and echo
		set_input_opt(child,input);

	
		//Initialize the flag to signal child has end
		child->started=1;
	
		if (child->fd>input) {
			maxfd=child->fd+1;
		}else{
			maxfd=input+1;
		}
	
		do {
			//initialize file descriptor for select
			FD_ZERO(&readset);
			FD_SET(input, &readset);
			FD_SET(child->fd, &readset);
		
		
			//Block until something to read on child stdout, parent stdin or SIGCHLD
			result = pselect(maxfd, &readset, NULL, NULL, NULL, &mask);
			if (result > 0) {
				if (FD_ISSET(child->fd, &readset)) {
					//Child has write on its stdout
					count = read(child->fd, buffer, sizeof(buffer)-1);
					if (count >= 0) {
						buffer[count] = 0;
						write(output, buffer, count);
					}
				}
				if (FD_ISSET(input, &readset)) {
					//User has write some stuff
					count = read(input, buffer, sizeof(buffer)-1);
					if (count >= 0) {
						buffer[count] = 0;
						write(child->fd, buffer, count);
					}
				}
			}
		} while (child->started);
		remove_child(child->pid);
		//reset input to its original mode and quit
		restore_input_opt(child,input);
	}
    return 1;
}
