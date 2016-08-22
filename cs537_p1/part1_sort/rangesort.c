#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/types.h>
#include <sys/stat.h>

void usage(char *prog) {
  char *exe = NULL;
  char *token = strtok(prog, "/");
  
  while (token) {
    exe = token;
    token = strtok(NULL, "/");
  }

  fprintf(stderr, "Usage: %s -i inputfile -o outputfile -l lowvalue -h highvalue\n", exe);
  exit(1);
}

int compKeyFunc (const void *r1, const void *r2) {
  return (*(rec_t* const *)r1)->key - (*(rec_t* const *)r2)->key;
}

int main(int argc, char *argv[]) {
  unsigned long low, high; 
  unsigned long *lowValue = NULL, *highValue = NULL;
  char *inputFile = NULL, *outputFile = NULL, *ptr;
  double val;

  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "i:o:l:h:")) != -1) {
    switch (c) {
    case 'i':
        inputFile = strdup(optarg);
        break;
    case 'o':
        outputFile = strdup(optarg);
        break;
    case 'l':
        val = atof(optarg);
        low = strtoul(optarg, &ptr, 10);
        if ((isdigit(*optarg) != 0) && (val < 4294967296) && (strlen(ptr) == 0)) {
          lowValue = &low;  
	} else {
          fprintf(stderr, "Error: Invalid range value\n");
          exit(1);
	}
	break;
    case 'h':
        val = atof(optarg);
        high = strtoul(optarg, &ptr, 10);
        if (isdigit(*optarg) && val < 4294967296 && strlen(ptr) == 0) {
          highValue = &high;  
	} else {
          fprintf(stderr, "Error: Invalid range value\n");
          exit(1);
	}
	break;
    default:
        usage(argv[0]);
    }
  }

  if (argc != 9 || !lowValue || !highValue || !inputFile || !outputFile) {
    usage(argv[0]);
  }
  
  if (*lowValue > *highValue) {
    fprintf(stderr, "Error: Invalid range value\n");
    exit(1);
  }
  
  // open binary file storing data to be sorted
  int fd = open(inputFile, O_RDONLY);
  if (fd <0) {
    fprintf(stderr, "Error: Cannot open file %s\n", inputFile);
    exit(1);
  }
 
  // access inputFile info and get file size
  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1) {
    fprintf(stderr, "Error: cannot access info of file: %s successfully\n", inputFile);
    exit(1);
  }
  
  // total data size in bytes
  int fileSize = statbuf.st_size;

  // set buffer storing data from inputFile
  rec_t *recordBuf;
  recordBuf = (rec_t *)malloc(fileSize);

  // exit cleanly if malloc fails and returns NULL
  if(!recordBuf) {
    exit(0);
  }

  // read data from inputFile into buffer
  int readInput = read(fd, recordBuf, fileSize);
  if (readInput == -1) {
    fprintf(stderr, "Error: cannot read file: %s successfully\n", inputFile);
    exit(1);
  }
  
  // close inputFile
  readInput = close(fd);
  if (readInput == -1) {
    fprintf(stderr, "Error: cannot close file: %s successfully\n", inputFile);
    exit(1);
  }

  int arrayLength = fileSize/sizeof(rec_t);
  rec_t *selectedRecords[arrayLength];
  int recordCounter = 0;

  // select records based on specified range and save into array 
  int i;
  for (i = 0; i < arrayLength; i++) {
    if (recordBuf[i].key <= *highValue && recordBuf[i].key >= *lowValue) {
      selectedRecords[recordCounter] = &recordBuf[i];
      recordCounter++;
    }
  }

  // sort the selected records
  qsort(selectedRecords, recordCounter, sizeof(rec_t *), compKeyFunc);

  // open outputFile to store sorted records
  fd = open(outputFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
  if (fd <0) {
    fprintf(stderr, "Error: Cannot open file %s\n", outputFile);
    exit(1);
  }

  // save sorted records into outputFile
  int writeOutput;
  for (i = 0; i < recordCounter; i++) {
    writeOutput = write(fd, selectedRecords[i], sizeof(rec_t));
    if (writeOutput != sizeof(rec_t)) {
      fprintf(stderr, "Error: cannot write file: %s successfully\n", outputFile);
      exit(1);
    }
  }
  
  // close outputFile
  writeOutput = close(fd);
  
  free(recordBuf);

  exit(0);
}

