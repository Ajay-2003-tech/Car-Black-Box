/*
 * NAME         :   AJAY NAGIRIKANTI
 * DATE         :   29/01/2025
 * PROJECT NAME :   CAR BLACK BOX PROJECT
 * DESCRIPTION  :   We need to store gear(switches(matrix)),speed(potentiometer),
 *                  time(using RTC i.e 24 hours). We also have functions like view log, download log,
 *                  clear log and set time.
 */


#include <xc.h>
#include "main.h"
#include "adc.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include "EEPROM.h"

// Global Variables
char time[] = "00:00:00";
unsigned int event_count = 0;
unsigned int view_ind = 0;
unsigned int read_add = 0;
unsigned int address = 0;
unsigned char read_event[10][20];
unsigned int delay = 0;
unsigned int one_time = 1;
State_t state;

void init_config()
{
    state = e_dashboard;
    PEIE = 1;    
    ADCON1 = 0x0F;
    GIE = 1;

    init_clcd();
    init_adc();
    init_matrix_keypad();
    init_i2c();
    init_ds1307();
    init_uart();
    
    CLEAR_DISP_SCREEN;
}

/*get time function*/
static void get_time(void)
{
	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);

	if (clock_reg[0] & 0x40)
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	else
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	time[2] = ':';
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
	time[4] = '0' + (clock_reg[1] & 0x0F);
	time[5] = ':';
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
	time[7] = '0' + (clock_reg[2] & 0x0F);
	time[8] = '\0';
}

/*Store Event function*/
void store_event()
{   
    if(event_count > 10)                                                //If event_count is exceeding 10 , make it as 10
    {
        event_count = 10;
        address = 0;
        read_add = 12;
        for(unsigned char i = 0 ; i < 9 ; i++)                         //Overwriting the events
        {
            for(unsigned char j = 0 ; j < 12 ; j++)
            {
               write_external_eeprom(address++, read_external_eeprom(read_add++)); 
            }
        }
    }
    int i;
    for (i = 0; i < 8; i++) 
    {
        write_external_eeprom(address++, time[i]);                          //Storing time
    }
    for (i = 0; i < 2; i++) 
    {
        write_external_eeprom(address++, event[ev_index][i]);              //Storing event
    }
    write_external_eeprom(address++, speed / 10 + '0');                    //Storing speed
    write_external_eeprom(address++, speed % 10 + '0');

}

/*Event Reader function*/
void event_reader()
{
    int i, j;
    read_add=0;
    for (i = 0; i < event_count; i++) 
    {
        for (j = 0; j < 15; j++) 
        {
            if (j == 8 || j == 11) 
            {
                read_event[i][j] = ' ';
            } 
            else if (j == 14) 
            {
                read_event[i][j] = '\0';
            } 
            else 
            {
                read_event[i][j] = read_external_eeprom(read_add++);               //Reading from external EEPROM
            }
        }
    }
}

/*Displaying MAIN_MENU*/
void display_main_menu()
{
    /*Printing '*' based on star value*/
    if (star == 0) 
    {
        clcd_putch('*', LINE1(0));
        clcd_putch(' ', LINE2(0));
    } 
    else 
    {
        clcd_putch(' ', LINE1(0));
        clcd_putch('*', LINE2(0));
    }

    /*Printing and scrolling of Logs based on key press */
    clcd_print(menu[index], LINE1(1));
    clcd_print(menu[index + 1], LINE2(1));

    /*If switch2 is pressed , scroll down the menu*/
    if (key == MK_SW2) 
    {
        if (star == 0)               //If star = 0 , set star = 1
        {
            star = 1;
        } 
        else 
        {
            if ( index < 2 )        //if index < 2 , increment index value
            {
                index++;
            }
        }
    }

    /*If switch1 is pressed , scroll up the menu*/
    if (key == MK_SW1) 
    {
        if (star == 1)              //If star = 1 , reset the star value to zero
        {
            star = 0;
        } 
        else 
        {
            if ( index > 0 )        //If index > 0 , decrement index value by '1'
            {
                index--;
            }
        }
    }

    /*If switch11 is pressed , go to the log to which the star points*/
    if (key == MK_SW11) 
    {
        //CLEAR_DISP_SCREEN;
        if ( index == 0 && star == 0 )
        {
            state = e_view_log;                                                 //Selecting view log
        }
        else if ( index == 0 && star == 1 || ( index == 1 && star == 0 ))
        {
            state = e_clear_log;                                                //Selecting clear log
        }
        else if ( ( index == 1 && star == 1)  || ( index == 2 && star == 0 ) )
        {
            state = e_download_log;                                             //Selecting download log
        }
        else if ( index == 2 && star == 1)
        {
            CLEAR_DISP_SCREEN;
            one_time = 1;
            state = e_set_time;                                                 //Selecting Set time function
        }
    }
    
    /*On switch_12 press , go to dash board*/
    if ( key == MK_SW12 )
    {
        CLEAR_DISP_SCREEN;
        state = e_dashboard;
    }
}

