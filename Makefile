all: quad_main.c
	gcc -o part4 quad_main.c

clean:
	$(RM) all