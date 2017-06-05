CFLAGS = -Wall -Werror -Wextra -Wpedantic -std=c99 -O2
CFLAGS_PTHREAD = -pthread -Wall -Werror -Wextra -Wpedantic -std=c99 -O2
GCC = gcc
CPP = g++

# Since this is the first target, it will be executed by default.
.PHONY: all
all: Task1 Task2 #Task3 Task4 #include as many as you've done


#PHONY's are just to ensure, that the rule 'clean' is used, rather then the file 'clean'
.PHONY: clean
clean:
	rm -rf ./Task1 ./Task2
	clear

#Here comes the tasks
Task1:
	$(GCC) $(CFLAGS_PTHREAD) Exercise1/Task1.c -o $@

Task2:
	$(CPP) -pthread Exercise2/Task2.cpp -o $@

Task3:
	$(GCC) $(CFLAGS) Exercise3/Task3.c -o $@

Task4:
	$(GCC) $(CFLAGS) Exercise4/Task4.c -o $@

