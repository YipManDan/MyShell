#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <ncurses.h>
#define BUFFERSIZE 1024
#define BUFFERSIZE2 64

char *get_line(void);
char **split_line(char *);
int execute(char **);

int cd_func(char **);
int help_func(char **);
int exit_func(char **);

//Array of functions to be executed by main process
char *main_str[] = {
		"cd", 
		"help", 
		"exit"
};

//array of functions to be executed by main process
int (*main_func[])(char **) = 
{
	&cd_func,
	&help_func,
	&exit_func
};

//number of functions that the main process will execute
int num_main_func()
{
	return sizeof(main_str) / sizeof(char *);
}

int main() {
	//char string[30];
	char *string;
	char **arguments;
	char c;
	int i;
	printf("Hello World!\n");
	do {
		printf(">");
		string = get_line();
		arguments = split_line(string);
		execute(arguments);
	} while(true);
	return 0;
}

char *get_line() {
	int sizeofbuf = BUFFERSIZE;
	char *buffer = malloc(sizeof(char) * sizeofbuf);
	char c;
	int i;

	if(buffer == NULL) {
		printf("get_line: allocation error");
		exit(1);
	}
	while(true) {
		c = getchar();

		if(c == EOF || c == '\n') {
			buffer[i] = '\0';
			return buffer;
		} else {
			buffer[i] = c;
		}
		i++;
		if(i >= sizeofbuf) {
			sizeofbuf = 2*sizeofbuf;
			buffer = realloc(buffer, sizeofbuf);
			if(buffer == NULL) {
				printf("get_line: allocation error");
				exit(1);
			}
		}
	}
}

/*
arguments; user inputted string
return: pointers to char pointers aka array of argument string
Function will parse inputted string into separate arguments (delimted by: spaces, tabs)
   */

char **split_line(char *string) {
	int sizeofbuf = 64;
	int sizeofarg;
	char **args = malloc(sizeofbuf * sizeof(char*));
	char *arg;
	char c;
	int i, j, k; if(args == NULL) {
		printf("split_line: allocation error");
		exit(1);
	}
	i = 0;
	j = -1;
	k = 0;
	while(true) {
		c = string[++j];
		k = 0;
		sizeofarg = BUFFERSIZE2;
		arg = malloc(sizeofarg* sizeof(char));
		if(arg == NULL) {
			printf("split_line arg: allocation error");
			exit(1);
		}
		while(c != ' ' && c != '\0' && c != '\t') {
			arg[k] = c;
			k++;
			j++;
			c = string[j];
			if(k >= sizeofarg) {
				sizeofarg = 2*sizeofarg;
				arg = realloc(arg, sizeofarg);
				if(arg == NULL) {
					printf("split_line arg: allocation error");
					exit(1);
				}
			}
		}
		args[i] = arg;
		i++;
		if(c == '\0') {
			args[i] = NULL;
			return args;
		}
		if (i >= sizeofbuf) {
			sizeofbuf = sizeofbuf*2;
			args = realloc(args, sizeofbuf);
			if(args == NULL) {
				printf("split_line: allocation error");
				exit(1);
			}
		}
	}
}

/*
Arguments: pointer to char pointer, aka array of argument strings
Return: status integer
Function will execute arguments
*/

int execute(char **args) {
	int i = 0;
	//While loop displays arguments for testing purposes
	while(args[i] != NULL) {
		printf("Arg #%i: %s\n", i, args[i]);
		i++;
	}
	//check to see if args is empty
	if(i==0)
		return 1;
	//command: exit
	if(strcmp(args[0], "exit") == 0) {
		(*main_func[2])(args);
	//command: pwd
	} else if(strcmp(args[0], "pwd") == 0) {
		char temp[PATH_MAX + 1];
		char *cwd;
		cwd = getcwd(temp, 1024);
		printf("%s\n", cwd);
	} else if(strcmp(args[0], "cd") == 0) {
		(*main_func[0])(args);
	} else if(strcmp(args[0], "clr") == 0) {
		system("clear");
	//default command
	} else {
		printf("Command not found\n");
	}
	return 0;
}

int cd_func(char **args) {
	if (args[1] == NULL) 
	{
		fprintf(stderr, "myshell: expected argument to \"cd\"\n");
	} else 
	{
		if(chdir(args[1]) != 0) {
			printf("cd: %s: No such file or directory\n", args[1]);
		}
	}
	return 1;
}

int help_func(char **args) {
	return 1;
}

int exit_func(char **args) {
	exit(0);
	return 0;
}





