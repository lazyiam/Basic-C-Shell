#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<pwd.h>
#include<sys/wait.h>
#include "builtin.h"
#include "proc.h"
#include "pipe.h"
char *u;
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

void sigh(int signum)
{
        int status;
        pid_t wpid;
        wpid=waitpid(-1,&status,WNOHANG);
        if(wpid>0)
        {
                if(WIFEXITED(status)==0)
                {
                        printf("\n%s with pid %d exited normally\n",u,wpid);
                }
                if(WIFSIGNALED(status)==0)
                {
                        printf("\n%s with pid %d exited normally\n",u,wpid);
                }
        }
}


char path1[1025];

char *internal_name[] = {"cd","echo","exit","pwd","pinfo"};
int (*internal_fun[]) (char **) ={&fun_cd,&fun_echo,&fun_exit,&fun_pwd,&fun_pinfo};
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

char *lsh_read_line(void)
{
        char *line = NULL;
        ssize_t bufsize = 0;
        getline(&line, &bufsize, stdin);
        return line;
}
void redirect_out(char **args,int i)
{
        puts("In redirection1");
        pid_t  wpid,pid;
        int status;
        pid=fork();
        if(pid==0)
        {
                int fd1 = creat(args[i-1], 0644);
                dup2(fd1,1);
                close(fd1);
                execvp(args[0],args);
        }
        else
        {
                do 
                {
                        wpid = waitpid(pid, &status, WUNTRACED);

                } 
                while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        return ;
}
char **tokens2;
int position2=0;
void pipe_exec(char ** tokens, int in, int out)
{
     //   printf("%s %s\n",tokens[0], tokens[1]);
      //  puts("In pipe_exec");
        
        if (in!=0)
        {
                dup2(in,0);
                close(in);
        }
        if (out!=1)
        {
                dup2(out,1);
                close(out);
        }
        execvp(tokens[0],tokens);

}
int pipe_ini(int count,char** args, int com_count)
{
        char * token;
        char ** tokens=malloc(512*sizeof(char*));
        char ** tokens3=malloc(512*sizeof(char*));

        int k,position=0;
        //printf("in pipe fun->%s\n",copy_arr[0]);
        //printf("a\n");
        //printf("%d\n",position2);
        int i;
        i=com_count-1;
        
       // printf("test %s\n",copy_arr[i]);
        token = strtok(copy_arr[i],"|");
        while (token != NULL) 
        {
               // printf("token is %s\n",token);
                
                tokens[position] = token;
                position++;

                token = strtok(NULL,"|");

        }

        tokens[position]=NULL;
        //printf("position is %d\n",position);
        int position3=0;
        int in=0,fd[2];
        //printf("%s\n",tokens[0]);
        for (i=0;i<=position-2;i++)
        {
                position3=0;
                token=strtok(tokens[i]," ");

                while(token!=NULL)
                {
                   //     printf("token recurse is %s\n",token);
                        tokens3[position3]=token;
                        position3++;
                        token=strtok(NULL," ");
                }
                tokens3[position3]=NULL;
                //printf("position3 is %d\n",position3);

                //printf ("after space sep->%s\n",tokens3[0]);
                pid_t pid;

                pipe(fd);
                pid=fork();
                if(pid==0)
                {
                        close(fd[0]);
                        //printf("%d %d\n",fd[0],fd[1]);
                        pipe_exec(tokens3, in, fd[1]);
                }
                else
                {
                        close(fd[1]);
                        close(in);
                        in=fd[0];
                }
        }
        i=position-1;
        position3=0;
        token=strtok(tokens[i]," ");
        while(token!=NULL)
        {
                tokens3[position3]=token;
                position3++;
                token=strtok(NULL," ");
        }
        tokens3[position3]=NULL;

        pipe_exec(tokens3,in,STDOUT_FILENO);
        return 1;


}




int launch_proc(char **args, int com_count)
{
        //puts("In launch");
        int flag=0,i=0;
        int temp;
        for (i=0;;i++)
        {
                if (args[i]==NULL)
                        break;
        }
        //puts("After NULL check");
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
                        //        puts("After 1st if check");
                        }

                        if (i>2 && strcmp(args[i-2],">") == 0)
                        {
                           //     printf("Hello\n");
                                args[i-2]=NULL;
                                printf("%s\n",args[i-1]);
                                redirect_out(args,i);
                                flag2=1;

                        }
                }

                pid_t pid, wpid;
                int status;
                if (flag2==0)
                {

                        pid = fork();
                        if (pid == 0) 
                        {

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
                                        do 
                                        {
                                                wpid = waitpid(pid, &status, WUNTRACED);
                                        } 
                                        while (!WIFEXITED(status) && !WIFSIGNALED(status));
                                }
                                else
                                {
                                        flag=0;
                                        signal(SIGCHLD,sigh);
                                }

                        }
                }}

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

        for (i = 0; i < 5; i++) 
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
        char *line;	int status=1;
        getcwd(path1,sizeof(path1));
        while(status>0)
        {
                show_prompt();
                line = lsh_read_line();
                status = splitargs(line);

        }
        return 0;
}
