
#include "omegagl.h"

#include <iostream>

#define OUTPUT_PATH "./out.ppm"

#define MASK_RED 0x0000FF
#define MASK_GRE 0x00FF00
#define MASK_BLU 0xFF0000

void write (const char* path, Color32* buffer, int width, int height) {
    FILE* f = fopen(path, "wb");
    fprintf(f, "P6\n%d %d 255\n", width, height);

    for (size_t y = 0; y < height; y ++) {
        for (size_t x = 0; x < width; x ++) {
            Color32 pixel = buffer[y * height + x];

            uint8_t bytes[3] = {
                (pixel & MASK_RED)>>8*0,
                (pixel & MASK_GRE)>>8*1,
                (pixel & MASK_BLU)>>8*2,
            };
            
            fwrite(bytes, sizeof(bytes), 1, f);
        }
    }

    fclose(f);
}

int main () {
    glViewport(100, 100);
    glClear(0xFF181818);

    glColor(0xFFFF0000);
    glBegin(GL_LINE);
    glVertex2i(99, 50);
    glVertex2i(0, 0);
    glEnd();
    glColor(0xFF00FF00);
    glBegin(GL_TRIANGLE);
    glVertex2i(0, 10);
    glVertex2i(50, 10);
    glVertex2i(50, 50);
    glEnd();
    glColor(0xFF0000FF);
    glBegin(GL_TRIANGLE);
    glVertex2i(10, 10);
    glVertex2i(60, 10);
    glVertex2i(60, 50);
    glEnd();

    swapBuffers();

    Color32 buffer[10000];
    glReadBuffer(buffer, 0, 0, 100, 100);

    write(OUTPUT_PATH, buffer, 100, 100);
    
    glClearViewport();
}
