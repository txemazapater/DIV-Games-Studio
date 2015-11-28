
//�����������������������������������������������������������������������������
//      Pruebas con la interrupci�n de teclado
//�����������������������������������������������������������������������������

#include "inter.h"
#include "osdep.h"


#ifdef GCW

#define GCW_L SDLK_TAB
#define GCW_R SDLK_BACKSPACE
#define GCW_UP SDLK_UP
#define GCW_DOWN SDLK_DOWN
#define GCW_LEFT SDLK_LEFT
#define GCW_RIGHT SDLK_RIGHT
#define GCW_A SDLK_LALT
#define GCW_B SDLK_x
#define GCW_X SDLK_LSHIFT
#define GCW_Y SDLK_SPACE
#define GCW_START SDLK_RETURN
#define GCW_SELECT SDLK_ESCAPE
#define GCW_LOCK SDLK_PAUSE

#endif


//�����������������������������������������������������������������������������
// IRQ Data
//�����������������������������������������������������������������������������

typedef void (__interrupt __far *TIRQHandler)(void);

TIRQHandler OldIrqHandler;
TIRQHandler OldIrq23;
TIRQHandler OldIrq1b;

byte * shift = (byte *) 0x417; // Shift status

byte buf[64*3]; // {ascii,scan_code,shift_status}
int ibuf=0; // Puntero al buffer, inicio de la cola
int fbuf=0; // Puntero al buffer, fin de la cola

//�����������������������������������������������������������������������������
// Interrupt handler related functions.
//�����������������������������������������������������������������������������

int ctrl_c=0,alt_x=0;
//word * kb_start = (void*) 0x41a;
//word * kb_end = (void*) 0x41c;

void __far __interrupt __loadds IrqHandler(void)
{
#ifdef DOS
  int n;
  int scancode;

  scancode=inp(0x60);

  if (((*shift)&4) && scancode==46) {
    kbdFLAGS[46]=1; ctrl_c=1;
    outp(0x20,0x20);
  } else {

    if (scancode<128) {
      kbdFLAGS[scancode]=1;
    } else if (scancode>129) {
      kbdFLAGS[scancode-128]=0;
    } else {
      n=128; do { kbdFLAGS[--n]=0; } while (n);
    }

//    if (kbhit()) getch();

    OldIrqHandler();

    if ((*kb_end-28)%32+30==*kb_start) *kb_start=(*kb_start-28)%32+30;

  }
#endif
}

void __far __interrupt __loadds Irq23(void){}
void __far __interrupt __loadds Irq1b(void){}

//�����������������������������������������������������������������������������
// Obtains the address of an IRQ handler.
//�����������������������������������������������������������������������������

TIRQHandler GetIRQVector(int n)
{
#ifdef DOS
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x3500 + n;   // DOS4GW redefines the DOS get vector call.
    sregs.ds = sregs.es = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
    return (TIRQHandler)(MK_FP((word)sregs.es, outregs.x.ebx));
#endif
}

//�����������������������������������������������������������������������������
// Sets the address of an IRQ handler.
//�����������������������������������������������������������������������������

void SetIRQVector(int n, TIRQHandler vec)
{
#ifdef DOS
    struct SREGS sregs;
    union REGS inregs, outregs;

    inregs.x.eax = 0x2500 + n;   // DOS set vector call.
    inregs.x.edx = FP_OFF (vec);
    sregs.ds     = FP_SEG (vec);     // Handler pointer.
    sregs.es     = 0;
    int386x (0x21, &inregs, &outregs, &sregs);
#endif
}

//�����������������������������������������������������������������������������
// Function to initialize the handler.
//�����������������������������������������������������������������������������

