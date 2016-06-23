#include <stdio.h>
#include <stdlib.h>
#include "regex.h"

#define MAXLINE 1024

void matchInput(FILE *fid, char *filename, char* regex) {
    char line[MAXLINE+1];
    int start, len;

    while (1) {
        if (fgets(line, MAXLINE, fid) == NULL) break;

        if (match(line, regex, &start, &len)) {
            printf("[");
            printStr(line, start, len);
            printf("] in %s: ", filename);
            printf("%s", line);
        }
    }

    return;
}

int main(int argc, char **argv) {

    FILE *fid;
    char regex[MAXLINE+1];
    int i;

    if (argc == 1) {
        fprintf(stderr, "USAGE: %s 'regex' files\n", argv[0]);
        return 0;
    }

    strncpy(regex, argv[1], MAXLINE);

    if (argc == 2) {
        fid = stdin;
        matchInput(fid, "", regex);
    }

    if (argc > 2) {
        for (i = 2; i < argc; i++) {
            fid = fopen(argv[i],"r");
            if (fid == NULL) {
                fprintf(stderr, "Input file '%s' failed to open.\n", argv[i]);
                continue;
            } // End If

            matchInput(fid, argv[i], regex);

            fclose(fid);
        }
    }

    return 0;
}
