# Author: Gurshmeer Singh	
# File Name: Makefile
# Description: Builds and tests the scheduler.

CC = gcc-13
CFLAGS = -std=c11 -Wall -Wextra -pedantic
TARGET = sched
SRC = sched.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET) *.o
	rm -f *.out

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) --policy=FCFS --in="Workload Samples/workload1.txt"
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) --policy=RR --quantum=1 --in="Workload Samples/workload1.txt"