void kbdInit(void)
{
	sdlkeyinit();
#ifdef DOS
    if (GetIRQVector(9) != IrqHandler) {   // If not already installed.
      OldIrqHandler=GetIRQVector(9);       // Get old handler.
      SetIRQVector(9,IrqHandler);          // Set our handler.
      OldIrq1b=GetIRQVector(0x1b);         // Get old handler.
      SetIRQVector(0x1b,Irq1b);            // Set our handler.
      OldIrq23=GetIRQVector(0x23);         // Get old handler.
      SetIRQVector(0x23,Irq23);            // Set our handler.
    }

    signal(SIGBREAK,SIG_IGN);
    signal(SIGINT,SIG_IGN);
#endif
}

//�����������������������������������������������������������������������������
// Function to uninitialize the handler.
//�����������������������������������������������������������������������������

void kbdReset(void)
{
#ifdef DOS
    if (GetIRQVector(9) == IrqHandler) {   // If it was installed.
      SetIRQVector(9,OldIrqHandler);       // Uninstall it.
      SetIRQVector(0x1b,OldIrq1b);         // Uninstall it.
      SetIRQVector(0x23,OldIrq23);         // Uninstall it.
    }
#endif
}

//�����������������������������������������������������������������������������
//      Espera una tecla, actualiza variables (ascii,scan_code y shift_status)
//�����������������������������������������������������������������������������

void tecla_bios(void) {
#ifdef DOS
  int as,sc,ss;
  union REGS r;
  struct SREGS s;

  if (_bios_keybrd(_KEYBRD_READY)) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    do {
      r.h.ah=0; int386x(0x16,&r,&r,&s);
      as=r.h.al; sc=r.h.ah;
      r.h.ah=2; int386x(0x16,&r,&r,&s); ss=r.h.al;
      buf[fbuf]=as; buf[fbuf+1]=sc; buf[fbuf+2]=ss;
      if ((fbuf+=3)==64*3) fbuf=0;

    } while (_bios_keybrd(_KEYBRD_READY));
  }

  if (ctrl_c==1) {
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=2; int386x(0x16,&r,&r,&s); ss=r.h.al;
    buf[fbuf]=0; buf[fbuf+1]=46; buf[fbuf+2]=ss;
    if ((fbuf+=3)==64*3) fbuf=0; ctrl_c=0;
  }
#endif
}


void checkmod(SDLMod mod) {
	
//	shift_status = 0;
	if( mod == KMOD_NONE ){
//            printf( "None\n" );
            return;
        }
	// if( mod & KMOD_NUM ) printf( "NUMLOCK " );
    //    if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
        if( mod & KMOD_LCTRL ) shift_status |=4; 
       if( mod & KMOD_RCTRL ) shift_status |=4;
        if( mod & KMOD_RSHIFT ) shift_status |=1;
        if( mod & KMOD_LSHIFT ) shift_status |=2;
        if( mod & KMOD_RALT ) shift_status |=8;
        if( mod & KMOD_LALT ) shift_status |=8;
        if( mod & KMOD_CTRL ) shift_status |=4;
        if( mod & KMOD_ALT ) shift_status |=8;
        
        if (mod & KMOD_CAPS) shift_status |=64;
        if (mod & KMOD_NUM) shift_status |=32;
        /*
         if( mod & KMOD_NUM ) printf( "NUMLOCK " );
        if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
        if( mod & KMOD_LCTRL ) printf( "LCTRL " );
        if( mod & KMOD_RCTRL ) printf( "RCTRL " );
        if( mod & KMOD_RSHIFT ) printf( "RSHIFT " );
        if( mod & KMOD_LSHIFT ) printf( "LSHIFT " );
        if( mod & KMOD_RALT ) printf( "RALT " );
        if( mod & KMOD_LALT ) printf( "LALT " );
        if( mod & KMOD_CTRL ) printf( "CTRL " );
//        if( mod & KMOD_SHIFT ) printf( "SHIFT " );
        if( mod & KMOD_ALT ) printf( "ALT " );
        
        printf("\n");
        * */
}




