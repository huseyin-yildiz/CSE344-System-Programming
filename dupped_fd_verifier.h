# include "dup.h"
# include "dup2.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void test_duplicated(int fd, int newfd);
void readFile(char *filename);
