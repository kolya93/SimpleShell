#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to check if a string contains a substring
int contains_pattern(const char *line, const char *pattern) {
    return strstr(line, pattern) != NULL; // Returns true if pattern is found
}

int main(int argc, char *argv[]) {
    // Check if the program has the correct number of arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s pattern \n", argv[0]);
        return EXIT_FAILURE;
    }

    // The pattern to search for is the first argument
    const char *pattern = argv[1];

    // If no file is provided, read from standard input
    
    char line[1024];
    while (fgets(line, sizeof(line), stdin)) {
        if (contains_pattern(line, pattern)) {
            printf("%s", line);
        }
    }
     
    return EXIT_SUCCESS;
}
