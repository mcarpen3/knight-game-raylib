OUT=$(echo $1 | cut -d '.' -f 1)
gcc -Wall $1 -o $OUT ../datastructs/linkedlist.c -I/home/matt/raylib-5.0_linux_amd64/include -L /home/matt/raylib-5.0_linux_amd64/lib/ -lraylib -lm
export LD_LIBRARY_PATH=/home/matt/raylib-5.0_linux_amd64/lib
./$OUT