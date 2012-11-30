
CC=gcc
CFLAGS=-g -Wall -std=c99 -fms-extensions
LIBS=-lm -lGL -lGLU -lglut -lpng
#fms-extensions allows anonymous structures and unions
#I must have this magick!

prog: main.o matrix.o pngloader.o
	${CC} ${LIBS} -o prog main.o matrix.o pngloader.o
