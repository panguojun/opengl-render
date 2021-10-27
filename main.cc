#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <GL/glut.h>

// *****************************************************
// glRender
// *****************************************************
#define GAP 10
#define ROWS 1
#define COLS 1

#define OPENGL_WIDTH 48
#define OPENGL_HEIGHT 13

GLenum rgb, doubleBuffer;
GLint windW = 1920, windH = 1080;

GLint boxW, boxH;
static void Viewport(GLint row, GLint column)
{
    GLint x, y;

    boxW = (windW - (COLS + 1) * GAP) / COLS;
    boxH = (windH - (ROWS + 1) * GAP) / ROWS;

    x = GAP + column * (boxW + GAP);
    y = GAP + row * (boxH + GAP);

    glViewport(x, y, boxW, boxH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10, 10, -10, 10, -10, 10);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, boxW, boxH);
}

// ******************************************************
#pragma comment(lib, "libPMDLL")
extern "C"
{
    bool getStack(void** _estack);
    int _stdcall VB_dopmhg(const char* script);
}
// ******************************************************
#define vec vec3
#define real float
#define VECLIST	std::vector<vertex>
inline float _MIN(float a, float b) { return ((a) < (b) ? (a) : (b)); }
struct vec3 { float x = 0; float y = 0; float z = 0; };
struct vec2 { float x = 0; float y = 0;};
struct vertex
{
    vec3 p;
    vec3 n;
    vec2 uv;

    int cor = 0;
    int ind = -1;

    int tag = 0;

    // deform
    real deformwgt = 0;
    vec3 bone;

};

static void triang(vertex& p1, vertex& p2, vertex& p3)
{
    glVertex3f(p1.p.x, p1.p.y , p1.p.z);
    glVertex3f(p2.p.x, p2.p.y , p2.p.z);
    glVertex3f(p3.p.x, p3.p.y , p3.p.z);
}
void plane(vertex& p1, vertex& p2, vertex& p3, vertex& p4)
{
    triang(p2, p3, p1);
    triang(p4, p1, p3);
}

static void face(VECLIST& edge1, VECLIST& edge2)
{
    int len = _MIN(edge1.size(), edge2.size());
    len = int(len - 1);
    for (int i = 0; i < len; i++)
    {
        plane(edge1[i], edge1[i + 1], edge2[i + 1], edge2[i]);
    }
}
static void triangles()
{
    glBegin(GL_TRIANGLES);
    std::vector<VECLIST>* pestack;
    getStack((void**)&pestack);
    for (int i = 1; i < pestack->size(); i++)
    {
        face((*pestack)[i - 1], (*pestack)[i]);
    }
    glEnd();
}

static void lineloop(void)
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    std::vector<VECLIST>* pestack;
    getStack((void**)&pestack);
    for (int i = 0; i < pestack->size(); i++)
    {
        glBegin(GL_LINE_LOOP);
        for (auto& it : (*pestack)[i])
        {
            glVertex3f(it.p.x, it.p.y, it.p.z);
        }
        glEnd();
    }
}

static void points(void)
{
    glEnable(GL_POINT_SMOOTH);
    glPointSize(10.0); glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

    glBegin(GL_POINTS);
    std::vector<VECLIST>* pestack;
    getStack((void**)&pestack);
    for (int i = 0; i < pestack->size(); i++)
    {
        for (auto& it : (*pestack)[i])
        {
            glVertex3f(it.p.x, it.p.y, it.p.z);
        }
    }
    glEnd();
}

// *****************************************************
// Input
// *****************************************************
bool mouseLeftDown=0;
bool mouseRightDown = 0;
float mouseX = 0, mouseY = 0;
float cameraDistance = 0;
float cameraAngleX = 0;
float cameraAngleY = 0;
void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if (state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if (state == GLUT_UP)
            mouseRightDown = false;
    }
}

void mouseMotionCB(int x, int y)
{
    if (mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if (mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }

    glutPostRedisplay();
}

// *****************************************************
// Draw
// *****************************************************
static void Draw(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

    glTranslatef(0, 0, cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);

    Viewport(0, 0);

    //triangles();
    lineloop();
    points();

    glPopMatrix();

    if (doubleBuffer) {
        glutSwapBuffers();
    }
    else {
        glFlush();
    }
}

// *****************************************************
// main
// *****************************************************
int main(int argc, char** argv)
{
    //VB_dopmhg("C:\\Users\\18858\\Documents\\LAB\\ZEXE\\main.e");

    GLenum type;

    glutInit(&argc, argv);
    rgb = GL_TRUE;
    doubleBuffer = GL_FALSE;

    type = (rgb) ? GLUT_RGB : GLUT_INDEX;
    type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;
    glutInitDisplayMode(type);
    glutInitWindowSize(windW, windH);
    glutCreateWindow("glRender");

    //Init();

    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);
    glutDisplayFunc(Draw);
    glutMainLoop();
}
