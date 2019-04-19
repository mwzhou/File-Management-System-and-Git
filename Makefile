all:WTF WTFserver

WTF:client.c fileHelperMethods.o
	gcc -lm -Wall -Werror -fsanitize=address fileHelperMethods.o client.c -o WTF

WTFserver:server.c fileHelperMethods.o
	gcc -lm -Wall -Werror -fsanitize=address fileHelperMethods.o server.c -o WTFserver

fileHelperMethods.o: fileHelperMethods.c fileHelperMethods.h
	gcc -lm -Wall -Werror -fsanitize=address -g -c fileHelperMethods.c
	
clean:
	rm -f  *.o WTF WTFserver
