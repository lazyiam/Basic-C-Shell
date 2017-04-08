To execute the code run following-:
1)make
2)./main

Files enclosed-:
1)shell.c ->containing main function.
2)builtin.c ->containing the implementation of "ls pwd echo exit" commands.
3)builtin.h
4)proc.c ->module containing the function to provide process info ny pid.
5)proc.h
6)pipe.c
7)pipe.h
8)redirect.c
9)redirect.h
6)Makefile

After execution, command prompt is displayed with the current folder as home directory.
Type the bash commands with correct syntax, and they will get executed using fork() and exevp() syscall.
To get the info of a process type pinfo "pid".
To sequentially execute commands seperate them with ";".
To run a process in background type:
	process_name &
The directory path is showed relative to the directory in whicch you executed the code.
You can use any numbaer of pipes with redirections.
ctrl+c is applicable
fg,kjob,jobs,killallbg commands are usable.



