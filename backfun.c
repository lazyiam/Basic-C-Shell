#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<pwd.h>
#include<sys/wait.h>
#include <signal.h>
#include <termios.h>
char *u;
int shell_terminal;
pid_t shell_pgid, fpid=0;
char fname[1024];
struct termios shell_tmodes;
struct BgProcess
{
        pid_t pid;
        char *name;
        int status;
        int flag;
};
int n=0;
struct BgProcess *bg_array;

int jobs()
{
        int i, k;
        k = 0;
        for(i=0;i<n;i++)
        {
                if(bg_array[i].pid > 0)
                {
                        k++;
                        printf("[%d] %s [%d]\n", k, bg_array[i].name, bg_array[i].pid);
                }
        }
        if(k==0)
                printf("No jobs running!\n");
        return 1;
}
int fun_killall()
{
        int i,c=0;
        for(i=0;i<n;i++)
        {
                if(bg_array[i].pid > 0)
                {
                        c++;
                        kill(bg_array[i].pid, SIGKILL);
                        bg_array[i].pid=-1;
                }
        }
        printf ("%d processes killed\n",c);
        return 1;
}
int fg(char **args)
{
        if(args[1] == NULL)
        {
                fprintf(stderr, "Warning: Expected 1 argument\n");
                return 1;
        }
        int jobnum = atoi(args[1]);
        pid_t pid;
        int i,k;
        k = 0;
        for(i=0;i<n;i++)
        {
                if(bg_array[i].pid > 0)
                {
                        k++;
                        if(k == jobnum)
                        {
                                pid = bg_array[i].pid;
                                break;
                        }
                }
        }
        if(k==0 || k != jobnum)
        {
                fprintf(stderr, "No such job\n");
                return 1;
        }

        int status;
        pid_t groupId = getpgid(pid);
        bg_array[i].pid=-1;
        tcsetpgrp(shell_terminal, groupId);
        kill(pid, SIGCONT);
        do
        {
                waitpid(pid, &status, WUNTRACED | WCONTINUED);

                if(WIFSTOPPED(status))
                {
                        bg_array[i].pid = pid;
                        printf("\n%s stopped by signal %d\n", fname, WSTOPSIG(status));
                        break;
                }
        }while(!WIFEXITED(status) && !WIFSIGNALED(status)); 

        tcsetpgrp (shell_terminal, shell_pgid);
        tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);

        return 1;
}
int kjob(char **args)
{
        if(args[1] == NULL || args[2] == NULL)
        {
                fprintf(stderr, "Warning: Expected 2 arguments\n");
                return 1;
        }
        int jobnum = atoi(args[1]);
        int signalnum = atoi(args[2]);
        int i,k;
        int t;
        pid_t temp;
        k=0;
        for(i=0;i<n;i++)
        {
                if(bg_array[i].pid > 0 )
                {
                        k++;
                        if(k == jobnum)
                        {
                                if(signalnum == 9)
                                {       temp=bg_array[i].pid;
                                        bg_array[i].pid = -1;
                                }
                                break;
                        }
                }
        }
        if(k != jobnum || jobnum == 0 || k==0)
        {
                fprintf(stderr, "No such job\n");
                return 1;
        }
        kill(temp, signalnum);
        return 1;
}
