
CC=gcc
CFLAGS=-g -std=c99
LIBS=-lm -lGL -lGLU -lglut

prog: main.o matrix.o
	${CC} ${LIBS} -o prog main.o matrix.o
