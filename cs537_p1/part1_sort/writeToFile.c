#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"

void
usage(char *prog)
{
    fprintf(stderr, "usage: %s -i fromFile -o toFile\n", prog);
    exit(1);
}

int
main(int argc, char *argv[])
{
    // arguments
    char *inFile = "/no/such/file";
    char *outFile = "ddd";

    // input params
    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "i:o:")) != -1) {
	switch (c) {
	case 'i':
	    inFile = strdup(optarg);
	    break;
	case 'o':
            outFile = strdup(optarg);
	    break;
	default:
	    usage(argv[0]);
	}
    }

    // open and create output file
    int fd = open(inFile, O_RDONLY);
    FILE *fpt = fopen(outFile, "w");

    rec_t r;
    while (1) {
	int rc;
	rc = read(fd, &r, sizeof(rec_t));
	if (rc == 0) // 0 indicates EOF
	    break;
	if (rc < 0) {
	    perror("read");
	    exit(1);
	}
	fprintf(fpt, "%u ", r.key);
	int j;
	for (j = 0; j < NUMRECS; j++)
	    fprintf(fpt, "%u ", r.record[j]);
	fprintf(fpt, "\n");
    }

    fclose(fpt);

    return 0;
}
