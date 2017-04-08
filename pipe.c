#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<pwd.h>
#include<sys/wait.h>
void pipeHandler(char ** args){

        int i=0,j=0,k=0,l=0;
        char *command[256];

        pid_t pid;
        char input[64],output[64];
        int end = 0,num_cmds=0;
        int freq=0;

        int err = -1,filedes[2],filedes2[2];

        int fd0,fd1,in=0,out=0;


        while (args[l] != NULL)
        {
                if (strcmp(args[l],"|") == 0)
                {
                        num_cmds++;
                }
                l++;
        }
        num_cmds++;

        while (args[j] != NULL && end != 1)
        {
                k = 0;
                while (strcmp(args[j],"|") != 0)
                {
                        command[k] = args[j];
                        j++;	
                        if (args[j] == NULL)
                        {
                                end = 1;
                                k++;
                                break;
                        }
                        k++;
                }
                command[k] = NULL;
                j++;		
                if (i%2==0)
                {

                        pipe(filedes2); 
                }

                else if (i % 2 != 0)
                {
                        pipe(filedes);
                }

                pid=fork();

                if(pid==-1)
                {			
                        if (i != num_cmds - 1)
                        {
                                if (i%2==0)
                                {
                                        close(filedes2[1]); 
                                } 
                                else if (i % 2 != 0)
                                {
                                        close(filedes[1]); 
                                }
                        }			
                        printf("Child process could not be created\n");
                        return;
                }
                if(pid==0)
                {
                        if (i == num_cmds - 1)
                        {
                                for(i=0;command[i]!='\0';i++)
                                {

                                        if(strcmp(command[i],">")==0)
                                        {    
                                                command[i]=NULL;
                                                strcpy(output,command[i+1]);
                                                out=1;
                                        }
                                        else if (strcmp(command[i],">>")==0)
                                        {
                                                command[i]=NULL;
                                                strcpy(output,command[i+1]);
                                                out=2;
                                        }

                                }

                                if (out==2)
                                {
                                        int fd2 ;
                                        if ((fd2 = open(output , O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR| S_IXUSR , 0)) < 0) {
                                                perror("Couldn't open the output file");
                                                exit(0);
                                        }    

                                        dup2(fd2, 1); 
                                        close(fd2);
                                }

                                else if (out==1)
                                {
                                        if ((fd1 = creat(output , 0644)) < 0) 
                                        {
                                                perror("Couldn't open the output file");
                                                exit(0);
                                        }

                                        dup2(fd1, 1);
                                        close(fd1);
                                }



                                if (num_cmds % 2 != 0)
                                { 
                                        dup2(filedes[0],STDIN_FILENO);
                                }
                                else
                                {
                                        dup2(filedes2[0],STDIN_FILENO);
                                }
                        }

                        else if (i == 0)
                        {
                                int j;
                                
                                in=0;

                                for(j=0;command[j]!=NULL;j++)
                                {
                                        //  printf("%s\n",command[j]);
                                        if(strcmp(command[j],"<")==0)
                                        {    
                                                command[j]=NULL;
                                                strcpy(input,command[j+1]);
                                                in=2;    
                                        }
                                }

                                if(in==2)
                                {   
                                        if ((fd0 = open(input, O_RDONLY, 0)) < 0)  
                                        {
                                                perror("Couldn't open input file");
                                                exit(0);
                                        }
                                        dup2(fd0,0);
                                        close(fd0);
                                }



                                dup2(filedes2[1], STDOUT_FILENO);
                        }
                        else
                        {
                                if (i % 2 != 0)
                                {
                                        dup2(filedes2[0],STDIN_FILENO); 
                                        dup2(filedes[1],STDOUT_FILENO);
                                }
                                else if (i%2==0)
                                { 
                                        dup2(filedes[0],STDIN_FILENO); 
                                        dup2(filedes2[1],STDOUT_FILENO);					
                                } 
                        }

                        if (execvp(command[0],command)==err)
                        {
                                kill(getpid(),SIGTERM);
                        }	
                        int bla=0;
                        while (args[bla] != NULL)
                        {
                                if (strcmp(args[bla],"|") == 0)
                                {
                                        freq++;
                                }
                                bla++;
                        }
                        printf ("%d\n",freq);
                }

                if (i == 0)
                {
                        close(filedes2[1]);
                }
                else if (i == num_cmds - 1)
                {
                        if (num_cmds % 2 != 0)
                        {					
                                close(filedes[0]);
                        }
                        else
                        {					
                                close(filedes2[0]);
                        }
                }
                else
                {
                        if (i % 2 != 0)
                        {					
                                close(filedes2[0]);
                                close(filedes[1]);
                        }
                        else
                        {					
                                close(filedes[0]);
                                close(filedes2[1]);
                        }
                }

                waitpid(pid,NULL,0);

                i++;	
        }
}
