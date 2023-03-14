all: socketstarter

%: %.c
	$(CC) $(CFLAGS) -o $@ $<
