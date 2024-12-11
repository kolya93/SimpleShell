#include <stdio.h>
#include <ctype.h>

void count_wc(FILE *file) {
    int ch;
    int lines = 0, words = 0, characters = 0;
    int in_word = 0;

    // Read character by character from the file or input
    while ((ch = fgetc(file)) != EOF) {
        characters++;  // Count every character

        if (ch == '\n') {
            lines++;  // Count lines when a newline character is encountered
        }

        // Count words: transitions from non-space to space characters
        if (isspace(ch)) {
            in_word = 0;  // End of a word
        } else {
            if (in_word == 0) {
                words++;  // Start of a new word
                in_word = 1;
            }
        }
    }

    printf("Lines: %d\n", lines);
    printf("Words: %d\n", words);
    printf("Characters: %d\n", characters);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        // No file specified, use standard input
        count_wc(stdin);
    } else {
        for (int i = 1; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");
            if (file == NULL) {
                perror("Error opening file");
                continue;
            }
            printf("File: %s\n", argv[i]);
            count_wc(file);
            fclose(file);
        }
    }
    return 0;
}
