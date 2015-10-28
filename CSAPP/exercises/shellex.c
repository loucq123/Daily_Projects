/*
    Author: Lou
    Implementing a shell, first we need a main function to run shell forever.
    And then we need parse the input.
    After parsing, we can evaluate and print the answer.
    Must use the absolute address of the command.     ex. /usr/ls
*/

#include "csapp.h"
#define MAXARGS 16

void eval(char *cmdline);
int parsecmd(char *cmdline, char *argv[]);

int main() {
    char cmdline[MAXLINE];    
    
    while (1) {
        printf("<_>?");
        Fgets(cmdline, MAXLINE, stdin);     // get the input
        if(feof(stdin))
            exit(0);
        eval(cmdline);
    }
    return 0;
}
/*
    In order to run the command, we must get the filename and argv.
    And then fork a new process to run the command.
    If the we run the command at frontground, we need to reap the child process
*/
void eval(char *cmdline) {
    char *argv[MAXARGS];
    char buf[MAXLINE];    // holds modified cmdline
    strcpy(buf, cmdline);
    int bg;                // run background or frontground
    bg = parsecmd(buf, argv);
    if (Fork() == 0) {
        if(execve(argv[0], argv, environ) < 0) {
            printf("Can not execve %s\n", argv[0]);
            exit(0);
        }
    }
    if (bg == 0) {      // run frontground, we need reap the child process
        int status;
        waitpid(-1, &status, 0);
    }
    return;
}

char *skipblanks(char *s) {
    while (s && (*s == ' ' || *s == '\t'))
        ++s;
    return s;
}

/*
    return 1 if running background
    return 0 if running frontground
    return -1 when the input is blank line
*/
int parsecmd(char *cmd, char *argv[]) {
    int argc = 0;   // number of argv
    int bg = 0;
    
    cmd = skipblanks(cmd);
    if (cmd == NULL) {
        argv[0] = NULL;
        return -1;
    }

    while (1) {
        char *delim;
        if ((delim = strchr(cmd, ' ')) == NULL)
            break;
        *delim = '\0';
        argv[argc++] = cmd;
        cmd = skipblanks(delim + 1);
    }
    
    char *delim = strchr(cmd, '\n');
    *delim = '\0';
    argv[argc++] = cmd;
    argv[argc] = NULL;
    if (!strcmp(argv[argc - 1], "&")) {
        bg = 1;
        argv[--argc] = NULL;
    }
    return bg;
}

    
    














      
