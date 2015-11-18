
//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
//      M�dulo de acceso al rat�n
//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

#include "global.h"

extern int help_paint_active;
int m_b;

float m_x=0.0,m_y=0.0;
int joymx = 0, joymy=0;
void read_mouse2(void);
void libera_drag(void);


//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
//      Comprueba si el rat�n est� en una caja
//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

int mouse_in(int x, int y, int x2, int y2) {
  return(mouse_x>=x && mouse_x<=x2 && mouse_y>=y && mouse_y<=y2);
}

int wmouse_in(int x, int y, int an, int al) {
  return(wmouse_x>=x && wmouse_x<=x+an-1 &&
         wmouse_y>=y && wmouse_y<=y+al-1);
}

//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
//      Place your mouse in the indicated position
//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

void set_mouse(word x, word y) {
  m_x=(float)x;
  m_y=(float)y;
}

//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
//      Read mouse button and position
//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�

void read_mouse(void) {
  int n=0;
  int s,shift=0;

  old_mouse_b=mouse_b;

  read_mouse2();

#ifdef GCW
m_x+=joymx;
m_y+=joymy;
#endif

  if (modo<100 && hotkey && !help_paint_active) tecla();

  real_mouse_x=(int)m_x; real_mouse_y=(int)m_y;

  if (mouse_x!=(int)m_x || mouse_y!=(int)m_y || mouse_b!=m_b) {

    mouse_x=(int)m_x;
    mouse_y=(int)m_y;
    mouse_b=m_b;
    shift=1;

    if (modo<100 && hotkey && !help_paint_active) {
      if (key(_SPC)) {
        if (mouse_b!=0xfffd) {
          mouse_b=0xfffd;
        }
      } else if (mouse_b==0xfffd) mouse_b=0;
    }

  } else if (modo<100 && hotkey && !help_paint_active) { // Las teclas est�n solo activas en edici�n

    if (!(shift_status&4)) {

      mouse_x=mouse_shift_x;
      mouse_y=mouse_shift_y;

      if ((shift_status&3) || key(_L_SHIFT) || key(_R_SHIFT)) s=8; else s=1;

      if (key(_C_RIGHT) || key(_RIGHT) || key(_P)) {
        kbdFLAGS[_C_RIGHT]=0; kbdFLAGS[_RIGHT]=0; kbdFLAGS[_P]=0;
        mouse_x+=(1<<zoom)*s; shift=1;
      }

      if (key(_C_LEFT) || key(_LEFT) || key(_O)) {
        kbdFLAGS[_C_LEFT]=0; kbdFLAGS[_LEFT]=0; kbdFLAGS[_O]=0;
        mouse_x-=(1<<zoom)*s; shift=1;
      }

      if (key(_C_DOWN) || key(_DOWN) || key(_A)) {
        kbdFLAGS[_C_DOWN]=0; kbdFLAGS[_DOWN]=0; kbdFLAGS[_A]=0;
        mouse_y+=(1<<zoom)*s; shift=1;
      }

      if (key(_C_UP) || key(_UP) || key(_Q)) {
        kbdFLAGS[_C_UP]=0; kbdFLAGS[_UP]=0; kbdFLAGS[_Q]=0;
        mouse_y-=(1<<zoom)*s; shift=1;
      }

      if (key(_SPC)) {
        if (mouse_b!=0xfffd) {
          mouse_b=0xfffd;
        }
      } else if (mouse_b==0xfffd) mouse_b=0;

      if (shift) {
        real_mouse_x=mouse_x; real_mouse_y=mouse_y;
        set_mouse(mouse_x,mouse_y);
      } else mouse_shift=0;
    }
  }

  if (shift) {

    if (mouse_x<0) { mouse_x=0; n++; }
    else if (mouse_x>=vga_an) { mouse_x=vga_an-1; n++; }
    if (mouse_y<0) { mouse_y=0; n++; }
    else if (mouse_y>=vga_al) { mouse_y=vga_al-1; n++; }

    if (n) set_mouse(mouse_x,mouse_y);
  }

  if (shift) {
    mouse_shift=0;
    mouse_shift_x=mouse_x;
    mouse_shift_y=mouse_y;
  }

  coord_x=zoom_x+(mouse_shift_x-zx)/(1<<zoom);
  coord_y=zoom_y+(mouse_shift_y-zy)/(1<<zoom);
  if(free_drag) switch(arrastrar) {
    case 0:
      if ((mouse_b&1) && !(old_mouse_b&1)) {
        arrastrar=1; arrastrar_x=mouse_x; arrastrar_y=mouse_y;
      } break;
    case 1:
      arrastrar=0;
      break;
    case 2:
      if (mouse_b&1) {
        if (abs(mouse_x-arrastrar_x)>1 || abs(mouse_y-arrastrar_y)>1)
        {
          arrastrar=3;
          wmouse_x=-1;
          wmouse_y=-1;
          mouse_b=0;
          call((voidReturnType )v.click_handler);
          quien_arrastra=v.orden;
          mouse_b=1;
          mouse_graf=arrastrar_graf;
        }
      } else {
        arrastrar=0;
      } break;
    case 3:
      if (!(mouse_b&1)) {
        arrastrar=4;
      } break;
    case 4:
    case 5:
      libera_drag();
      break;
  }
}

