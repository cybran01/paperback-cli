#ifndef ___PAGEGFX_H___
#define ___PAGEGFX_H___

#include <stdbool.h>

// Deals with graphics pertaining to page

int Getprintablewidth(unsigned int, int);

int Getprintableheight (unsigned int, int, int, int);

int Getdatapointraster(int, int);

int Getpointsize(int, int);

int Getprintableaxiscount (unsigned int, int, int, int, int);

// These are settings commonly used through out the paperbak program,
// and were not eliminated in favor of recalculation because of its repeated use
// throughout the program.
struct pagegfx {
    int dx, dy;
    int px, py;
    int nx, ny;
    int width, height;
};

struct pagegfx init_pagegfx(int, 
                            int, 
                            int, 
                            int, 
                            int, 
                            int, 
                            int,
                            bool);
#endif
