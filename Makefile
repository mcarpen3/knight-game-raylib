RAYLIB_GCC_ARGS := -I/home/matt/raylib-5.0_linux_amd64/include -L /home/matt/raylib-5.0_linux_amd64/lib/ -lraylib -lm

build/linkedlist.o: Datastructs/linkedlist.c
	gcc -ggdb -c Datastructs/linkedlist.c -o build/linkedlist.o
build/doublelinkedlist.o: Datastructs/doublelinkedlist.c
	gcc -ggdb -c Datastructs/doublelinkedlist.c -o build/doublelinkedlist.o
build/set.o: Datastructs/set.c
	gcc -ggdb -c Datastructs/set.c -o build/set.o
build/sprite.o: Sprite/sprite.c build/linkedlist.o
	gcc -ggdb -c Sprite/sprite.c -o build/sprite.o $(RAYLIB_GCC_ARGS)
build/mapobject.o: Object/mapobject.c build/linkedlist.o build/doublelinkedlist.o build/set.o build/sprite.o
	gcc -ggdb -c Object/mapobject.c -o build/mapobject.o $(RAYLIB_GCC_ARGS)
build/TextureSource.o: World/TextureSource.c build/mapobject.o
	gcc -ggdb -c World/TextureSource.c -o build/TextureSource.o $(RAYLIB_GCC_ARGS)
build/LoadMap.o: Map/LoadMap.c build/mapobject.o
	gcc -ggdb -c Map/LoadMap.c -o build/LoadMap.o $(RAYLIB_GCC_ARGS)
build/main-game: main.c build/linkedlist.o build/doublelinkedlist.o build/set.o build/sprite.o build/mapobject.o build/LoadMap.o build/TextureSource.o
	gcc -Wall -ggdb main.c build/LoadMap.o build/sprite.o build/mapobject.o \
	build/linkedlist.o build/doublelinkedlist.o build/set.o build/TextureSource.o \
	 -o build/main-game \
	$(RAYLIB_GCC_ARGS)
build/MapEditor: Map/MapEditor.c build/linkedlist.o build/doublelinkedlist.o build/set.o build/sprite.o build/mapobject.o
	gcc -Wall -Wextra -ggdb Map/MapEditor.c build/linkedlist.o \
	build/doublelinkedlist.o build/mapobject.o build/set.o build/sprite.o \
	-o build/MapEditor \
	$(RAYLIB_GCC_ARGS) && \
	export LD_LIBRARY_PATH=/home/matt/raylib-5.0_linux_amd64/lib &&\
	build/MapEditor
runmapeditor: build/MapEditor
	export LD_LIBRARY_PATH=/home/matt/raylib-5.0_linux_amd64/lib && build/MapEditor
rungame: build/main-game
	export LD_LIBRARY_PATH=/home/matt/raylib-5.0_linux_amd64/lib && build/main-game
clean:
	rm build/*