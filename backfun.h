
#ifndef __FUNCT__
#define __ FUNCT__
#include <stdio.h>
extern char *u;
extern int shell_terminal;
extern pid_t shell_pgid, fpid=0;
extern char fname[1024];
extern struct termios shell_tmodes;
extern struct BgProcess
{
        pid_t pid;
        char *name;
        int status;
        int flag;
};
extern int n=0;
extern struct BgProcess *bg_array;
int jobs();
int fun_killall();
int fg(char **args);
int kjob(char **args);
#endif
