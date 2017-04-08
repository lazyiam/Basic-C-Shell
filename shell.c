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
#include "builtin.h"
#include "proc.h"
#include "pipe.h"
#include "redirect.h"
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
void alloc_bg_array()
{
        bg_array=malloc(sizeof(struct BgProcess)*1024);
}
int start=0,qid[1024],on_off[1024];
char qname[1024][1024];

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

int findstr(char str1[],char str2[],int a,int b)
{
        if (a==0)
                return 1;
        if (b==0)
                return 0;
        if (str1[a-1]==str2[b-1])
                return findstr(str1,str2,a-1,b-1);
        else
                return findstr(str1,str2,a,b-1);
}

void bg_process(pid_t pid, char *process, int status)
{
        bg_array[n].name = malloc(sizeof(char) * 1024);
        bg_array[n].pid = pid;
        strcpy(bg_array[n].name, process);
        bg_array[n].status = status;
        bg_array[n].flag = 1;
        n++;
        if(n == 1024)
        {
                bg_array = realloc(bg_array, 1024);

                if(!bg_array)
                {
                        fprintf(stderr, "Memory Allocation Error!\n");
                        exit(EXIT_FAILURE);
                }
        }
}
void finished_BgProcess(int signum)
{
        int i;
        for(i=0;i<n;i++)
        {
                if(bg_array[i].pid > 0)
                {
                }
        }
}

void sigh(int signum)
{
        int status,i;
        pid_t wpid;
        wpid=waitpid(-1,&status,WNOHANG);
        if (wpid>0)
        {
                if (WIFSIGNALED(status)==0)
                {

                        for(i=0;i<n;i++)
                        {
                                if (wpid==bg_array[i].pid)
                                {
                                        break;
                                }
                        }
                        bg_array[i].pid=-1;
                        fprintf(stderr,"\n%s exited normally\n",bg_array[i].name);
                }

        }
        return ;
}


char path1[1025];

char *internal_name[] = {"cd","echo","quit","pwd","pinfo","jobs","killallbg","fg","kjob"};
int (*internal_fun[]) (char **) ={&fun_cd,&fun_echo,&fun_exit,&fun_pwd,&fun_pinfo,&jobs,&fun_killall,fg,kjob};
char **copy_arr;
void show_prompt()
{
        struct passwd *var1;
        var1=getpwuid(getuid());
        char hostname[1024];
        gethostname(hostname,1023);
        char cwd[1024];
        getcwd(cwd,sizeof(cwd));
        if(strcmp(cwd,path1)==0)
        {
                printf("<%s@%s:~>",var1->pw_name,hostname);
        }

        else if (findstr(path1,cwd,strlen(path1),strlen(cwd)))
        {
                printf ("<%s@%s:~",var1->pw_name,hostname);
                int i;
                int m,n;
                m=strlen(path1);n=strlen(cwd);
                for (i=m;i<n;i++)
                {
                        printf("%c",cwd[i]);
                }
                printf(">");
        }
        else
                printf ("<%s@%s:%s>",var1->pw_name,hostname,cwd);
        char *command;
        char **tokens;
}
void sigintHandler(int sig_num)
{
        signal(SIGINT, sigintHandler);
        printf("\n");
        fflush(stdout);
}


char *lsh_read_line(void)
{
        char *line = NULL;
        ssize_t bufsize = 0;
        signal(SIGINT, sigintHandler); 
        getline(&line, &bufsize, stdin);
        return line;
}
char **tokens2;
int position2=0;

int launch_proc(char **args, int com_count)
{
        int flag=0,i=0;
        int temp;
        for (i=0;;i++)
        {
                if (args[i]==NULL)
                        break;
        }
        int count=0,j=0;
        for (j=0;j<i;j++)
        {
                if (strcmp(args[j],"|")==0)
                {
                        count++;
                }
        }
        if (count>0)
        {
                pipeHandler(args);
        }
        else{
                int flag2=0;
                if (args[i-1]!=NULL)
                {
                        if( strcmp(args[i-1],"&")==0)
                        {
                                u=args[0];
                                args[i-1]='\0';
                                flag=1;
                        }

                        if (i>2 && (strcmp(args[i-2],">") == 0 || strcmp(args[i-2], "<")==0 || strcmp(args[i-2],">>")==0))
                        {
                                redirect(args);
                                flag2=1;

                        }
                }

                pid_t pid, wpid;
                int status=0;
                if (flag2==0)
                {

                        pid = fork();
                        if (pid == 0) 
                        {
                                signal(SIGINT,SIG_DFL);

                                if (execvp(args[0], args) == -1) 
                                {
                                        perror("error");
                                }	
                                exit(EXIT_FAILURE);
                        } 
                        else if (pid < 0) 
                        {
                                fprintf(stderr,"couldn't create the child process");
                        } 
                        else 
                        {
                                if (flag==0)
                                {
                                        /*do 
                                          {
                                          wpid = waitpid(pid, &status, WUNTRACED);
                                          } 
                                          while (!WIFEXITED(status) && !WIFSIGNALED(status));*/
                                        waitpid(pid,&status, WUNTRACED);
                                }
                                else
                                {
                                        flag=0;
                                        bg_process(pid,args[0],status);
                                        signal(SIGCHLD,sigh);

                                }

                        }
                }
        }


        return 1;
}

int exec_comm(char **args, int com_count);
int splitargs(char *line)
{
        int bufsize = 2000, position = 0,status;
        position2=0;
        char **tokens = malloc(bufsize * sizeof(char*));
        tokens2 = malloc(512 * sizeof(char*));
        char *token;

        char * token2;
        token2 = strtok(line,";");
        while (token2 != NULL) 
        {
                tokens2[position2] = token2;
                position2++;

                token2 = strtok(NULL,";");

        }
        tokens2[position2]=NULL;
        int i=0;
        copy_arr = malloc(sizeof(char**) * position2);
        for (i=0;i < position2; i++) 
        {
                char* pCurrent = tokens2[i];
                size_t currentLength = strlen(pCurrent);
                copy_arr[i] = malloc(currentLength + 1);
                strcpy(copy_arr[i], tokens2[i]);
        }
        int com_count=1;
        for (i=0;i<position2;i++)
        {
                token = strtok(tokens2[i]," ;\t\n\"");
                while (token != NULL) 
                {
                        tokens[position] = token;
                        position++;

                        token = strtok(NULL," ;\t\n\"");

                }
                tokens[position]=NULL;
                position=0;
                status=exec_comm(tokens,com_count);
                com_count++;
        }
        return status;
}
int exec_comm(char **args, int com_count)
{
        int i;

        if (args[0] == NULL) 
        {
                return 1;
        }

        for (i = 0; i <9; i++) 
        {
                if (strcmp(args[0], internal_name[i]) == 0) 
                {
                        return (*internal_fun[i])(args);
                }
        }

        return launch_proc(args, com_count);
}

int main()
{
        char *line;	int status=1,i,j;
        /*for (i=0;i<1000;i++)
          {
          bg_process(pid,args[0],status);
          }*/
        alloc_bg_array();

        getcwd(path1,sizeof(path1));
        while(status>0)
        {
                show_prompt();
                line = lsh_read_line();
                signal(SIGINT,SIG_IGN);
                status = splitargs(line);


        }
        return 0;
}
