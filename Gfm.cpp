
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define  u8 unsigned  char
#define  s8  signed   char
#define u16 unsigned short
#define s16  signed  short
#define u32 unsigned  int
#define s32  signed   int

#define clDeep     u32
#define clDeepDec  2
#define clDeepSze  4
#define deepByte   32

extern u32 tick;

clDeep * data2Gfm(unsigned char *data);
void unCrunchGfm(clDeep ** gfm, u32 frmNb);
void drawGfm(clDeep *Gfm, int x, int y);
void udrawGfm(clDeep *Gfm,clDeep *scr);

// blit zone definition routines ..

// delimate current blit zone
void   setBlitLimit(u32 x, u32 y, u32 x1, u32 x2);
void setBlitLimit(clDeep *start, clDeep *end, u32 widthStart, u32 widthEnd);

// save or load a blit zone
void  saveBlitLimit(void); // save current blit limit
void  loadBlitLimit(void); // retreve saved blit limit

void internalLoadBlitLimit(void);
void internalSaveBlitLimit(void);

void setOutBuffer(clDeep*bf,int w, int h);

// maximize blit zone
void fullBlitLimit(void); // define blit limit as full screen

	#define xy2scr(x,y)		(&pixel[(x)+(y)*WIDTH])
	#define getGfmSize(x,y,gfm)	if(x)*(x)=(((gfm)[2])>>16);if(y)*(y)=(((gfm)[2])&0xffff)

//#include "../../../Gdl.h"

clDeep *scrStart,*scrEnd,*pixel ;
int frmWidth, frmWidthStart, frmWidthEnd, bufWidth, bufHeight;

clDeep *saved_scrStart=0, *saved_scrEnd=0, *saved_screen=0 ;
u32 saved_frmWidth=0, saved_frmWidthStart=0, saved_frmWidthEnd=0;

clDeep *isaved_scrStart=0, *isaved_scrEnd=0, *isaved_screen=0 ;
u32 isaved_frmWidth=0, isaved_frmWidthStart=0, isaved_frmWidthEnd=0;


void setOutBuffer(clDeep*bf,int w, int h)
{	pixel = bf;
	bufWidth = w;
	bufHeight = h;
	fullBlitLimit();
}

void fullBlitLimit(void)
{	scrStart  = pixel;
	scrEnd    = &pixel[bufWidth*bufHeight-1];
	frmWidth  = bufWidth;
	frmWidthStart = 0;
	frmWidthEnd   = bufWidth;
}

void saveBlitLimit(void)
{	saved_scrStart  = scrStart;
	saved_scrEnd    = scrEnd;
	saved_frmWidth  = frmWidth;
	saved_frmWidthStart = frmWidthStart;
	saved_frmWidthEnd   = frmWidthEnd;
}

void loadBlitLimit(void)
{	scrStart  = saved_scrStart;
	scrEnd    = saved_scrEnd;
	frmWidth  = saved_frmWidth;
	frmWidthStart = saved_frmWidthStart;
	frmWidthEnd   = saved_frmWidthEnd;
}

void internalSaveBlitLimit(void)
{	isaved_scrStart  = scrStart;
	isaved_scrEnd    = scrEnd;
	isaved_frmWidth  = frmWidth;
	isaved_frmWidthStart = frmWidthStart;
	isaved_frmWidthEnd   = frmWidthEnd;
}

void internalLoadBlitLimit(void)
{	scrStart  = isaved_scrStart;
	scrEnd    = isaved_scrEnd;
	frmWidth  = isaved_frmWidth;
	frmWidthStart = isaved_frmWidthStart;
	frmWidthEnd   = isaved_frmWidthEnd;
}

void setBlitLimit(clDeep *start, clDeep *end, u32 widthStart, u32 widthEnd)
{	scrStart  = start;
	scrEnd    = end  ;
	frmWidth  = widthEnd-widthStart;
	frmWidthStart = widthStart;
	frmWidthEnd   = widthEnd;
}

void setBlitLimit(u32 x, u32 y, u32 x1, u32 y1)
{	u32 width = x1 - x;
	scrStart  = &pixel[x+y*bufWidth];
	scrEnd    = &pixel[x1+y1*bufWidth];
	frmWidth  = width;
	frmWidthStart = x;
	frmWidthEnd   = x1;
}

