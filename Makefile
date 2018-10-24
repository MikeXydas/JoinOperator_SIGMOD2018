CC = gcc
SOURCE = src/hello.c

NAME_OF_EXECUTABLE = exec

OBJECT = $(SOURCE:.c=.o)

VALGRIND_FLAGS = --leak-check=yes --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes -v

all: executable
	@echo Compile finished

executable: $(SOURCE) $(NAME_OF_EXECUTABLE)
	$(CC) -g -O0 -Wall -o  $(NAME_OF_EXECUTABLE) $(SOURCE) -lm -lcunit
 
$(NAME_OF_EXECUTABLE): $(OBJECT)
	$(CC) -g  -O0 $(OBJECT) -o $@ -lm -lcunit


.c.o:
	$(CC) -c $< -o $@ -lm -lcunit


runSearchTest: executable
	./$(NAME_OF_EXECUTABLE)

clean:
	rm -f $(NAME_OF_EXECUTABLE)
	rm -f src/*.o
