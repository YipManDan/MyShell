#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
//#include <sys/types.h>
//#include <sys/wait.h>
#define BUFFERSIZE 1024
#define BUFFERSIZE2 64

void set_environ(void);
char *get_line(void);
char **split_line(char *, bool *);
FILE *new_stream(char **, bool *);
int execute(char **);
int p_forker(char **);

int cd_func(char **);
int help_func(char **);
int pause_func(char **);
int clr_func(char **);
int environ_func(char **);
int exit_func(char **);

//Global variables
int arg_count;
bool background = false;
bool has_pipe = false;
bool new_out = false;
bool new_in = false;
FILE *envfile;

//Array of functions to be executed by main process
char *main_str[] = {
		"cd", 
		"help", 
		"pause", 
		"clr",
		"environ",
		"exit"
};

//array of functions to be executed by main process
int (*main_func[])(char **) = 
{
	&cd_func,
	&help_func,
	&pause_func,
	&clr_func,
	&environ_func,
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
	FILE *fp;
	bool error;
	bool iop;
	int i = 1;
	envfile = tmpfile();
	set_environ();
	do {
		error = false;
		iop = false;
		cwd = getcwd(temp, 1024);
		printf("myshell: %s", cwd);
		printf(">");
		string = get_line();
		arguments = split_line(string, &iop);
		if(iop)
		{
			fp = new_stream(arguments, &error);
		}
		if(!error)
			i = execute(arguments);
		if(iop && fp != NULL)
		{
			fclose(fp);
			freopen("/dev/tty", "a", stdout);
		}
		free(string);
		free(arguments);
	} while(i);
	return 0;
}

/*
   Function creates an environment file which contains the information printed by the system command "printenv". In the file, the shell address is replaced with the location of this myshell program.
   */
void set_environ()
{
	char temp[PATH_MAX + 1];
	char *cwd, line[BUFFERSIZE];
	char *delete_line = "SHELL="; //part of the line that will be erased
	FILE *file;
	int filedes;
	char tempname[BUFFERSIZE2];
	char pre[BUFFERSIZE2];

	strcpy(tempname, "environXXXXXX");
	filedes = mkstemp(tempname);
	
	printf("The temp name is: %s\n", tempname);
	cwd = getcwd(temp, 1024);

	strcpy(pre, "printenv > ");

	//Prints env details to a file
	system(strcat(pre, tempname));
	
	//Opens the local temp file and a temp file
	file = fdopen(filedes, "r");
	//tempfile = fopen("temp_environ", "w");
	
	//Checks to ensure that both files are opened correctly
	if(file == NULL || envfile == NULL)
	{
		printf("error: unable to open file");
		exit(1);
	}

	printf("###finding the string \"%s\"\n", delete_line);
	/*
	   This loop will loop through the file with the env information and print all lines to a second temp file. All lines except the shell path which is replaced with the path of this program
	   */
	while(fgets(line, sizeof(line), file))
	{
		if(strstr(line, delete_line)!=NULL)
		{
			printf("###string \"SHELL=\" found. Deleting...\n");
			fprintf(envfile, "SHELL=%s/myshell\n", cwd);
		}
		else
			fprintf(envfile, "%s", line);
	}

	//Both files are closed
	fclose(file);
	//fclose(tempfile);

	//local temp file is erased, then temp file is renamed
	remove(tempname);
	//rename("temp_environ", tempname);
	//__fpurge(stdin);
	return;
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

char **split_line(char *string, bool *iop) {
	int sizeofbuf = BUFFERSIZE2;
	int sizeofarg;
	char **args = malloc(sizeofbuf * sizeof(char*));
	char *arg;
	char c;
	int i, j, k; 
	has_pipe = false;
	new_out = false;
	new_in = false;
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
		//Checks for pipe, file output, or file input
		if(strcmp(arg, "|")==0)
			*iop = true;
			//has_pipe = true;
		else if (strcmp(arg, ">") == 0)
			*iop = true;
			//new_out = true;
		else if(strcmp(arg, "<") == 0)
			*iop = true;
			//new_in = true;

		//Checks for null character
		if(c == '\0') {
			args[i] = NULL;
			arg_count = i;
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

FILE *new_stream(char **args, bool *error) {
	int i = 0;
	FILE *fp = NULL;
	while(args[i+1] != NULL) {
		if(strcmp(args[i], ">")==0)
		{
			printf("Redirecting stdout to %s\n", args[i+1]);
			char *s;
			fp = freopen(args[i+1], "w", stdout);
			if(fp == NULL)
			{
				printf("Error: Unable to change stdout\n");
				*error = true;
				return NULL;
			}
			//free(args[i]);
			//free(args[i+1]);
			//args[i] = NULL;
			//args[i+1] = NULL;
			//arg_count--;
			//arg_count--;

			do
			{
				//strcpy(args[i], args[i+2]);
				args[i] = args[i+2];
				arg_count--;
				i++;
			} while(args[i+2] != NULL);
			args[i] = NULL;
			arg_count--;
			//args[i+1] = NULL;
			//args[i] = NULL;
			return fp;
		}
		else if(strcmp(args[i], "<") ==0)
		{
			fp = freopen(args[i-1], "r", stdin);
			return fp;
		}
		i++;
	}
	printf("myshell: syntax error near unexpected token 'newline'\n");
	*error = true;
	return NULL;
}

/*
Arguments: pointer to char pointer, aka array of argument strings
Return: status integer
Function will execute arguments
*/

int execute(char **args) {
	int i = 0;
	background = false;
	//While loop displays arguments for testing purposes
	printf("###Number of args: %d\n", arg_count);
	if(strcmp(args[arg_count-1], "&")==0)
		background = true;
	if(background)
	{
		printf("###Running command in background\n");
		free(args[arg_count-1]);
		args[arg_count-1]=NULL;
	}
	else
		printf("###Running command in foreground\n");
	
	while(args[i] != NULL) {
		printf("###Arg #%i: %s\n", i, args[i]);
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
	printf("\nThe command \"clr\" is not portable and is usable only on unix based systems");

	if(arg_count > 1)
	{
		strcpy(args[0], "man");
		p_forker(args);
	}

	printf("\nUse the help command with arguments for more information regarding the commands\n\n");
	return 1;
}

//Function pauses until newline or EOF is inputted
int pause_func(char **args) {
	int x;
	while(x = getchar() != EOF && getchar() != '\n');
	return 1;
}

//function calls system call clear
int clr_func(char **args) {
	system("clear");
	return 1;
}

//Function prints out the temporary file containing the environment details
int environ_func(char **args) {
	char line[1024];
	rewind(envfile);
	//system("cat environ.txt | more");
	while(fgets(line, sizeof(line), envfile))
	{
		printf("%s", line);
	}
	return 1;
}

/*
Arguments: pointer to char pointer (unused)
Return: status integer;
Function will be accessed with exit command and will exit the program
*/
int exit_func(char **args) {
//	exit(0);
	printf("Thank you for using this shell. Good bye!\n");
	fclose(envfile);
	return 0;
}

/*
Arguments: pointer to char pointer
Return: status integer
Function forks process. For child process execvp is called with args[0]. The parent process will wait until child process exits unless the background bool is set.
*/
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
		if(background)
			return 1;
		do 
		{
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

