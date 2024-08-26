all: cmd.elf

test.c: test.pnml
	python pnmlgen.py $< test

cmd.elf: main.c test.c
	$(CC) -I ../ -o $@ main.c test.c ../petri.o -lubsan

clean:
	$(RM) cmd.elf test.c test.h
