#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

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



	return argvSize;

}



void forkWriterReader(char** argvWriter, int argvSizeWriter, char** argvReader, int argvSizeReader) {
	int pipefd[2];
    pid_t pid1, pid2;

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork the first child process (for cat)
    pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) { // Child process 1 (cat)
        // Close the write end of the pipe (no need to write to it)
        close(pipefd[0]);
        
        // Redirect stdout to the pipe (write end)
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Execute "cat" command
        execvp(argvWriter[0], argvWriter);
        perror("execlp writer");
        exit(EXIT_FAILURE);
    }

    // Fork the second child process (for grep)
    pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) { // Child process 2 (grep)
        // Close the read end of the pipe (no need to read from it)
        close(pipefd[1]);

        // Redirect stdin to the pipe (read end)
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        // Execute "grep" command
        execvp(argvReader[0], argvReader);
        perror("execvp reader");
        exit(EXIT_FAILURE);
    }

    // Parent process closes both ends of the pipe
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

}





void executePipe(char** argv, int argvSize) {

	// printf("executing pipe...\n");
	for (int i=0; i<argvSize; ++i) {
		if (strcmp(argv[i], "|") == 0) {

			char* writer[ARGV_SIZE];
			char* reader[ARGV_SIZE];

			//copy the first half of the pipe command into writer
			for (int j=0; j<i; ++j) {
				writer[j] = argv[j];
			}

			//copy the second half of pipe command to reader
			for (int j=i+1; j<argvSize; ++j) {
				reader[j - (i + 1)] = argv[j];
			}

			forkWriterReader(writer, i, reader, argvSize - i);
			return;
		}
	}

	printf("Sorry, for pipe commands, the `|` must be in between spaces.\n");
	return;
}






void forkOutputRedirect(char** argvWriter, int argvSizeWriter, char** file) {
	pid_t pid = fork();  // Create a child process

    if (pid < 0) {
        // Fork failed
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child process

        // Open the file for writing (create if it doesn't exist)
        int fd = open(file[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            printf("Error opening %s\n", file[0]);
            return;
        }

        // Redirect stdout (file descriptor 1) to the file (fd)
        dup2(fd, STDOUT_FILENO);  // STDOUT_FILENO is 1

        // Close the file descriptor (it’s already duplicated to stdout)
        close(fd);

        // Exec
        execvp(argvWriter[0], argvWriter);

        // If execlp() fails
        perror("execlp failed");
        return;
    }
    else {
        // Parent process
        wait(NULL);  // Wait for the child to finish
    }

}




void executeOutputRedirection(char** argv, int argvSize) {

	// printf("executing pipe...\n");
	for (int i=0; i<argvSize; ++i) {
		if (strcmp(argv[i], ">") == 0) {

			char* writer[ARGV_SIZE];
			char* file[ARGV_SIZE];

			//copy the first half of the redirect command into writer
			for (int j=0; j<i; ++j) {
				writer[j] = argv[j];
			}

			//copy the second half of redirect command to file
			for (int j=i+1; j<argvSize; ++j) {
				file[j - (i + 1)] = argv[j];
			}

			forkOutputRedirect(writer, i, file);
			return;
		}
	}

	printf("Sorry, for redirecting output to files, the `>` must be in between spaces.\n");
	return;
}





void parseCommand(char** argv, int argvSize) {

	//check if it's a pipe
	for (int i=0; i<argvSize; ++i) {
		for (int j=0; argv[i][j] != '\0'; ++j) {
			if (argv[i][j] == '|') {
				executePipe(argv, argvSize);
				return;
			}
		}
	}

	//check if it's output redirection
	for (int i=0; i<argvSize; ++i) {
		for (int j=0; argv[i][j] != '\0'; ++j) {
			if (argv[i][j] == '>') {
				executeOutputRedirection(argv, argvSize);
				return;
			}
		}
	}

	//check for cd
	if (strcmp(argv[0], "cd") == 0) {

		if (argvSize != 2) {
			printf("cd takes one arg\n");
			return;
		}
		
		//change the cwd to argv[1]
		if (chdir(argv[1]) != 0) {
    	    perror("chdir failed\n");
    	    return;
		}

		return;
	}


	//check if it's a help command
	if (strcmp(argv[0], "help") == 0) {
		printf("This is a simple unix-like shell written in c for my RCOS project. It is meant for educational purposes. You can execute all the usual usr/bin binaries (ie `ls -l`). Also you can pipe two processes with `(process1) | (process2)` (but make sure the `|` is between two spaces). Type `end` to close the shell. And use `man` for everything else.\n");
		return;
	
	}

	//check if it's an end
	if (strcmp(argv[0], "end") == 0) {
		exit(0);
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
    }
}


int main() {
	printf("Welcome to SimpleShell!\nThis is a simple unix-like shell. Type `help` for a list of commands.\n");
	char* input = (char*)malloc(STRING_LEN * sizeof(char));



	do {
		printf(">");
	
		fgets(input, STRING_LEN, stdin);

		//removes all `\n` and `\t`
		for (int i=0; i<strlen(input); ++i) {
			if (input[i] == '\n' || input[i] == '\t')
				input[i] = '\0';
		}

		char** argv = (char**)malloc(ARGV_SIZE * sizeof(char**));

		// split the input into a vector of strings
		int argvSize = split(argv, input);

		parseCommand(argv, argvSize);

		//free everything
		for (int i=0; i<argvSize; ++i) {
			free(argv[i]);
		}

	} while (true);


	



}