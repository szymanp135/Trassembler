
.PHONY: clean all
all: trassembler

trassembler: trassembler.c
	gcc -std=gnu99 -Wall trassembler.c -o trassembler

old: .trassembler.c.old
	gcc -std=gnu99 -Wall .trassembler.c.old -o trassembler

clean:
	rm trassembler trassembler
