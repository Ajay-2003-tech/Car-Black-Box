#ifndef MAIN_H
#define MAIN_H

#include <xc.h>
#include <string.h>
#include "uart.h"


/* Enum for maintaining the app state */
typedef enum {
    e_dashboard, e_main_menu, e_view_log, e_set_time, e_download_log, e_clear_log
} State_t;


void init_config(void);

//char time[] = "00:00:00";

unsigned int star = 0;
unsigned int index = 0;
unsigned ev_index = 0;

char event[9][3] = {"ON","GN","G1","G2","G3","G4","G5","GR","C_"};
unsigned int speed = 0;
unsigned int key = 0;

static char menu[4][16]={" View log      "," Clear log     "," Download log  "," Set time      "};

unsigned char clock_reg[3];

void store_event(void);
void init_dashboard(void);
void dashboard(void);

static void get_time(void);


extern State_t state; // App state

//Function declarations

//Dashboard function declaration
void view_dashboard(void);

//Storing events function declaration
void store_event(void);

//Password function declaration
void password(void);

//main menu function declaration
void display_main_menu(void);

//View log function declaration
void view_log(void);

//Reading events function declaration
void event_reader(void);

//Change password function declaration
void change_password(void);

//Set time function declaration
void set_time(void);

//Download log function _declaration
void download_log(void);

//Clear log function declaration
void clear_log(void);

#endif