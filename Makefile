mklisp: mklisp.o parser.o
	$(CC) -o $@ $^

clean:
	rm -f *.o
	rm -f mklisp

.PHONY: clean

%.o: %.c
	$(CC) -c -O3 $<
