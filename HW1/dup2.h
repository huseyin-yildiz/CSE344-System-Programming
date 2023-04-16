# include <fcntl.h>
# include <errno.h>
# include <unistd.h>
# include <assert.h>
# include <stdio.h>

int mydup2(int oldfd, int newfd);