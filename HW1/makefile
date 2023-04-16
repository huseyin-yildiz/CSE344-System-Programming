SHELL = bash -O extglob

main: appendMeMore dup dup2 dupped_fd_verifier

appendMeMore:clean
	$(CC) appendMeMore.c -o appendMeMore


dup:clean
	$(CC) dup.c -o dup


dup2:clean
	$(CC) dup2.c -o dup2


dupped_fd_verifier:clean
	$(CC) dupped_fd_verifier.c -o dupped_fd_verifier


clean:
	${RM} !(*.c|*.h|*.pdf|makefile)

