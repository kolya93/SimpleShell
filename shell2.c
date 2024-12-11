#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <dirent.h> 
#include <fcntl.h>

// Define ANSI color codes
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"

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


void list_directory() {
    DIR *dir;
    struct dirent *entry;

    // Open the current directory (".")
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // Read and print each directory entry
    while ((entry = readdir(dir)) != NULL) {
        // Ignore the "." and ".." directories
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\n", entry->d_name);
        }
    }

    // Close the directory
    closedir(dir);
}



void cat(const char* filename) {
	FILE *file = fopen(filename, "r");  // Open the file in read mode
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    int ch;
    // Read each character and print it
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }

    fclose(file);  // Close the file
}


void move_file_to_directory(const char *src, const char *dest_dir) {
    char dest[1024];  // Buffer to store destination path

    // Ensure the source file exists
    if (access(src, F_OK) == -1) {
        perror("Source file does not exist");
        exit(1);
    }

    // Ensure the destination directory exists
    if (access(dest_dir, F_OK) == -1 || access(dest_dir, R_OK | X_OK) == -1) {
        perror("Destination directory does not exist or is not accessible");
        exit(1);
    }

    // Concatenate destination directory and file name
    snprintf(dest, sizeof(dest), "%s/%s", dest_dir, strrchr(src, '/') ? strrchr(src, '/') + 1 : src);

    // Attempt to rename (move) the file
    if (rename(src, dest) == 0) {
        printf("Moved file '%s' to '%s'\n", src, dest);
    } else {
        perror("Error moving file");
        exit(1);
    }
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






void execute_command(char *cmd, char *const argv[], int input_fd, int output_fd) {
    if (input_fd != 0) {  // If there's an input pipe, redirect stdin
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("dup2 input");
            exit(1);
        }
    }
    if (output_fd != 1) {  // If there's an output pipe, redirect stdout
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("dup2 output");
            exit(1);
        }
    }

    // Execute the command
    execvp(cmd, argv);
    perror(RED "execvp failed" RESET);  // If execvp fails
    exit(1);
}



void executePipe(char** argv, int argvSize) {

	char** componentArray[100]; //this is an array of each of the components of the pipeline

	for (int i=0; i<100; ++i) {
		componentArray[i] = malloc(100 * sizeof(char*));
		
	}

	int componentIndex = 0;
	int componentArraySize = 0;

	int indexOfLastPipe = 0;

	for (int i=0; i<argvSize; ++i) {

		if (strcmp(argv[i], "|") == 0) {
			++componentArraySize;
			indexOfLastPipe = i + 1;
			++componentIndex;

			continue;
		}

		componentArray[componentIndex][i - indexOfLastPipe] = argv[i];
	}
	++componentArraySize;

	int num_commands = componentArraySize;

	int pipefds[2 * (num_commands - 1)]; // Create enough pipes for the chain of commands

    // Create pipes for each pair of commands
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    // Create processes and set up pipes
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            // Determine input and output for each command
            int input_fd = (i == 0) ? 0 : pipefds[(i - 1) * 2];  // Input from previous pipe
            int output_fd = (i == num_commands - 1) ? 1 : pipefds[i * 2 + 1];  // Output to next pipe

            // Close all unused pipe ends
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                if (j != (i - 1) * 2 && j != i * 2 + 1) {
                    close(pipefds[j]);
                }
            }

            // Execute the command
            execute_command(componentArray[i][0], componentArray[i], input_fd, output_fd);
        }
    }

    // Parent process: Close all pipe file descriptors and wait for children
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    for (int i = 0; i < num_commands; i++) {
        wait(NULL);  // Wait for each child to finish
    }

}


