#ifndef _TERMINAL_SERVER_H
#define _TERMINAL_SERVER_H

/*
Start a terminal on a fork pty that execute a bash --login
input : the input file descriptor to send command to the child process
output : the ouput filedescriptor to display all child output
*/
int start_terminal(int input, int output);

#endif