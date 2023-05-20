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
    Tigr *screen = tigrWindow(640, 360, "Hello", 0);
    float clock, nt, fps = 30.0;

    clock = tigrTime();
    nt = clock + (1 / fps);
    while (!tigrClosed(screen))
    {
        tigrClear(screen, tigrRGB(0x80, 0x90, 0xa0));
        tigrPrint(screen, tfont, 120, 110, tigrRGB(0xff, 0xff, 0xff), "Hello, world.");
        tigrUpdate(screen);
        while (clock < nt) {
            usleep(10000);
            clock += tigrTime();
        }
        nt = clock + (1 / fps);
    }
    tigrFree(screen);
    return 0;
    */
}

