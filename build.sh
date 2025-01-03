gcc -Wall main.c -o main-game -I/home/matt/raylib-5.0_linux_amd64/include -L /home/matt/raylib-5.0_linux_amd64/lib/ -lraylib -lm
export LD_LIBRARY_PATH=/home/matt/raylib-5.0_linux_amd64/lib
./main-game