/*View Log Function*/
unsigned int view_clear = 1;

void view_log()
{
    if ( view_clear )
    {
        CLEAR_DISP_SCREEN;
        view_clear = 0;
    }

    //Printing "NO LOGS" message , in case of zero events
    if (event_count == 0) 
    {
        for ( int i = 1000 ; i-- ; )
        {
            clcd_print("    NO LOGS    ", LINE1(0));
            clcd_print(" DO SOME EVENTS ",LINE2(0));
        }
        CLEAR_DISP_SCREEN;
        state = e_main_menu;                                        //Redirecting to menu screen
        return;
    }

    clcd_print("S   TIME   EV SP", LINE1(0));

    event_reader();

    if (view_ind < event_count) 
    {
        clcd_putch(view_ind+'0' , LINE2(0));
        clcd_print(read_event[view_ind], LINE2(2));
    }

    //On Switch_2 press scroll down the events
    if (key == MK_SW2 && ( view_ind < event_count -1 ) ) 
    {
        view_ind++;
    }

    //On Switch_1 press scroll up the events
    else if (key == MK_SW1 && ( view_ind > 0) ) 
    {
        view_ind--;
    }

    if (key == MK_SW12)                             //On switch_12 press , go to menu screen
    {
        CLEAR_DISP_SCREEN;
        state = e_main_menu;
    }
    
}


/*Download Log function*/
void download_log()
{
    CLEAR_DISP_SCREEN;
    
    //Printing "NO LOGS" message , in case of zero events
    if ( event_count == 0 )
    {    
        for ( int i = 1000 ; i-- ; )
        {
            clcd_print("    NO LOGS    ", LINE1(0));
            clcd_print(" DO SOME EVENTS ",LINE2(0));
        }
        CLEAR_DISP_SCREEN;
        state = e_main_menu;                                        //Redirecting to menu screen
        return;
    }
    
    event_reader();
    
    //Printing on TeraTerm
    puts("  TIME   EV SP\n\r");
    for (int i = 0; i < event_count; i++) 
    {
        puts(read_event[i]);
        puts("\n\r");
    }
    
    /*Printing Info messages*/
    for ( int i = 1000 ; i-- ; )
    {
        clcd_print(" DOWNLOADING... ", LINE1(0));
    }
    
    for ( int i = 1000 ; i-- ; )
    {
        clcd_print("   DOWNLOADED   ", LINE1(0));
        clcd_print("  SUCCESSFULLY  ", LINE2(0));
    }
    
    CLEAR_DISP_SCREEN;
    state = e_main_menu;                                      //Going back to menu screen after downloading logs                 
}


/*Clear Log Function*/
void clear_log()
{
    CLEAR_DISP_SCREEN;
    
    //Printing "NO LOGS" message , in case of zero events
    if ( event_count == 0 )
    {
        for ( int i = 1500 ; i-- ; )
        {
            clcd_print("    NO LOGS    ", LINE1(0));
            clcd_print(" DO SOME EVENTS ",LINE2(0));
        }
        
        state = e_main_menu;
        CLEAR_DISP_SCREEN;
        return;
    }
  
    //Resetting event_count and read_add
    event_count = 0;
    read_add = 0;

    /*Printing Info messages*/
    for ( int i = 1500 ; i-- ; )
    {
        clcd_print("CLEARING LOGS...", LINE1(0));
    }
    
    for ( int i = 1500 ; i-- ; )
    {
        clcd_print("  LOGS CLEARED  ", LINE1(0));
        clcd_print("  SUCCESSFULLY  ", LINE2(0));
    }
    
    CLEAR_DISP_SCREEN;
    state = e_main_menu;                                  //Going back to menu screen after clearing logs
}