void drawGfm(clDeep *Gfm, int x, int y)
{    
      int sx = Gfm[2]>>16 ; u32 sy = Gfm[2]&65535 ;

     if(x >= frmWidthStart && x + sx < frmWidthEnd)
		{		udrawGfm(Gfm,&pixel[y*bufWidth+x]) ;
                return;
        }

     if(x < (frmWidthStart-sx) || x > frmWidthEnd) return ;// out of screen on x
     
     if(x >= frmWidthStart) // clip right only
     {   u32 max = frmWidthEnd-x ;  u32 lnb, sze ;
         clDeep *scr = &pixel[y*bufWidth+x] ;
         if(scr > scrEnd) return ; // out of screen at down
         
         u32 upClip = (scr + sy*bufWidth > scrEnd) ; // is clipped at down ?

          Gfm += 3;
         
         if(scr < scrStart) // is clipped at up ?
         {  if(scr + sy*bufWidth < scrStart) return ; // out of screen at up
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                scr += bufWidth ; sy-- ;
            } while(scr < scrStart) ;
         }

         clDeep *screen = scr ; u32 c = 0 ; clDeep * lend ;
         while(c < sy)
         {  lnb = *Gfm++ ; lend = scr + max ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr + sze < lend) memcpy(scr,Gfm,sze<<clDeepDec) ;
                           else if(scr < lend)  memcpy(scr,Gfm,(lend-scr)<<clDeepDec) ;
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + bufWidth*(++c) ;
            if(upClip) if(scr > scrEnd) return ;
         };
     } else if(x+sx < frmWidthEnd) { // clip left only
         u32 lnb, sze ; clDeep *s = &pixel[y*bufWidth + frmWidthStart] ;

         if(s > scrEnd) return ; // out of screen at up
         u32 upClip = (s + sy*bufWidth > scrEnd) ; // is clipped at up ?

          Gfm += 3;

         if(s < scrStart) // is clipped at down ?
         {  if(s + sy*bufWidth < scrStart) return ; // out of screen at down
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                s += bufWidth ; sy-- ;
            } while(s < scrStart) ;
         }
         
         clDeep*scr = s+x ;
         scr -= frmWidthStart ;
         clDeep *screen = scr ;
         u32 c=0 ; u32 size ;
         
         while(c < sy)
         {  lnb = *Gfm++ ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr >= s)   memcpy(scr,Gfm,sze<<clDeepDec) ;
                           else if(scr + sze > s) { size = (scr + sze)-s ;
                                                    memcpy(s,Gfm+(sze-size),size<<clDeepDec) ;
                                                  }
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + bufWidth*(++c) ; s = scr+frmWidthStart ; s -= x ;
            if(upClip) if(s > scrEnd) return ;
         };
     } else return ;
}


void udrawGfm(clDeep*Gfm,clDeep*scr)
{
    Gfm+=2 ; u32 sy = (*Gfm++)&65535 ;
    
    u32 lnb, sze ;
    
    if(scr > scrEnd) return ; // out of screen at up
    u32 upClip = (scr + sy*bufWidth > scrEnd) ; // is clipped at up ?

    if(scr < scrStart) // is clipped at down ?
    {  if(scr + sy*bufWidth < scrStart) return ; // out of screen at down
       do{    lnb = *Gfm++ ;
              while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
              scr += bufWidth ; sy-- ;
       } while(scr < scrStart) ;
    }

    clDeep *screen = scr ;
    u32 c = 0 ;
    
    if(upClip)
     {  while(c < sy)
        {  lnb = *Gfm++ ;
           while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                          while(sze>1)
                            { *scr++ = *Gfm++;
                              *scr++ = *Gfm++;
                              sze-=2;
                            };
                          if(sze) *scr++ = *Gfm++;
           };  scr = screen + bufWidth*(++c) ;
           if(scr > scrEnd) return ;
        };
     }
    else
     {  while(c < sy)
        {  lnb = *Gfm++ ;
           while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                          
                          while(sze>1)
                            { *scr++ = *Gfm++;
                              *scr++ = *Gfm++;
                              sze-=2;
                            };
                          if(sze) *scr++ = *Gfm++;
           };  scr = screen + bufWidth*(++c) ;
        };
     }
} 

