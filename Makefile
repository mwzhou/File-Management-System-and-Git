all:WTF WTFserver test

fileHelperMethods.o: fileHelperMethods.c fileHelperMethods.h
	gcc -lm -Wall -Werror -fsanitize=address -g -c fileHelperMethods.c

WTF:client.c client.h fileHelperMethods.o
	gcc -lm -Wall -Werror -fsanitize=address fileHelperMethods.o client.c -o WTF

WTFserver:server.c server.h fileHelperMethods.o
	gcc -lm -Wall -Werror -fsanitize=address fileHelperMethods.o server.c -o WTFserver

test: test.c fileHelperMethods.o #TODO delete!!!
	gcc -lm -Wall -Werror -fsanitize=address fileHelperMethods.o  test.c -o test

clean:
	rm -f  *.o WTF WTFserver test
