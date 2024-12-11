#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024

// Structure to hold each line of input
typedef struct {
    char *line;
} Line;

// Comparison function for sorting (ascending)
int compare_asc(const void *a, const void *b) {
    return strcmp(((Line *)a)->line, ((Line *)b)->line);
}

// Comparison function for sorting (descending)
int compare_desc(const void *a, const void *b) {
    return strcmp(((Line *)b)->line, ((Line *)a)->line);
}

// Comparison function for sorting (ignoring case)
int compare_asc_ignore_case(const void *a, const void *b) {
    return strcasecmp(((Line *)a)->line, ((Line *)b)->line);
}

// Comparison function for sorting (descending, ignoring case)
int compare_desc_ignore_case(const void *a, const void *b) {
    return strcasecmp(((Line *)b)->line, ((Line *)a)->line);
}

int main(int argc, char *argv[]) {
    int ignore_case = 0;   // Default is not ignoring case
    int descending = 0;    // Default is ascending order

    // Process command line options (e.g., -r for reverse, -f for ignore case)
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'f')) {
                ignore_case = 1;  // -f: ignore case
            }
            if (strchr(argv[i], 'r')) {
                descending = 1;  // -r: reverse order (descending)
            }
        } else {
            // In this version, we don't expect any file argument.
            fprintf(stderr, "Unexpected argument: %s\n", argv[i]);
            return 1;
        }
    }

    // Read all lines from stdin
    Line *lines = NULL;
    size_t num_lines = 0;
    char buffer[MAX_LINE_LENGTH];

    while (fgets(buffer, sizeof(buffer), stdin)) {
        size_t len = strlen(buffer);
        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';  // Remove the newline character
        }

        // Reallocate memory for the new line
        lines = realloc(lines, (num_lines + 1) * sizeof(Line));
        lines[num_lines].line = strdup(buffer);  // Copy the line
        num_lines++;
    }

    // Determine the appropriate comparison function based on options
    int (*compare)(const void *, const void *);
    if (ignore_case) {
        compare = descending ? compare_desc_ignore_case : compare_asc_ignore_case;
    } else {
        compare = descending ? compare_desc : compare_asc;
    }

    // Sort the lines using qsort
    qsort(lines, num_lines, sizeof(Line), compare);

    // Print the sorted lines
    for (size_t i = 0; i < num_lines; i++) {
        printf("%s\n", lines[i].line);
        free(lines[i].line);  // Free each line
    }

    free(lines);  // Free the array of lines

    return 0;
}
