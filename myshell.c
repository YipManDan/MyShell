#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#define BUFFERSIZE 1024
#define BUFFERSIZE2 64

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

char **split_line(char *string) {
	int sizeofbuf = 64;
	int sizeofarg;
	char **args = malloc(sizeofbuf * sizeof(char*));
	char *arg;
	char c;
	int i, j, k;
	if(args == NULL) {
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

int execute(char **arguments) {
	int i = 0;
	//While loop displays arguments for testing purposes
	while(arguments[i] != NULL) {
		printf("Arg #%i: %s\n", i, arguments[i]);
		i++;
	}
	if(strcmp(arguments[0], "exit") == 0) {
		exit(0);
	} else if(strcmp(arguments[0], "pwd") == 0) {
		char temp[PATH_MAX + 1];
		char *cwd;
		cwd = getcwd(temp, 1024);
		printf("%s\n", cwd);
	} else {
		printf("Command not found\n");
	}
	return 0;
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
