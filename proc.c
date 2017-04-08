#include<stdio.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<pwd.h>
#include<sys/wait.h>
//function to provide info of the given process id
int fun_pinfo(char **args)
{
	char str[30000];
	char proc[1000]="/proc/";
	char status[1000]="/status";
	strcat(proc,args[1]);
	strcat(proc,status);
	FILE *p; 
	if((p=fopen(proc,"r"))==NULL)
	{
		fprintf(stderr,"Unable to open the required file\n");
		return 1;
	}
	while(fgets(str,70,p)!=NULL)
		puts(str);
	fclose(p);
	return 1;
}

