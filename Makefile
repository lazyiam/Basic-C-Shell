main: shell.c builtin.c proc.c pipe.c redirect.c 
	gcc -g shell.c builtin.c proc.c pipe.c redirect.c -o main
