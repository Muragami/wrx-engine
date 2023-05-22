/*
    wrx-engine: the engine

    Jason A. Petrasko, muragami, muragami@wishray.com 2023

    MIT License
*/

#include "wrx.h"

int main(int argc, char *argv[])
{
    wrxState *ps = wrxNewState();
    char *arg;

    if (argc > 1) arg = argv[1];
     else {
        printf("wrx:: no argument passed to the engine, nothing to do.");
        return -1;
     }

    printf("wrx:: state created\n");
    if (WRX_ERROR(wrxStart(ps, arg))) {
        return -1;
    }

    printf("wrx:: state started\n");

    while (wrxRunning(ps)) {
        if (WRX_ERROR(wrxUpdate(ps))) {
            break;
        }
    }

    return 0;

/*
    printf("init!\n");
    Tigr *screen = tigrMainWindow(640, 360, "Hello", 0);
    float clock, nt, fps = 30.0;

    printf("ok!\n");
    clock = tigrTime();
    nt = clock + (1 / fps);
    printf("started!\n");

    Tigr *twin = tigrWindow(128, 128);
    tigrClear(twin, tigrRGBA(255, 0, 0, 192));

    int px = 120;
    int py = 110;
    while (!tigrClosed(screen))
    {
        tigrClear(screen, tigrRGB(0x80, 0x90, 0xa0));
        tigrChange(screen);
        tigrPrint(screen, tfont, px, py, tigrRGB(0xff, 0xff, 0xff), "Hello, world.");
        tigrUpdate(screen);
        while (clock < nt) {
            usleep(10000);
            clock += tigrTime();
        }
        nt = clock + (1 / fps);
        px += 1;
        py += 1;
    }
    printf("ended!\n");
    tigrFree(screen);
    return 0;
 */   
}

