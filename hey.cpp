#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "sdlglutils.h"
#include <SDL/SDL_image.h>
#include <string.h>
#include <stdbool.h>

#include "Gfm.cpp"
#include "star.h"

SDL_Surface * flipSurface(SDL_Surface * surface);

GLuint loadTexture(const char * filename,bool useMipMap)
{
    GLuint glID;
    SDL_Surface * picture_surface = NULL;
    SDL_Surface *gl_surface = NULL;
    SDL_Surface * gl_fliped_surface = NULL;
    Uint32 rmask, gmask, bmask, amask;

    picture_surface = IMG_Load(filename);
    if (picture_surface == NULL)
        return 0;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN

    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else

    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_PixelFormat format = *(picture_surface->format);
    format.BitsPerPixel = 32;
    format.BytesPerPixel = 4;
    format.Rmask = rmask;
    format.Gmask = gmask;
    format.Bmask = bmask;
    format.Amask = amask;

    gl_surface = SDL_ConvertSurface(picture_surface,&format,SDL_SWSURFACE);

    gl_fliped_surface = flipSurface(gl_surface);

    glGenTextures(1, &glID);

    glBindTexture(GL_TEXTURE_2D, glID);


    if (useMipMap)
    {

        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, gl_fliped_surface->w,
                          gl_fliped_surface->h, GL_RGBA,GL_UNSIGNED_BYTE,
                          gl_fliped_surface->pixels);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);

    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, 4, gl_fliped_surface->w,
                     gl_fliped_surface->h, 0, GL_RGBA,GL_UNSIGNED_BYTE,
                     gl_fliped_surface->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);


    SDL_FreeSurface(gl_fliped_surface);
    SDL_FreeSurface(gl_surface);
    SDL_FreeSurface(picture_surface);

    return glID;
}

SDL_Surface * flipSurface(SDL_Surface * surface)
{
    int current_line,pitch;
    SDL_Surface * fliped_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   surface->w,surface->h,
                                   surface->format->BitsPerPixel,
                                   surface->format->Rmask,
                                   surface->format->Gmask,
                                   surface->format->Bmask,
                                   surface->format->Amask);



    SDL_LockSurface(surface);
    SDL_LockSurface(fliped_surface);

    pitch = surface->pitch;
    for (current_line = 0; current_line < surface->h; current_line ++)
    {
        memcpy(&((unsigned char* )fliped_surface->pixels)[current_line*pitch],
               &((unsigned char* )surface->pixels)[(surface->h - 1  -
                                                    current_line)*pitch],
               pitch);
    }

    SDL_UnlockSurface(fliped_surface);
    SDL_UnlockSurface(surface);
    return fliped_surface;
}

GLuint hey, hey2, hey3;

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 64
#define SCREEN_PX SCREEN_WIDTH*SCREEN_HEIGHT

double angleX = 1, angleZ = 1;

u32 *screen, tick;

struct anim * astar;

void Dessiner(void);

void square(int x, int y, int sx, int sy)
{	glBegin(GL_QUADS);
	glColor4f(255, 1, 1, 1);
	glVertex3d(x,y,0);
	glVertex3d(x+sx,y,0);
	glVertex3d(x,y+sy,0);
	glVertex3d(x+sx,y+sy,0);
	glEnd();
}

int main(int argc, char *argv[])
{
    SDL_Event event;
 
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);
    SDL_WM_SetCaption("hey", NULL);
    SDL_SetVideoMode(640, 480, 32, SDL_OPENGL);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(70,(double)640/480,1,1000);
 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    hey = loadTexture("test.jpg",true);
    hey2 = loadTexture("test2.jpg",true);
    
    Uint32 last_time = SDL_GetTicks();
    Uint32 current_time,ellapsed_time;
    Uint32 start_time;
 
    glGenTextures(1,&hey3);
    glBindTexture(GL_TEXTURE_2D,hey3);

    screen = (u32*)malloc(SCREEN_WIDTH*SCREEN_HEIGHT*4);
    
    setOutBuffer(screen,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    memset(screen,0,SCREEN_WIDTH*SCREEN_HEIGHT*4);
    
    unCrunchGfm(star,starFrmNb);
    
    astar = setAnim(star,20,42,0,0,0,0);
    
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)screen);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    Dessiner();
    
    for (;;)
    {
        start_time = SDL_GetTicks();
        while (SDL_PollEvent(&event))
        {
 
            switch(event.type)
            {
                case SDL_QUIT:
                exit(0);
                break;
            }
        }
 
        current_time = SDL_GetTicks();
	tick = current_time;
        ellapsed_time = current_time - last_time;
        last_time = current_time;
 
        angleZ += 0.05 * ellapsed_time;
        angleX += 0.05 * ellapsed_time;

        Dessiner();
 
	
        ellapsed_time = SDL_GetTicks() - start_time;
        if (ellapsed_time < 10)
        {
            SDL_Delay(10 - ellapsed_time);
        }
 
    }
 
    return 0;
}

