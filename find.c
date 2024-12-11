#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

// Function to recursively search for files
void find_files(const char *dir_name, const char *search_name) {
    struct dirent *entry;
    DIR *dir = opendir(dir_name);

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    // Iterate through all entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_name, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        // Check if it's a directory or file
        if (S_ISDIR(statbuf.st_mode)) {
            // recurse
            find_files(full_path, search_name);
        } else if (S_ISREG(statbuf.st_mode)) {
            if (strcmp(entry->d_name, search_name) == 0) {
                printf("%s\n", full_path);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *dir_name = argv[1];
    const char *search_name = argv[2];

    // Start the search from the given directory
    find_files(dir_name, search_name);

    return 0;
}
