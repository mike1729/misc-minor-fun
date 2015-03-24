SRC_H := $(wildcard expr*.h)
SRC_C := $(wildcard expr*.c)
SRC_X := $(wildcard expr*.cpp)
SRC_A := $(wildcard expr*.asm)

OBJ_C := $(patsubst %,%.o,${SRC_C})
OBJ_X := $(patsubst %,%.o,${SRC_X})
OBJ_A := $(patsubst %,%.o,${SRC_A})

PRG_L := expr.so
PRG_S := expr-static
PRG_D := expr

all : ${PRG_S} ${PRG_D}

${PRG_S} : main.o ${OBJ_C} ${OBJ_X} ${OBJ_A}
	g++ -static $^ -lm -o $@

${PRG_D} : main.o ${PRG_L}
	g++ $^ -lm -o $@

${PRG_L} : ${OBJ_C} ${OBJ_X} ${OBJ_A}
	ld -shared $^ -lm -o $@

${OBJ_C} : %.o : % ${SRC_H}
	gcc -std=c99 -c $< -fPIC -O2 -o $@
	objcopy -N expr_eval $@

${OBJ_X} : %.o : % ${SRC_H}
	g++ -std=c++0x -c $< -fPIC -O2 -o $@
	objcopy -N expr_eval $@

${OBJ_A} : %.o : %
	nasm $< -felf64 -o $@

main.o : main.c ${SRC_H}
	gcc -std=c99 -c $< -O2 -o $@

clean :
	rm -f ${PRG_S} ${PRG_D} ${PRG_L} *.o *~

.PHONY : all clean

.SUFFIXES :
