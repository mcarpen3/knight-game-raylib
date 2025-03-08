OUT=$(echo $1 | cut -d '.' -f 1)
gcc -Wall -ggdb $1 -o ../build/$OUT \
    ../build/linkedlist.o \
    ../build/doublelinkedlist.o \
    ../build/set.o \
    ../build/mapobject.o \
    ../build/sprite.o \
    -I/home/matt/raylib-5.0_linux_amd64/include -L /home/matt/raylib-5.0_linux_amd64/lib/ -lraylib -lm

export LD_LIBRARY_PATH=/home/matt/raylib-5.0_linux_amd64/lib
../build/$OUT
