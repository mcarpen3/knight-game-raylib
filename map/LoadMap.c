// Map.bin is default file
// Order of saved entities:
// 1. Rectangle: world,
// 2. size_t: strlen(objname)
// 3. char *: objname
// 4. int: objtype {Player, Enemy, Item, Projectile, Env}
// 5. Vector2: position
// 6. float: rotation
// 7. float: scale
// 8. int: sprites->size
// 9. foreach sprite
//   a. size_t: strlen(spritename)
//   b. char *: spritename
//   c. size_t: strlen(filename)
//   d. char *: filename
//   e. int: frameCount
//   f. Rectangle: spriteRect
//   g. foreach collider
//     i. int: collider type {RectType, LineType}
//     ii. Rectangle | Line

#include<stdio.h>
#include<stdlib.h>
#include<raylib.h>
#include"../datastructs/doublelinkedlist.h"
#include"../datastructs/linkedlist.h"

int main(void) {
    printf("Hello World\n");
    const char *infile = "Map.bin";
    FILE *in = fopen(infile, "r");
    if (in == NULL) {
        fprintf(stderr, "ERROR: Failed to load file %s\n", infile);
        return 1;
    }
    Rectangle world = {0};
    fread(&world, sizeof(Rectangle), 1, in);
    printf("Rectangle: [%f,%f,%f,%f]", world.x, world.y, world.width, world.height);
    return 0;
}