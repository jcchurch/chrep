CC = gcc
CCGLAGS = -Wall

chrep: src/chrep.c
	$(CC) src/chrep.c -o chrep
