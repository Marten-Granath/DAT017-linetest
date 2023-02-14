/*
 * 	  startup.c
 */
 
__attribute__((naked)) 
__attribute__((section (".start_section")) )

void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

// Port Definitions
#define SysTick  ((volatile unsigned int *) 0xE000E010)
#define STK_CTRL ((volatile unsigned int *) SysTick + 0x0)
#define STK_LOAD ((volatile unsigned int *) SysTick + 0x4)
#define STK_VAL  ((volatile unsigned int *) SysTick + 0x8)

/*
#define PORT_BASE   ((volatile unsigned int  *) 0x40021000)
// Definitions of Word Adresses for Initiation 
#define portModer   ((volatile unsigned int  *) PORT_BASE + 0x0)
#define portOtyper  ((volatile unsigned int  *) PORT_BASE + 0x4)
#define portOspeedr ((volatile unsigned int  *) PORT_BASE + 0x8)
#define portPupdr   ((volatile unsigned int  *) PORT_BASE + 0xC)
// Definitions of Byte Adresses for Data and Controlregister 
#define portIdrLow  ((volatile unsigned char *) PORT_BASE + 0x10)
#define portIdrHigh ((volatile unsigned char *) PORT_BASE + 0x11)
#define portOdrLow  ((volatile unsigned char *) PORT_BASE + 0x14)
#define portOdrHigh ((volatile unsigned char *) PORT_BASE + 0x15)
// Definitions for Bit Positions in Controlregister 
#define B_E         ((volatile unsigned char *) 0x40)
#define B_SELECT    ((volatile unsigned char *) 0x4)  
#define B_RW        ((volatile unsigned char *) 0x2)
#define B_RS        ((volatile unsigned char *) 0x1)
*/

typedef struct
{
    char x,y;
} POINT, *PPOINT;

typedef struct
{
    POINT p0; 
    POINT p1;
} LINE, *PLINE;

// Definitions of lines
LINE lines[] = {
    {40,10,  100,10},
    {40,10,  100,20},
    {40,10,  100,30},
    {40,10,  100,40},
    {40,10,  100,50},
    {40,10,  100,60},
    {40,10,   90,60},
    {40,10,   80,60},
    {40,10,   70,60},
    {40,10,   60,60},
    {40,10,   50,60},
    {40,10,   40,60}
};

__attribute__((naked))
void graphic_initialize(void)
{
    __asm volatile (".HWORD 0xDFF0\n");
    __asm volatile ("BX LR\n");
}

__attribute__((naked))
void graphic_clear_screen(void)
{
    __asm volatile (".HWORD 0xDFF1\n");
    __asm volatile ("BX LR\n");
}

__attribute__((naked))
void graphic_pixel_set(int x, int y)
{
    __asm volatile (".HWORD 0xDFF2\n");
    __asm volatile ("BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear(int x, int y)
{
    __asm volatile (".HWORD 0xDFF3\n");
    __asm volatile ("BX LR\n");
}

// Method to draw a line between two points
int draw_line(PLINE l)
{
    POINT p0 = l->p0;
    POINT p1 = l->p1;
    int steep;
    int x_temp;
    int y_temp;
    int deltax;
    int deltay;
    int error = 0;
    int x;
    int y;
    int y_step;
    
    // If-statement to check if were within bounds
    if ((1 <= p0.x <= 128) && (1 <= p1.x <= 128) && (1 <= p0.y <= 64) && (1 <= p1.y <= 64))
    {
        // Draw a line according to Bresenham's Algorithm
        if (abs(p1.y-p0.y) > abs(p1.x-p0.x))
        {
            steep = 1;
        } 
        else
        {
            steep = 0;
        }
        if (steep)
        {
            // Swap(x0, y0)
            x_temp = p0.x;
            p0.x = p0.y;
            p0.y = x_temp;
            
            //Swap(x1, y1)
            x_temp = p1.x;
            p1.x = p1.y;
            p1.y = x_temp;
        }
        
        if (p0.x > p1.x)
        {
            //Swap(x0,x1)
            x_temp = p0.x;
            p0.x = p1.x;
            p1.x = x_temp;
            
            //Swap(y0,y1)
            y_temp = p0.y;
            p0.y = p1.y;
            p1.y = y_temp;
        }
        
        // Calculate delta values
        deltax = p1.x-p0.x;
        deltay = abs(p1.y-p0.y);
        
        // Assign y-value
        y = p0.y;
        
        // Assign y_step-value
        if (p0.y < p1.y)
        {
            y_step = 1;
        }
        else
        {
            y_step = -1;
        }
        
        for (x=p0.x; x <= p1.x; x++)
        {
            if (steep)
            {
                graphic_pixel_set(y,x);
            }
            else
            {
                graphic_pixel_set(x,y);
            }
            
            error = error + deltay;
            
            if (2*error >= deltax)
            {
                y = y + y_step;
                error = error-deltax;
            }
        }
        return 1;
    }
    return 0;
}


// Method to delay 250 ns
void delay_250ns(void)
{
    // System Clock: 168*10^6 Hz
    // 250 nanosekunder: 250*168*10^6*10^-9 = 42 = 0x2A
    *STK_CTRL = 0;      // Reset SysTick
    *STK_LOAD = 0x2A-1; // Load value (N klockcykler ges av räknarvärde N-1)
    *STK_VAL  = 0;      // Reset counter
    *STK_CTRL = 5;      // Prepare SysTick (STK_LOAD to STK_VAL)
    
    while ((*STK_CTRL & 0x00010000) == 0)
    {
        //*STK_VAL = *STK_VAL - 1;
    }
    
    *STK_CTRL = 0;
}

// Method to delay variable mikroseconds
void delay_mikro(unsigned int us)
{
    for (int i = 1; i <= us; i++)
    {
        #ifdef SIMULATOR
            ms = ms/1000;
            ms++;
       
        #endif
            delay_250ns();
            delay_250ns();
            delay_250ns();
            delay_250ns();
    }
}

// Method to delay variable millieseconds
void delay_milli(unsigned int ms)
{
    for (int i = 1; i <= ms; i++)
    {
        #ifdef SIMULATOR
            ms = ms/1000;
            ms++;
        
        #endif
            delay_mikro(4);
    }
}

void main(void)
{
    graphic_initialize();
    graphic_clear_screen();
    while(1)
    {
        for (int i = 0; i < 12; i++)
        {
            draw_line(&lines[i]);
            delay_milli(50);
        }
    graphic_clear_screen();
    }
}


