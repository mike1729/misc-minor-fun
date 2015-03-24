find : find.o
	ld $^ -s -o $@

find.o : find.asm
	nasm $< -felf64 -o $@

clean :
	rm find find.o *~
