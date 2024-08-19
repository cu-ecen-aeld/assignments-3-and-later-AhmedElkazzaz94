#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    // Initialize syslog with LOG_USER facility
    openlog("writer", LOG_PID | LOG_CONS, LOG_USER);

    // Check if the correct number of arguments are provided
    if (argc != 3) {
        syslog(LOG_ERR, "Invalid number of arguments. Expected 2, got %d", argc - 1);
        fprintf(stderr, "Usage: %s <writefile> <writestr>\n", argv[0]);
        exit(1);
    }

    // Extract arguments
    const char *writefile = argv[1];
    const char *writestr = argv[2];

    // Attempt to open the file for writing
    FILE *file = fopen(writefile, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Could not open file %s for writing", writefile);
        fprintf(stderr, "Error: Could not open file %s for writing\n", writefile);
        exit(1);
    }

    // Write the string to the file
    fprintf(file, "%s", writestr);
    syslog(LOG_DEBUG, "Writing '%s' to '%s'", writestr, writefile);

    // Close the file
    fclose(file);

    // Close the syslog
    closelog();

    return 0;
}