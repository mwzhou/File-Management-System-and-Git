all:WTF WTFserver

fileHelperMethods.o: fileHelperMethods.c fileHelperMethods.h
	gcc -lm -Wall -Werror -fsanitize=address -lssl -lcrypto -D_GNU_SOURCE -lpthread -g -c fileHelperMethods.c

structures.o: structures.c structures.h fileHelperMethods.h
	gcc -lm -Wall -Werror -fsanitize=address -lssl -lcrypto -D_GNU_SOURCE -lpthread -g -c structures.c

WTF:client.c client.h fileHelperMethods.o structures.o
	gcc -lm -Wall -Werror -fsanitize=address -lssl -lcrypto -D_GNU_SOURCE -lpthread fileHelperMethods.o structures.o client.c -o WTF

WTFserver:server.c server.h fileHelperMethods.o structures.o
	gcc -lm -Wall -Werror -fsanitize=address -lssl -lcrypto -D_GNU_SOURCE -lpthread fileHelperMethods.o structures.o server.c -o WTFserver

clean:
	rm -f  *.o WTF WTFserver
