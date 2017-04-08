#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<pwd.h>
#include<sys/wait.h>
char path1[1025];
char spcl[5]="~";
int jobs();
int fun_cd(char **args)
{
	if (args[1] == NULL) 
	{
		fprintf(stderr, "expected argument to \"cd\"\n");
	} 
	else if (strcmp(args[1],spcl)==0)
	{
		chdir(path1);
	}
	else 
	{
		if (chdir(args[1]) != 0) 
		{
			perror("");
		}
	}
	return 1;
}
int fun_echo(char **args)
{
	int i;
	i=1;
	while(args[i]!=NULL)
	{
		printf("%s",args[i]);
		i=i+1;
	}
	printf("\n");

	return 1;
}
int fun_pwd(char ** args)
{
	char directory[1000];
	getcwd(directory,sizeof(directory));
	printf("%s\n",directory);
	return 1;
}

int fun_exit(char **args)
{
	return 0;
}
int fun_listjobs()
{
        jobs();
        return 1 ;
}
