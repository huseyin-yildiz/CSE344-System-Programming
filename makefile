main: appendMeMore dup dup2


appendMeMore:clean
	$(CC) appendMeMore.c -o appendMeMore


dup:clean
	$(CC) dup.c -o dup


dup2:clean
	$(CC) dup2.c -o dup2


dupped_fd_verifier:clean



clean:
	${RM} appendMeMore dup dup2 dupped_fd_verifier
