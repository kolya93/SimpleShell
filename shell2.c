#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int STRING_LEN = 1000;
int ARGV_SIZE= 1000;


void makeStringEmpty(char* str) {
	str[0] = '\0';
}


void deepcopy(char* array[], int index, char* str) {
	printf("copying %s to array[%d]\n", str, index);

	char* temp = (char*)malloc(strlen(str) * sizeof(char));
	if (temp == NULL) {
		printf("deepcopy() failed to malloc\n");
		exit(1);
	}

	strcpy(temp, str);
	array[index] = temp;
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
		return;
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


	return size;

}



void callLs(char** argv) {
	return;
}


void parseCommand(char** argv) {
	//ls
	if (strcmp(argv[0], "ls") == 0) {
		printf("executing ls...");
		callLs(argv[0]);
		return;
	}
	//cd
	if (strcmp(argv[0], "ls") == 0) {
		printf("executing cd...");
		return;
	}
}


int main() {
	printf("Welcome to SimpleShell!\nThis is a minimalistic unix-like shell. Type `help` for a list of commands.\n>");
	char* input = (char*)malloc(STRING_LEN * sizeof(char));

	fgets(input, STRING_LEN, stdin);
	for (int i=0; i<strlen(input); ++i) {
		if (input[i] == '\n' || input[i] == '\t')
			input[i] = '\0';
	}

	printf("%s\n", input);

	
	for (int i=0; i<STRING_LEN; ++i) {
		printf("%c\n", input[i]);
		if (input[i] == '\t')
			printf("Tab!!");

		if (input[i] == '\0') {
			printf("null!\n");
			break;
		}
	}

	char** argv = (char**)malloc(ARGV_SIZE * sizeof(char**));

	split the input into a vector of strings
	int argvSize = split(argv, input);

	parseCommand(argv);




	



}