all : sort

test : sort test.in test.sorted
	./sort < test.in > test.out
	diff -w test.out test.sorted

sort : main.o sort.o
	g++ $^ -o $@

main.o : main.cpp
	g++ -c $^ -o $@

sort.o : sort.asm
	nasm $^ -felf64 -o $@

clean :
	rm -f sort sort.o main.o test.out *~

.PHONY : all test clean
