#include "main.h"
#include "clcd.h"
#include "adc.h"
#include "matrix_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include "EEPROM.h"
#include <xc.h>

//Global variables
extern unsigned int event_count;
extern State_t state;
int next = 0;
extern char time[];
unsigned int once = 1;

void init_dash()
{
    clcd_print("  TIME    EV  SP", LINE1(0));               //Prinitng info message in 1st line of the display
}

void view_dashboard()
{
    init_dash();
    /*Printing time on display*/
    clcd_print(time, LINE2(0));
    clcd_print("  ",LINE2(8));
    clcd_print("  ",LINE2(12));
    
    if ( once == 1 )
    {
        store_event();
        event_count++;              //Check this
        once = 0;
    }

    if (next == 1 && ( key == MK_SW2 || key == MK_SW3 )) 
    {
        event_count++;
        ev_index = 1;
        store_event();
        next = 0;
        key = 0;
    }

    if (key == MK_SW2 && ev_index < 7) 
    {
        event_count++;
        ev_index++;
        store_event();
    }

    if (key == MK_SW3 && ev_index > 1) 
    {
        event_count++;
        ev_index--;
        store_event();
    }

    if (key == MK_SW1) 
    {
        event_count++;
        ev_index = 8;
        next = 1;
        store_event();
    }

    /*Printing event on display*/
    clcd_print(event[ev_index], LINE2(10));

    /*Printing time on display*/
    clcd_putch((speed / 10) + '0', LINE2(14));
    clcd_putch((speed % 10) + '0', LINE2(15));

    if (key == MK_SW11) 
    {
        index = 0;
        star = 0;
        CLEAR_DISP_SCREEN;
        state = e_main_menu;
    }
}
