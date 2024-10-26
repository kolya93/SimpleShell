#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int STRING_LEN = 1000;
int ARGV_SIZE= 1000;


void makeStringEmpty(char* str) {
	str[0] = '\0';
}



//fills argv with split str
int split(char** argv, char* str) {

	//find the first non-space
	int startIndex = -1;
	for (int i=0; str[i] != '\0'; ++i) {
		if (str[i] != ' ') {
			startIndex = i;
			break;
		}
	}

	//if it's all spaces, then return
	if (startIndex == -1) {
		return 0;
	}

	int argvSize = 0;
	int tempSize = 0;
	char temp[STRING_LEN];
	for (int i=startIndex; str[i] != '\0'; ++i) {
		if (str[i] != ' ') {
			temp[tempSize] = str[i];
			++tempSize;
			continue;
		}


		if (str[i] == ' ' && i != 0 && str[i - 1] != ' ') {
			//if str[i] is a space, then copy it to argv, clear temp, reset tempSize 
			temp[tempSize] = '\0';
			argv[argvSize] = malloc(tempSize + 5);
			strcpy(argv[argvSize], temp);
			++argvSize;

			//clear temp
			for (int j=0; temp[j] != '\0'; ++j) {
				temp[j] = '\0';
			}

			tempSize = 0;	
		}	
	}

	if (tempSize != 0) {
		//if str[i] is a space, then copy it to argv, clear temp, reset tempSize 
		temp[tempSize] = '\0';
		argv[argvSize] = malloc(tempSize + 5);
		strcpy(argv[argvSize], temp);
		++argvSize;
	}


	for (int i=0; i<argvSize; ++i)
		printf("-%s-\n", argv[i]);


	return argvSize;

}









void parseCommand(char** argv, int argvSize) {
	//first check for cd
	if (strcmp(argv[0], "cd") == 0) {
		printf("executing cd...\n");

		if (argvSize != 2) {
			perror("cd takes one arg\n");
			return;
		}
		
		//change the cwd to argv[1]
		if (chdir(argv[1]) != 0) {
    	    perror("chdir failed\n");
    	    return;
		}

		return;
	}

	//just fork into the the process specified
	pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        execvp(argv[0], argv); // Replace child process with argv
        
        // execvp only returns on failure
        perror("exec failed");
        exit(1);
    } else {
        // Parent process
        wait(NULL); // Wait for the child to finish
        printf("Child process finished.\n");
    }
}


int main() {
	printf("Welcome to SimpleShell!\nThis is a simple unix-like shell. Type `help` for a list of commands.\n>");
	char* input = (char*)malloc(STRING_LEN * sizeof(char));

	fgets(input, STRING_LEN, stdin);

	//removes all `\n` and `\t`
	for (int i=0; i<strlen(input); ++i) {
		if (input[i] == '\n' || input[i] == '\t')
			input[i] = '\0';
	}

	printf("%s\n", input);

	

	char** argv = (char**)malloc(ARGV_SIZE * sizeof(char**));

	// split the input into a vector of strings
	int argvSize = split(argv, input);

	printf("%s\n", argv[0]);

	parseCommand(argv, argvSize);




	



}