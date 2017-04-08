#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<pwd.h>
#include<sys/wait.h>
void redirect(char **args)
{
        pid_t  wpid,pid;
        int status;
        int fd0,fd1,i,in=0,out=0;
        char input[64],output[64];
        pid=fork();
        if(pid==0)
        {
                int in=0;
                int out=0;
                int i=0;
                for(i=0;args[i]!='\0';i++)
                {
                        if(strcmp(args[i],"<")==0)
                        {    
                                args[i]=NULL;
                                strcpy(input,args[i+1]);
                                in=2;    
                        }    

                        else if(strcmp(args[i],">")==0)
                        {    
                                args[i]=NULL;
                                strcpy(output,args[i+1]);
                                out=1;
                        }
                        else if (strcmp(args[i],">>")==0)
                        {
                                args[i]=NULL;
                                strcpy(output,args[i+1]);
                                out=2;
                        }
                }
                if(in)
                {   int fd0;
                        if ((fd0 = open(input, O_RDONLY, 0)) < 0)  
                        {
                                perror("Couldn't open input file");
                                exit(0);
                        }
                        dup2(fd0,0);
                        close(fd0);
                }
                else if (out==2)
                {
                        int fd2 ;
                        if ((fd2 = open(output , O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR| S_IXUSR , 0)) < 0) {
                                perror("Couldn't open the output file");
                                exit(0);
                        }    

                        dup2(fd2, 1); 
                        close(fd2);
                }
                if (out==1)
                {
                        int fd1 ;
                        if ((fd1 = creat(output , 0644)) < 0) {
                                perror("Couldn't open the output file");
                                exit(0);
                        }    

                        dup2(fd1, 1); 
                        close(fd1);
                }

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

