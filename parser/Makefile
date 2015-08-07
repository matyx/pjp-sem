CC=g++
CFLAGS=-c -g -Wall -pedantic -D __DEBUG__
LDFLAGS=
SOURCES=test-parser.cpp lexan.cpp parser.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=lexan

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f lexan
	rm -f *.o

run:
	valgrind --main-stacksize=134217728 ./lexan factorization.p