CFLAGS = -Wall -pedantic -std=c++11 -O2
LIBS = -lSDL2 -lSDL2_image

OBJS = build/main.o build/game.o
TARGET = main

${TARGET}: ${OBJS}
	g++ -o $@ $+ ${LIBS}
 
build/%.o: src/%.cpp
	g++ -c ${CFLAGS} -o $@ $+ 


.PHONY:
	clean
	
clean:
	rm ${OBJS} ${TARGET}

