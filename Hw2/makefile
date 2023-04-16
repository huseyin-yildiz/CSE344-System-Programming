SHELL = bash -O extglob

main: emulator

emulator:clean
	$(CC) terminal_emulator.c -o terminal_emulator


clean:
	${RM} !(*.c|*.h|*.pdf|makefile|test_commands.txt|logs)

