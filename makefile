OBJS=main.o owner_memory.o
CFLAGS=-Wall -Wextra

all: main.exe

main.exe: $(OBJS)
	gcc $(CFLAGS) -o $@ $^

%.o: %.c owner_memory.h
	gcc $(CFLAGS) -c $< -o $@

clean:
	del /f /q *.o main.exe