void libera_drag(void) {
 int n;
  arrastrar=0;
  for (n=0;n<max_windows;n++)
    if (ventana[n].tipo && ventana[n].orden==quien_arrastra) break;
  if (n<max_windows && ventana[n].tipo==101 && ventana[n].mapa!=NULL) {
    free(ventana[n].mapa->map);
    free(ventana[n].mapa);
    ventana[n].mapa=NULL;

    if (n) {
      wup(n);
    }

    wmouse_x=-1; wmouse_y=-1; call((voidReturnType )v.paint_handler);

    if (n) {
      wdown(n);
    }

    if(modo>=100) vuelca_ventana(n); ventana[n].volcar=0;
  }
}

//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
//  New mouse routines ( without interruption )
//様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様�
#define JOY_DEADZONE 500
#ifdef GCW
extern float w_ratio;
extern float h_ratio;
#endif
void read_mouse2(void) {
	scan_code  =0;
	ascii=0;
//printf("divmouse.c- read_mouse2 REPLACE WITH SDL VERSION\n");
SDL_Event event;
#ifdef GCW_
while(SDL_PollEvent(&event))
{
	switch(event.type)
	{
		case SDL_QUIT:
		break;
		case SDL_KEYDOWN:			// Button press
			switch(event.key.keysym.sym)
			{
				case SDLK_LEFT:		// D-PAD LEFT
				break;
				case SDLK_RIGHT:	// D-PAD RIGHT
				break;
				case SDLK_UP:		// D-PAD UP
				break;
				case SDLK_DOWN:		// D-PAD DOWN
				break;
				case SDLK_LCTRL:	// A
				break;
				case SDLK_LALT:		// B
					m_b=1;
				break;
				case SDLK_LSHIFT:	// X
				break;
				case SDLK_SPACE:	// Y
				break;
				case SDLK_TAB:		// Left shoulder
				break;
				case SDLK_BACKSPACE:	// Right shoulder
				break;
				case SDLK_RETURN:	// Start
				break;
				case SDLK_ESCAPE:	// Select
				break;
				case SDLK_PAUSE:	// Lock
				break;

				default:
				break;
			}
		break;
		case SDL_KEYUP:				// Button release
			switch(event.key.keysym.sym)
			{
				case SDLK_LEFT:		// D-PAD LEFT
				break;
				case SDLK_RIGHT:	// D-PAD RIGHT
				break;
				case SDLK_UP:		// D-PAD UP
				break;
				case SDLK_DOWN:		// D-PAD DOWN
				break;
				case SDLK_LCTRL:	// A
				break;
				case SDLK_LALT:		// B
					m_b=0;
				break;
				case SDLK_LSHIFT:	// X
				break;
				case SDLK_SPACE:	// Y
				break;
				case SDLK_TAB:		// Left shoulder
				break;
				case SDLK_BACKSPACE:	// Right shoulder
				break;
				case SDLK_RETURN:	// Start
				break;
				case SDLK_ESCAPE:	// Select
				break;
				case SDLK_PAUSE:	// Lock
				break;

				default:
				break;
			}
		break;
		case SDL_JOYAXISMOTION:			// Analog joystick movement
			switch(event.jaxis.axis)
			{
				case 0:		// axis 0 (left-right)
					if(event.jaxis.value < -JOY_DEADZONE)
					{
						m_x-=2;//=event.jaxis.value;
						// left movement
					}
					else if(event.jaxis.value > JOY_DEADZONE)
					{
						m_x+=2;//=event.jaxis.value;
						// right movement
					}
				break;
				case 1:		// axis 1 (up-down)
					if(event.jaxis.value < -JOY_DEADZONE)
					{
						m_y-=2;//=event.jaxis.value;
						// up movement
					}
					else if(event.jaxis.value > JOY_DEADZONE)
					{
						m_y+=2;//=event.jaxis.value;
						// down movement
					}
				break;

				default:
				break;
			}
		break;

		default:
		break;
	}
}
#else

while(SDL_PollEvent(&event))
        {
			if(event.type == SDL_JOYAXISMOTION) {			// Analog joystick movement
				
			switch(event.jaxis.axis)
			{
				case 0:		// axis 0 (left-right)
					joymx=0;
					if(event.jaxis.value < -JOY_DEADZONE)
					{
						joymx=-2;//=event.jaxis.value;
						// left movement
					}
					else if(event.jaxis.value > JOY_DEADZONE)
					{
						joymx=2;//=event.jaxis.value;
						// right movement
					}
				break;
				case 1:		// axis 1 (up-down)
					joymy=0;
					if(event.jaxis.value < -JOY_DEADZONE)
					{
						joymy=-2;//=event.jaxis.value;
						// up movement
					}
					else if(event.jaxis.value > JOY_DEADZONE)
					{
						joymy=2;//=event.jaxis.value;
						// down movement
					}
				break;

				default:
				break;
			}
            /* If a quit event has been sent */
            }
            if (event.type == SDL_QUIT)
            {
                /* Quit the application */
                salir_del_entorno=1;
            }
              if (event.type == SDL_MOUSEMOTION)
            {
				m_x = event.motion.x;
            	m_y = event.motion.y;
#ifdef GCW
				m_x = event.motion.x*w_ratio;
            	m_y = event.motion.y*w_ratio;
#endif

//				m_x+=event.motion.xrel;
//				m_y+=event.motion.yrel;
			}
            /* If a button on the mouse is pressed. */
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
				
				if(event.button.button == SDL_BUTTON_LEFT)
				{
					m_b = 1;
				}
/*				if(event.button.button == SDL_BUTTON_RIGHT)
				{
					mouse->right = 1;
				}
				if(event.button.button == SDL_BUTTON_MIDDLE)
				{
					mouse->middle = 1;
				}
				* */
			//	printf("click\n");
				m_b = 1;
			}
			 if (event.type == SDL_KEYDOWN)
            {
				scan_code = sdl2key[event.key.keysym.sym];
				ascii = event.key.keysym.unicode&0x7f;
				key(scan_code)=1;
#ifdef GCW
					if(event.key.keysym.sym ==SDLK_LALT)		// B
						m_b=1;
#endif
			}
			if(event.type == SDL_KEYUP) 
			{
				scan_code = sdl2key[event.key.keysym.sym];
				//scan_code = event.key.keysym.scancode;
				key(scan_code)=0;
				scan_code=0;
#ifdef GCW
				if(event.key.keysym.sym ==SDLK_LALT)		// B
					m_b=0;
#endif
			}

			if (event.type == SDL_MOUSEBUTTONUP)
            {
				m_b = 0;
			}
			
                /* If the left button was pressed. */
//                if (event.button.button == SDL_BUTTON_LEFT)
                    /* Quit the application */
  //                  quit = 1;
            
        }
#endif

#ifdef NOTYET
  union REGS regs;
  short ix,iy;

  memset(&regs,0,sizeof(regs));
  regs.w.ax=3;
  int386(0x33,&regs,&regs);

  m_b=regs.w.bx;

  memset(&regs,0,sizeof(regs));
  regs.w.ax=0xb;
  int386(0x33,&regs,&regs);

  ix=regs.w.cx;
  iy=regs.w.dx;

  m_x+=(float)ix/(1.0+((float)Setupfile.mouse_ratio/3.0));
  m_y+=(float)iy/(1.0+((float)Setupfile.mouse_ratio/3.0));
#endif

}