int cpt=0, cptway = 1;

int wayx=1, wayy=-1, xx=16, yy=48;

void Dessiner()
{
      cpt += cptway;
    if(cpt == 0 || cpt == 255) cptway = -cptway;
    u32*end = &screen[SCREEN_PX];
    u32*p = screen;
//    while(p < end){ *p++ = 0x00ff00ff;
	      /*char *pp = (char*)p++;
	      *pp++ = 0xff;
	      *pp++ = 0;
	      *pp++ = 0;
	      *pp++ = 0;*/
  //  }
    memset(screen,cpt,SCREEN_WIDTH*SCREEN_HEIGHT*4);

    xx += wayx;
    yy += wayy;
    
    if(xx < 0 || xx > SCREEN_WIDTH-16) wayx = -wayx;
    if(yy < 0 || yy > SCREEN_HEIGHT-16) wayy = -wayy;    

    playAnim(&astar,xx,yy,0);
    /*drawAnim(&astar,8,8,0);
    drawAnim(&astar,16,16,0);*/
  
  
  
  
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
 
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
 
    gluLookAt(2,2,2,0,0,0,0,0,1);
 
    glRotated(angleZ,0,0,1);
    glRotated(angleX,1,0,0);
 
   // glBegin(GL_QUADS);
 
    glBindTexture(GL_TEXTURE_2D, hey);
    glBegin(GL_QUADS);
    glColor4f(1, 1, 1, 1);
    glTexCoord2d(0,1);  glVertex3d(1,1,1);
    glTexCoord2d(0,0);  glVertex3d(1,1,-1);
    glTexCoord2d(1,0);  glVertex3d(-1,1,-1);
    glTexCoord2d(1,1);  glVertex3d(-1,1,1);
    //glEnd();
    /*
    glColor3ub(255,0,0); //face rouge
    glVertex3d(1,1,1);
    glVertex3d(1,1,-1);
    glVertex3d(-1,1,-1);
    glVertex3d(-1,1,1);
 */
    glEnd();
    glBindTexture(GL_TEXTURE_2D, hey2);
    glBegin(GL_QUADS);
 //   glColor4f(1, 1, 1, 1);
    glTexCoord2d(0,3); glVertex3d(1,-1,1);
    glTexCoord2d(0,0); glVertex3d(1,-1,-1);
    glTexCoord2d(3,0); glVertex3d(1,1,-1);
    glTexCoord2d(3,3); glVertex3d(1,1,1);
 
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, 0);
   
    glBegin(GL_QUADS);
    glColor3ub(0,0,255); //face bleue
    glVertex3d(-1,-1,1);
    glVertex3d(-1,-1,-1);
    glVertex3d(1,-1,-1);
    glVertex3d(1,-1,1);
 
    glColor3ub(255,255,0); //face jaune
    glVertex3d(-1,1,1);
    glVertex3d(-1,1,-1);
    glVertex3d(-1,-1,-1);
    glVertex3d(-1,-1,1);
 
    glColor3ub(0,255,255); //face cyan
    glVertex3d(1,1,-1);
    glVertex3d(1,-1,-1);
    glVertex3d(-1,-1,-1);
    glVertex3d(-1,1,-1);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, hey3);
    glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)screen);

    
    glBegin(GL_QUADS);
    glColor4f(1, 1, 1, 1);
    glTexCoord2d(0,1); glVertex3d(1,-1,1);
    glTexCoord2d(0,0); glVertex3d(1,1,1);
    glTexCoord2d(1,0); glVertex3d(-1,1,1);
    glTexCoord2d(1,1); glVertex3d(-1,-1,1);
    glEnd();

    square(1,1,1,1);
    square(2,2,1,1);
    
    glFlush();
    SDL_GL_SwapBuffers();
}