void forkOutputRedirect(char** argvWriter, int argvSizeWriter, char** file) {
	pid_t pid = fork();  // Create a child process

    if (pid < 0) {
        // Fork failed
        perror(RED "fork failed" RESET);
        return;
    }

    if (pid == 0) {
        // Child process

        // Open the file for writing (create if it doesn't exist)
        int fd = open(file[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            printf(RED "Error opening %s\n RESET", file[0]);
            return;
        }

        // Redirect stdout (file descriptor 1) to the file (fd)
        dup2(fd, STDOUT_FILENO);  // STDOUT_FILENO is 1

        // Close the file descriptor (it’s already duplicated to stdout)
        close(fd);

        // Exec
        execvp(argvWriter[0], argvWriter);

        // If execlp() fails
        perror(RED "execlp failed" RESET);
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

	printf(RED "Sorry, for redirecting output to files, the `>` must be in between spaces.\n" RESET);
	return;
}


void forkInputRedirect(char** argvWriter, int argvSizeWriter, char** file) {
	pid_t pid = fork();  // Create a child process

    if (pid < 0) {
        // Fork failed
        perror(RED "fork failed" RESET);
        return;
    }

    if (pid == 0) {
        // Child process

        // Open the file for writing (create if it doesn't exist)
        int fd = open(file[0], O_RDONLY);
        if (fd < 0) {
            printf(RED "Error opening %s\n RESET", file[0]);
            return;
        }

        // Redirect stdout (file descriptor 1) to the file (fd)
        if (dup2(fd, STDIN_FILENO) == -1) {
    	    perror("Failed to redirect input");
    	    close(fd);
    	    return;
    	}

        // Close the file descriptor (it’s already duplicated to stdout)
        close(fd);

        // Exec
        execvp(argvWriter[0], argvWriter);

        // If execlp() fails
        perror(RED "execlp failed" RESET);
        return;
    }
    else {
        // Parent process
        wait(NULL);  // Wait for the child to finish
    }

}



void executeInputRedirection(char** argv, int argvSize) {
	for (int i=0; i<argvSize; ++i) {
		if (strcmp(argv[i], "<") == 0) {

			char* reader[ARGV_SIZE];
			char* file[ARGV_SIZE];

			//copy the first half of the redirect command into reader
			for (int j=0; j<i; ++j) {
				reader[j] = argv[j];
			}

			//copy the second half of redirect command to file
			for (int j=i+1; j<argvSize; ++j) {
				file[j - (i + 1)] = argv[j];
			}

			forkInputRedirect(reader, i, file);
			return;
		}
	}

	printf(RED "Sorry, for redirecting input to files, the `<` must be in between spaces.\n" RESET);
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

	//check if it's input redirection
	for (int i=0; i<argvSize; ++i) {
		for (int j=0; argv[i][j] != '\0'; ++j) {
			if (argv[i][j] == '<') {
				executeInputRedirection(argv, argvSize);
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

	if (strcmp(argv[0], "ls") == 0) {
		list_directory();
		return;
	}

	if (strcmp(argv[0], "cat") == 0) {
		cat(argv[1]);
		return;
	}

	if (strcmp(argv[0], "move") == 0) {
		if (argvSize != 3) 
			printf(RED "move requires 2 a source and a dest\n" RESET);
		
		move_file_to_directory(argv[1], argv[2]);
		return;
	}



	//check if it's a help command
	if (strcmp(argv[0], "help") == 0) {
		printf(MAGENTA "This is a simple unix-like shell written in c for my RCOS project. It is meant for educational purposes. You can execute all the usual usr/bin binaries (ie `ls -l`). Also you can pipe two processes with `(process1) | (process2)` (but make sure the `|` is between two spaces). There is input redirection to files with `(process1) < (file)`, and output redirection to files with `(process1) > (file)` Type `end` to close the shell. And use `man` for everything else.\n" RESET);
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
        perror(RED "fork failed" RESET);
        exit(1);
    } else if (pid == 0) {
        // Child process
        execvp(argv[0], argv); // Replace child process with argv
        
        // execvp only returns on failure
        perror(RED "exec failed" RESET);
        exit(1);
    } else {
        // Parent process
        wait(NULL); // Wait for the child to finish
    }
}


int main() {
	printf(YELLOW "Welcome to SimpleShell!\nThis is a simple unix-like shell. Type `help` for a list of commands.\n" RESET);
	char* input = (char*)malloc(STRING_LEN * sizeof(char));


	do {
		printf(BLUE ">" RESET);
	
		fgets(input, STRING_LEN, stdin);

		//removes all `\n` and `\t`
		for (int i=0; i<strlen(input); ++i) {
			if (input[i] == '\n' || input[i] == '\t')
				input[i] = '\0';
		}

		char** argv = (char**)malloc(ARGV_SIZE * sizeof(char**));

		// split the input into a vector of strings
		int argvSize = split(argv, input);

		if (argvSize == 0) {
			printf(RED "empty string is not a valid command\n" RESET); 
			continue;
		}


		parseCommand(argv, argvSize);

		//free everything
		for (int i=0; i<argvSize; ++i) {
			free(argv[i]);
		}

	} while (true);


}