/*Set time function*/
unsigned int field = 0;
unsigned int hour , minute , second ; 

void set_time()
{
    clcd_print("    HH:MM:SS    ", LINE1(0));            //Printing time format in first line of the display
    
    
    if ( one_time )                                     //Converting time from character to integer type
    {
        one_time = 0;
        field = 0;
        hour  = ((time[0]-48)*10) + ( time[1]-48);
        minute = ((time[3]-48)*10) + ( time[4]-48);
        second = ((time[6]-48)*10) + ( time[7]-48);
    }
    
    //If switch1 has pressed , increment the time
    if ( key == MK_SW1 )
    {
        if ( field == 0 )
        {
            if ( hour >= 23 )                        //If hour >= 23 , reset the hour value to zero
            {
                hour = 0;
            }
            else
            {
                hour++;
            }
        }
        if ( field == 1 )
        {
            if ( minute >= 59 )                        //If min >= 59 , reset the minute value
            {
                minute = 0;
            }
            else
            {
                minute++;                              //else increment minute value by '1'
            }
        }
        if ( field == 2 )
        {
            if ( second >= 59 )                        //If sec >= 59 , reset the second value
            {
                    second = 0;
            }
            else
            {
                second++;                              //else increment second value by '1'
            }   
        }
    }
    
    //If Switch2 has pressed , change the field
    if ( key == MK_SW2 )
    {
        if ( field < 2 )
        {
            field++;
        }
        else
        {
            field = 0;
        }
    }
    
    //Printing time on CLCD display
    if ( delay++ < 500 )
    {
        clcd_putch((hour/10)+'0', LINE2(4));
        clcd_putch((hour%10)+'0', LINE2(5));
        clcd_putch(':', LINE2(6));
        clcd_putch((minute/10)+'0', LINE2(7));
        clcd_putch((minute%10)+'0', LINE2(8));
        clcd_putch(':', LINE2(9));
        clcd_putch((second/10)+'0', LINE2(10));
        clcd_putch((second%10)+'0', LINE2(11));;
    }
    
    //Logic to print black box for the selected field for every half second
    if ( delay++ >= 500 )
    {
            if ( field == 0 )
            {
                clcd_putch(0xFF, LINE2(4));
                clcd_putch(0xFF, LINE2(5));
            }
            else if ( field == 1 )
            {
                clcd_putch(0xFF, LINE2(7));
                clcd_putch(0xFF, LINE2(8));
            }
            else if ( field == 2 )
            {
                clcd_putch(0xFF, LINE2(10));
                clcd_putch(0xFF, LINE2(11));
            }
    }
 
    if ( delay > 999 )
    {
        delay = 0;
    }
    /*Going back to dashboard*/
    if ( key == MK_SW12 )                                           //Shift to dashboard without saving
    {
        CLEAR_DISP_SCREEN;
        state = e_dashboard;
    }
    if ( key == MK_SW11 )                                           //Save and shift to dashboard
    {
        //saving time to RTC
        write_ds1307(HOUR_ADDR , ( (hour/10) << 4 ) | ( hour % 10 ) );
        write_ds1307(MIN_ADDR , ( (minute/10) << 4 ) | ( minute % 10 ) );
        write_ds1307(SEC_ADDR , ( (second/10) << 4 ) | ( second % 10 ) );
        CLEAR_DISP_SCREEN;
        state = e_dashboard;
    }
}

void main()
{
    init_config();

    while (1) 
    {
        get_time();
        key = read_switches(STATE_CHANGE);                      //Checking key press
        speed = read_adc(CHANNEL4) / 10.33;                     //Reading speed 

        /* Switch case to select particular logs */
        switch (state) 
        {
            case e_dashboard:
                view_dashboard();                               //To go to dashboard
                break;
                
            case e_main_menu:
                display_main_menu();                            //To go to menu screen
                break;
                
            case e_view_log:
                view_log();                                     //To go to view log
                break;
                
            case e_download_log:
                download_log();                                 //To go to download log
                break;
                
            case e_clear_log:
                clear_log();                                    //To go to clear log
                break;
                
            case e_set_time:
                set_time();                                     //To go to set time function
                break;
        }
    }
}