void tecla(void) {
//printf("tecla\n");
ascii=0; scan_code=0;
SDL_Event event;
while(SDL_PollEvent(&event))
        {
            /* If a quit event has been sent */
            if (event.type == SDL_QUIT)
            {
                /* Quit the application */
//                salir_del_entorno=1;
				alt_x=1;
                //exit(0);
            }
            
            if (event.type == SDL_KEYDOWN)
            {
				switch(event.key.keysym.sym) {
#ifdef GCW
				case SDLK_LEFT:		// D-PAD LEFT
					key(sdl2key[GCW_LEFT])=1;
				break;
				
				case SDLK_RIGHT:	// D-PAD RIGHT
					key(sdl2key[GCW_RIGHT])=1;

				break;
				case SDLK_UP:		// D-PAD UP
					key(sdl2key[GCW_UP])=1;
				break;
				
				case SDLK_DOWN:		// D-PAD DOWN
					key(sdl2key[GCW_DOWN])=1;
				break;
				
				case SDLK_LCTRL:	// A
					key(sdl2key[GCW_A])=1;
				break;
				
				case SDLK_LALT:		// B
					key(sdl2key[GCW_B])=1;
					mouse->left=1;
				break;
				
				case SDLK_LSHIFT:	// X
					key(sdl2key[GCW_X])=1;
				break;
				
				case SDLK_SPACE:	// Y
					key(sdl2key[GCW_Y])=1;
				break;

				case SDLK_TAB:		// Left shoulder
					key(sdl2key[GCW_L])=1;
				break;

				case SDLK_BACKSPACE:	// Right shoulder
					key(sdl2key[GCW_R])=1;
				break;
				
				case SDLK_RETURN:	// Start
					key(sdl2key[GCW_START])=1;
				break;
				
				case SDLK_ESCAPE:	// Select
					key(sdl2key[GCW_SELECT])=1;			
					if(key(sdl2key[GCW_X])==1)
						alt_x=1;
				break;
				
				case SDLK_PAUSE:	// Lock
					key(sdl2key[GCW_LOCK])=1;
				break;

				default:
				break;
#else
					case SDLK_LSHIFT:
						shift_status|=2;
						break;
					case SDLK_RSHIFT:
						shift_status|=1;
						break;
					case SDLK_LCTRL:
						shift_status|=4;
						break;
					case SDLK_RCTRL:
						shift_status|=4;
						break;
					case SDLK_LALT:
						shift_status|=8;
						break;
					case SDLK_RALT:
						shift_status|=8;
						break;
					case SDLK_INSERT:
						shift_status|=128;
#endif
				}
#ifndef GCW
				checkmod((SDLMod) event.key.keysym.mod);
				scan_code = sdl2key[event.key.keysym.sym];
				ascii = event.key.keysym.scancode;
// unicode not working on android
#ifndef DROID
				if(event.key.keysym.unicode>=0 &&  event.key.keysym.unicode<0x80)
					ascii = event.key.keysym.unicode;
#endif				
				kbdFLAGS[scan_code]=1;				
#endif
			}
			if(event.type == SDL_KEYUP) 
			{
#ifdef GCW
				switch(event.key.keysym.sym) {
				case SDLK_LEFT:		// D-PAD LEFT
					key(sdl2key[GCW_LEFT])=0;
				break;
				
				case SDLK_RIGHT:	// D-PAD RIGHT
					key(sdl2key[GCW_RIGHT])=0;

				break;
				case SDLK_UP:		// D-PAD UP
					key(sdl2key[GCW_UP])=0;
				break;
				
				case SDLK_DOWN:		// D-PAD DOWN
					key(sdl2key[GCW_DOWN])=0;
				break;
				
				case SDLK_LCTRL:	// A
					key(sdl2key[GCW_A])=0;
				break;
				
				case SDLK_LALT:		// B
					key(sdl2key[GCW_B])=0;
					mouse->left=1;
				break;
				
				case SDLK_LSHIFT:	// X
					key(sdl2key[GCW_X])=0;
				break;
				
				case SDLK_SPACE:	// Y
					key(sdl2key[GCW_Y])=0;
				break;

				case SDLK_TAB:		// Left shoulder
					key(sdl2key[GCW_L])=0;
				break;

				case SDLK_BACKSPACE:	// Right shoulder
					key(sdl2key[GCW_R])=0;
				break;
				
				case SDLK_RETURN:	// Start
					key(sdl2key[GCW_START])=0;
				break;
				
				case SDLK_ESCAPE:	// Select
					key(sdl2key[GCW_SELECT])=0;			
				break;
				
				case SDLK_PAUSE:	// Lock
					key(sdl2key[GCW_LOCK])=0;
				break;
				default:
				break;
			}
#else
				shift_status=0;
				checkmod((SDLMod) event.key.keysym.mod);
				
				scan_code = sdl2key[event.key.keysym.sym];
				//scan_code = event.key.keysym.scancode;
				kbdFLAGS[scan_code]=0;
#endif
			}
			  if (event.type == SDL_MOUSEMOTION)
            {
				mouse->x = event.motion.x;
	           	mouse->y = event.motion.y;

//				m_x+=event.motion.xrel;
//				m_y+=event.motion.yrel;
			}
            /* If a button on the mouse is pressed. */
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
				if(event.button.button == SDL_BUTTON_LEFT)
				{
					mouse->	left = 1;
#ifdef DEBUG
					mouse_b|=1;
#endif
				}
				if(event.button.button == SDL_BUTTON_RIGHT)
				{
					mouse->right = 1;
#ifdef DEBUG
					mouse_b|=2;
#endif
				}
				if(event.button.button == SDL_BUTTON_MIDDLE)
				{
					mouse->middle = 1;
#ifdef DEBUG
					mouse_b|=4;
#endif
				}
			}
			
			if (event.type == SDL_MOUSEBUTTONUP)
            {

				if(event.button.button == SDL_BUTTON_LEFT)
				{
					mouse->left = 0;
#ifdef DEBUG
					mouse_b ^=1;
#endif
				}
				if(event.button.button == SDL_BUTTON_RIGHT)
				{
					mouse->right = 0;
#ifdef DEBUG
					mouse_b ^=2;
#endif
					}
				if(event.button.button == SDL_BUTTON_MIDDLE)
				{
					mouse->middle = 0;
#ifdef DEBUG
					mouse_b ^=4;
#endif
				}	
			}
        }

    if ((shift_status&8) && scan_code==_x) alt_x=1; 

