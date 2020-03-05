CC = clang
CFLAGS = -Wall -Wextra -g
OBJS = stop_and_copy.o mark_and_sweep.o alloc.o interp.o minizam.o mlvalues.o parser.o primitives.o domain.o

all: minizam

minizam: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) minizam
