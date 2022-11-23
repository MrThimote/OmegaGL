
#include <iostream>
#include <omegagl.h>
#include <assert.h>
#include <vector>
#include <queue>

struct PixelData {
    Color32 color;
    int last_rendered;

    void setColor (Color32 color, int render) {
        this->color = color;
        last_rendered = render;
    }
};

struct FaceStarter {
    Color32 color; int x; int y; int renderId;

    FaceStarter(Color32 p_color, int p_x, int p_y, int p_renderId) 
    : color(p_color), x(p_x), y(p_y), renderId(p_renderId) {};
};

PixelData* glViewportData = nullptr;
int glViewportWidth;
int glViewportHeight;

int glRenderId;

PixelData* get (int x, int y) {
    return glViewportData + y * glViewportHeight + x;
}
PixelData* get_opposite (int x, int y) {
    return get(y, x);
}

void _drawLine (PixelData* (*get)(int, int), Color32 color, int x1, int y1, int x2, int y2, int renderId) {
  if (x2 < x1) {
    _drawLine(get, color, x2, y2, x1, y1, renderId);
    return ;
  }
  
  int x; int y; int dx; int dy;
  float e; float e_10; float e_01;

  dy = y2 - y1;
  dx = x2 - x1;
  y = y1;
  e = 0.0;
  e_10 = ((float)dy) / ((float)dx);
  e_01 = -1.0;
  for (x = x1; x <= x2; x ++) {
    get(x, y)->setColor(color, renderId);
    if ((e = e + e_10) >= 0.5) {  // erreur pour le pixel suivant de même rangée
      y = y + 1;  // choisir plutôt le pixel suivant dans la rangée supérieure
      e = e + e_01;  // ajuste l’erreur commise dans cette nouvelle rangée
    }
  }
}
void drawLine (Color32 color, int x1, int y1, int x2, int y2, int renderId) {
    if (abs(x1 - x2) < abs(y1 - y2)) {
        _drawLine(get_opposite, color, y1, x1, y2, x2, renderId);
    } else {
        _drawLine(get, color, x1, y1, x2, y2, renderId);
    }
}

void fillFace(FaceStarter face) {
    std::queue<int> queue_x;
    std::queue<int> queue_y;

    queue_x.push(face.x);
    queue_y.push(face.y);

    while (queue_x.size() != 0) {
        int x = queue_x.front(); queue_x.pop();
        int y = queue_y.front(); queue_y.pop();

        if (get(x, y)->last_rendered >= face.renderId) continue;
        get(x, y)->last_rendered = face.renderId;
        get(x, y)->color = face.color;

        if (x + 1 < glViewportWidth) {
            queue_x.push(x + 1);
            queue_y.push(y);
        }
        if (x - 1 >= 0) {
            queue_x.push(x - 1);
            queue_y.push(y);
        }
        if (y + 1 < glViewportHeight) {
            queue_x.push(x);
            queue_y.push(y + 1);
        }
        if (y - 1 >= 0) {
            queue_x.push(x);
            queue_y.push(y - 1);
        }
    }
}

/**
 * I/O
 */

void glClear(Color32 color) {
    for (int glX = 0; glX < glViewportWidth; glX ++) {
        for (int glY = 0; glY < glViewportHeight; glY ++) {
            get(glX, glY)->color = color;
        }
    }
}

void glViewport (int width, int height) {
    printf("Creating viewport %d/%d\n", width, height);

    glViewportWidth  = width;
    glViewportHeight = height;

    if (glViewportData != nullptr) free( glViewportData );

    glViewportData = (PixelData*) malloc(sizeof(PixelData) * width * height);
}

void glReadBuffer (Color32* buffer, int x, int y, int width, int height) {
    for (int dy = 0; dy < height; dy ++) {
        for (int dx = 0; dx < width; dx ++) {
            buffer[dy * height + dx] = get(x + dx, y + dy)->color;
        }
    }
}

void glClearViewport () {
    if (glViewportData == nullptr) return ;

    free(glViewportData);

    glViewportData = nullptr;
}




/**
 * Rendering system
 */
bool glBegan = false;
int draw_mode = GL_NONE;
int* vertex_buffer;
int  vertex_index;

Color32 draw_color = 0xFF000000;

std::vector<FaceStarter> faces;

void glColor (Color32 color) {
    draw_color = color;
}

void glBegin (int mode) {
    assert(!glBegan);
    glBegan = true;

    if (mode == GL_LINE) {
        vertex_buffer = (int*) malloc(4 * sizeof(int));
        vertex_index  = 0;
    }
    if (mode == GL_LINES) {
        vertex_buffer = (int*) malloc(64 * sizeof(int));
        vertex_index  = 0; 
    }
    if (mode == GL_TRIANGLE) {
        vertex_buffer = (int*) malloc(6 * sizeof(int));
        vertex_index = 0;
    }

    draw_mode = mode;
}

void glEnd () {
    assert (glBegan);
    glBegan = false;

    int renderId = glRenderId ++;

    if (draw_mode == GL_LINE) {
        drawLine(
            draw_color, 
            vertex_buffer[0], 
            vertex_buffer[1], 
            vertex_buffer[2],
            vertex_buffer[3],
            renderId
        );
    }
    if (draw_mode == GL_LINES) {
        for (int id = 0; id + 2 < vertex_index; id += 2) {
            drawLine(
                draw_color, 
                vertex_buffer[id + 0], 
                vertex_buffer[id + 1], 
                vertex_buffer[id + 2], 
                vertex_buffer[id + 3],
                renderId
            );
        }
    }
    if (draw_mode == GL_TRIANGLE) {
        for (int id = 0; id + 2 < vertex_index; id += 2) {
            drawLine(
                draw_color, 
                vertex_buffer[id + 0], 
                vertex_buffer[id + 1], 
                vertex_buffer[id + 2], 
                vertex_buffer[id + 3],
                renderId
            );
        }
        drawLine(
            draw_color, 
            vertex_buffer[0], 
            vertex_buffer[1], 
            vertex_buffer[4], 
            vertex_buffer[5],
            renderId
        );

        int bx = vertex_buffer[0] + vertex_buffer[2] + vertex_buffer[4];
        int by = vertex_buffer[1] + vertex_buffer[3] + vertex_buffer[5];
        fillFace(FaceStarter(draw_color, bx / 3, by / 3, renderId));
    }

    free(vertex_buffer);
}



void swapBuffers () {
    // We don't swap buffer here, but we implement the filling of faces

    for (int idFaceStarter = faces.size() - 1; idFaceStarter >= 0; idFaceStarter --) {
        fillFace(faces[idFaceStarter]);
    }

    faces.clear();
}

void glVertex1i(int x) {
    vertex_buffer[vertex_index ++] = x;
}
void glVertex2i (int x, int y) {
    glVertex1i(x);
    glVertex1i(y);
}
