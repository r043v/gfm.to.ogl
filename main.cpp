#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}


#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 64
#define SCREEN_PX (SCREEN_WIDTH*SCREEN_HEIGHT)

#include "Gfm.cpp"
#include "star.h"

#include <GL/glu.h>

u32 tick;
GLuint tscreen;
u32 * screen;
struct anim * astar;
double angleX = 1, angleZ = 1;

void screenit(){
    static int cpt=0, cptway = 1, wayx=1, wayy=-1, xx=16, yy=48;
    cpt += cptway; if(cpt == 0 || cpt == 255) cptway = -cptway;
    //u32*end = &screen[SCREEN_PX], *p = screen;
    memset(screen,cpt,SCREEN_PX*4);

    xx += wayx;
    yy += wayy;
    
    if(xx < 0 || xx > SCREEN_WIDTH-16) wayx = -wayx;
    if(yy < 0 || yy > SCREEN_HEIGHT-16) wayy = -wayy;    

    playAnim(&astar,xx,yy,0);   
}

void square(int x, int y, int sx, int sy)
{	glBegin(GL_QUADS);
	glColor4f(255, 0, 255, 1);
	glVertex3d(x,y,0);
	glVertex3d(x+sx,y,0);
	glVertex3d(x+sx,y+sy,0);
	glVertex3d(x,y+sy,0);
	glEnd();
}

int main(void)
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "hey joe", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
 
        glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(70,(double)640/480,1,1000);
 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    screen = (u32*)malloc( SCREEN_PX * 4 );
    
    setOutBuffer(screen,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    memset(screen,0,SCREEN_PX*4);
    
    unCrunchGfm(star,starFrmNb);
    
    astar = setAnim(star,20,42,0,0,0,0);
    
    glGenTextures(1,&tscreen);
    glBindTexture(GL_TEXTURE_2D,tscreen);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)screen);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

	tick = (u32)(glfwGetTime()*1000.0f);
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	gluLookAt(2,2,2,0,0,0,0,0,1);
    
	screenit();

	glBindTexture(GL_TEXTURE_2D, tscreen);
	glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)screen);
	
	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1);
	glTexCoord2d(0,1); glVertex3d(1,-1,1);
	glTexCoord2d(0,0); glVertex3d(1,1,1);
	glTexCoord2d(1,0); glVertex3d(-1,1,1);
	glTexCoord2d(1,1); glVertex3d(-1,-1,1);
	glEnd();

	glFlush();
 
       //if(tick < 3000)
	glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}