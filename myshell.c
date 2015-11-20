#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#define BUFFERSIZE 1024
#define BUFFERSIZE2 64

char *get_line(void);
char **split_line(char *);
int execute(char **);
int p_forker(char **);

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
	char temp[PATH_MAX + 1];
	char *cwd;
	char c;
	int i;
	cwd = getcwd(temp, 1024);
	printf("shell=<%s>/myshell\n", cwd);
	do {
		cwd = getcwd(temp, 1024);
		printf("myshell: %s", cwd);
		printf(">");
		string = get_line();
		arguments = split_line(string);
		i = execute(arguments);

		free(string);
		free(arguments);
	} while(i);
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
	for(i=0; i<num_main_func(); i++)
	{
		if(strcmp(main_str[i], args[0])==0)
		{
			return (*main_func[i])(args);
		}
	}
	return p_forker(args);
}

/*
Arguments: pointer to char pointer, aka array of argument strings
Return: status integer
Function will be accessed when args[0] is cd, args[1] should be commands or filepath
*/
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

/*
Arguments: pointer to char pointer
Return: status integer
Function will be accessed with help command and return information about the myshell program
*/
int help_func(char **args) {
	int i;
	printf("\n\nThis is Daniel Yeh's MyShell\n");
	printf("To please enter in a valid command with or without arguments\n");
	printf("Here are the MyShell custom commands:\n");
	for(i=0; i<num_main_func(); i++)
	{
		printf("%s\n", main_str[i]);
	}
	printf("\nUse the man command for information regarding the other commands\n\n");
	return 1;
}

/*
Arguments: pointer to char pointer (unused)
Return: status integer;
Function will be accessed with exit command and will exit the program
*/
int exit_func(char **args) {
//	exit(0);
	return 0;
}

int p_forker(char **args) {
	pid_t pid, wpid;
	int status;

	pid=fork();
	if (pid == 0)
	{
		if(execvp(args[0], args) == -1)
		{
			perror("myshell");
		}
		exit(EXIT_FAILURE);
	} else if(pid < 0) {
		perror("myshell");

	} else {
		do 
		{
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

/*
if(strcmp(args[0], "pwd") == 0) {
	char temp[PATH_MAX + 1];
	char *cwd;
	cwd = getcwd(temp, 1024);
	printf("%s\n", cwd);
*/
