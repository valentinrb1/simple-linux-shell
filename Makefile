FLAGS = -Wall -Werror -pedantic -g

bin/myShell : obj/myShell.o obj/jobControl.o
	mkdir -p bin
	gcc $(FLAGS) obj/myShell.o obj/jobControl.o -o bin/myShell

obj/myShell.o : inc/myShell.h src/myShell.c
	mkdir -p obj
	gcc $(FLAGS) -c src/myShell.c -o obj/myShell.o

obj/jobControl.o : inc/jobControl.h src/jobControl.c
	gcc $(FLAGS) -c src/jobControl.c -o obj/jobControl.o

.PHONY: clean

clean:
	rm -f -r bin
	rm -f -r obj