// convert a 4b Gfm to a 32b Gfm
clDeep * data2Gfm(unsigned char *data)
{ //unsigned char *d = data ;
  data+=3 ; u32 clNum=*data++ ; data+=4 ;
  u32 sx   = *(short*)data ; data+=2 ;
  u32 sy   = *(short*)data ; data+=2 ;
  int outSize = *(int*)data ; data+=4 ;
  //printf("\n%i colors %i*%i out size %i",clNum,sx,sy,outSize) ;
  clDeep *Gfm =  (clDeep*)malloc(outSize) ;
  clDeep *pal =  (clDeep*)malloc(4*clNum) ;
  memcpy(pal,data,4*clNum) ; data+=(4*clNum) ;
  clDeep *o = Gfm ;
  u32 cnt=0, c, jump, size, p1, p2 ;

  //printf("\n\nout size : %i\n%i colors { %x",outSize,clNum,*pal) ;
  //for(int c=1;c<clNum;c++) printf(",%x",pal[c]) ; printf(" }\nsize : %i*%i",sx,sy) ;

  *o++ = 0x6d6647 ;              // put signature "Gfm\0"
  *o++ = outSize ;               // put Gfm object size
  *o++ = sx<<16 | (sy & 65535) ; // put frame size x and y

  while(cnt++ < sy)
  {    *o++ = c = *data++ ; //printf("\n* line %i, %i sublines",cnt,c) ;
       while(c--) { jump = *data++ ; size = *data++ ; *o++ = jump ; *o++ = size ;
                    //printf("\n jmp %i sze %i | ",jump,size) ;
                    if(jump > sx || size > sx) {
                      //printf("\njump or size error ... pos %i",data-d) ;
                      //return 0 ;
                    }

                    while(size > 1) { p1 = (*data)>>4 ; p2 = (*data)&15 ;
                                      //printf(",%x,%x",p1,p2) ;
                                      size-=2 ; *o++ = pal[p1]>>8 ; *o++ = pal[p2]>>8 ;

                                      if(p1 >= clNum || p2 >= clNum) {
                                       //printf("\ndata error, out of pal ! ... pos %i ... data %i | %x | %c",data-d,*data,*data,*data) ;
                                        //return 0 ;
                                      }

                                      ++data ;
                                    };
                    if(size!=0) { //printf(" + %x",*data);
									*o++ = pal[(*data++)]>>8 ;
								}
                  };
  };
  *o = 0x2a2a2a2a ;
//  addFreeEntry(Gfm);
  return Gfm ;
}

struct	anim
{	u32	lastTime ;
	u32	frmTime  ;
	u32	curentFrm;
	u32	frmNumber;
	u32	animType;
	void (*onfinish)(struct anim **);
	void (*onflip)(struct anim **);
	void (*onplay)(struct anim **);
	clDeep	**Gfm, **iGfm;
};

void unCrunchGfm(clDeep ** gfm, u32 frmNb)
{	for(u32 c=0;c<frmNb;c++)
		gfm[c] = data2Gfm((u8*)(gfm[c]));
}


int Animate(struct anim **b) // check for time and animate if need, not draw
{	struct anim *a = *b ;
	//printf("\nanimate, tick : %u",tick);
	if(a->lastTime + a->frmTime < tick)
	{	//printf("%s","++");
		if(++(a->curentFrm) >= a->frmNumber)
		{	switch(a->animType)
			{ case 0 : a->curentFrm = 0 ; break ; /* loop anim */
			  case 1 : a->curentFrm = a->frmNumber-1 ; break ; /* stop at last frame */
			};
			if(a->onfinish) (a->onfinish)(b) ;
		} else if(a->onflip) (a->onflip)(b) ;
		a->lastTime = tick ; return 1 ;
	} else if(a->onplay) a->onplay(b) ;
    return 0 ;
}

void playAnim(struct anim **b,clDeep * screen)//, int way)
{	struct anim *a = *b ;    Animate(b) ;
	udrawGfm((a->Gfm)[a->curentFrm],screen) ;
}

void playAnim(struct anim **b, int x, int y, u32 way)
{   struct anim *a = *b ;    Animate(b) ;
	if(!way) drawGfm((a->Gfm)[a->curentFrm],x,y) ;
     //else   drawGfm((a->iGfm)[a->curentFrm],x,y) ;
}

void drawAnim(struct anim **b, int x, int y, u32 way)
{  	struct anim *a = *b ;
    if(!way) drawGfm((a->Gfm)[a->curentFrm],x,y) ;
     //else   drawGfm((a->iGfm)[a->curentFrm],x,y) ;
}

void drawFrm(struct anim **b, int x, int y, int frm, u32 way)
{   struct anim *a = *b ;
    if(!way) drawGfm((a->Gfm)[frm],x,y) ;
     //else   drawGfm((a->iGfm)[frm],x,y) ;
}

void ifreeAnim(struct anim *a)
{ if(!(a->iGfm)) return ;
  for(u32 c=0;c<a->frmNumber;c++) free((a->iGfm)[c]) ;
}

void mifreeAnim(struct anim **a, u32 nb)
{ for(u32 c=0;c<nb;c++) ifreeAnim(a[c]) ; }

struct anim * setAnim( clDeep **Gfm, u32 nb,
                       u32 frmTime, u32 animType,
                       void (*onfinish)(struct anim**),
                       void (*onflip)(struct anim**),
                       void (*onplay)(struct anim**)
                     )
{	struct anim *a = (struct anim *)malloc(sizeof(struct anim)) ;
	a->Gfm = Gfm ;	a->frmNumber = nb ; // a->iGfm = flipGfm(Gfm,nb) ;
	a->curentFrm = a->lastTime = 0 ;
    a->onplay=onplay ; a->onflip=onflip ; a->onfinish=onfinish ;
	a->frmTime = frmTime ; a->animType = animType ;
    return a ;
}

void resetAnim(struct anim **a)
{ (*a)->curentFrm=0 ; (*a)->lastTime = tick ; }