#ifdef DOS
  union REGS r;
  struct SREGS s;

  tecla_bios();

  if (ibuf!=fbuf) {
    ascii=buf[ibuf]; scan_code=buf[ibuf+1]; shift_status=buf[ibuf+2];
    if ((shift_status&8) && scan_code==_x) alt_x=1; else alt_x=0;
    if ((ibuf+=3)==buf_max*3) ibuf=0;
  } else {
    ascii=0; scan_code=0;
    s.ds=s.es=s.fs=s.gs=FP_SEG(&s);
    r.h.ah=2; int386x(0x16,&r,&r,&s); shift_status=r.h.al;
  }
#endif
}

//�����������������������������������������������������������������������������
//      Vacia el buffer de teclado (real e interno)
//�����������������������������������������������������������������������������

void vacia_buffer(void) {
#ifdef DOS
  union REGS r;
  struct SREGS s;

  s.ds=s.es=s.fs=s.gs=FP_SEG(&s);

  while(_bios_keybrd(_KEYBRD_READY)) { r.h.ah=0; int386x(0x16,&r,&r,&s); }
  ascii=0; scan_code=0;
  r.h.ah=2; int386x(0x16,&r,&r,&s); shift_status=r.h.al;
#endif
  ibuf=fbuf=0; // Vacia el buffer interno
}
