
#include <cstdint>

typedef uint32_t Color32;

void glClear( Color32 color );
void glViewport (int width, int height);

void glReadBuffer (Color32* buffer, int x, int y, int width, int height);

void glClearViewport ();

#define GL_NONE -1
#define GL_LINES 0
#define GL_LINE 1
#define GL_TRIANGLE 2

void glBegin(int mode);
void glEnd();

void glVertex1i(int x);
void glVertex2i(int x, int y);

void glColor(Color32 color);
void swapBuffers();