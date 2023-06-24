/* Copyright 2023 <Tudor Cristian-Andrei> */
#ifndef UTILS_H_
#define UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* useful macro for handling error codes */
#define DIE(assertion, call_description)                                       \
    do {                                                                       \
        if (assertion) {                                                       \
            fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                 \
            perror(call_description);                                          \
            exit(errno);                                                       \
        }                                                                      \
    } while (0)

/* macro for the maximum number of buckets within a hash */
#define HMAX 100

/* macro for increaseing the size of the arrays */
#define SERVER_INC 10

/*macro for a big value that has no effect to the program */
#define IRELLEVANT 1000000

#endif /* UTILS_H_ */
