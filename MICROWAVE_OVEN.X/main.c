/*
 * File:   main.c
 * Author: kumar
 *
 * Created on 9 October, 2023, 9:36 PM
 */


#include "main.h"
#pragma config WDTE = OFF  // Watchdog timer disabled

unsigned char sec=0, min=0 ,flag;
int operation_flag = POWER_ON_SCREEN ;

static void init_config(void) {
 //initialization of clcd module
    init_clcd();
    
   // initialization of matrix_keypad
    init_matrix_keypad();
    
    //RC2 pin as a output
    FAN_DDR =0;
    FAN =OFF ;  //TURN OFF
    
    //initialization of timer2
    init_timer2();
    PEIE = ON;
    GIE =ON;
    
    BUZZER_DDR = 0; //RC1 pin as output
    BUZZER = OFF;
    
}

void main(void) {
    init_config(); // calling initializing function
    
    //variable declaration
    
    int reset_flag ; // power on screen
    unsigned char key;
    while (1) 
    {
          key = read_matrix_keypad(STATE);
        
        //sw1  is pressed 
        if(operation_flag == MENU_DISPLAY_SCREEN)
        {
            if(key == 1)
            {
                operation_flag = MICRO_MODE;
                reset_flag = MODE_RESET;
                clear_screen();
                clcd_print(" Power = 900W   ",LINE2(0));
                __delay_ms(3000); //3sec
                clear_screen();
            }
            else if(key==2)
            { 
                operation_flag = GRILL_MODE;
                clear_screen();
                reset_flag = MODE_RESET;
                
            }
            else if(key==3)
            {
                operation_flag = CONVECTION_MODE;
                reset_flag = MODE_RESET;
                clear_screen();
            }
            else if(key==4)
            {
                sec=30;
                min=0;
                FAN= ON;
                TMR2ON =ON ;
                operation_flag = TIME_DISPLAY;
                clear_screen();
            }
        }
        else if(operation_flag == TIME_DISPLAY)
        {
            if(key==4)//start/resume
            {
              
              if(sec < 30)
              {
                sec = sec + 30;   
              }
              else if(sec >=30)
              {
                 min++;
                 sec= sec-30;
                         
              }
            }
            else if (key==5)//Pause
            {
                operation_flag = PAUSE;
            }
            else if(key==6)//stop
            {
                operation_flag= STOP;
                clear_screen();
            }
        }
          else if(operation_flag == PAUSE)
          {
              if(key==4)//start/resume
            { 
                FAN = ON;
                TMR2ON =ON;
                operation_flag = TIME_DISPLAY;  
            }
          }
       
        switch(operation_flag)
        {
            case POWER_ON_SCREEN :
                power_on_screen();
                operation_flag = MENU_DISPLAY_SCREEN;
                clear_screen();
                break;
                
            case MENU_DISPLAY_SCREEN :
                menu_display_screen();
                break;
                
            case MICRO_MODE: 
                set_time(key,reset_flag);
                break;
            case GRILL_MODE:
                set_time(key,reset_flag);
                break;
            case CONVECTION_MODE:
                set_temp(key,reset_flag);
                break;
            case TIME_DISPLAY :
                time_display_screen();
                break;
            case STOP:
                FAN = OFF;
                TMR2ON =OFF;
                operation_flag = MENU_DISPLAY_SCREEN;
                break;
            case PAUSE:
                FAN = OFF;
                TMR2ON =OFF;
                break;
            case START:
                operation_flag = TIME_DISPLAY;
                break;
        }
        if(flag==0)
        reset_flag = RESET_NOTHING;
        else if(flag==1)
        reset_flag = MODE_RESET;
    }
}
void time_display_screen(void)
{       
    //linel display
    clcd_print(" TIME =  ",LINE1(0));
    
    //printing sec and min on screen
    //min
    clcd_putch(min/10 + '0',LINE1(9));
    clcd_putch(min%10 + '0',LINE1(10));
    clcd_putch(':',LINE1(11));
    //SEC
    clcd_putch(sec/10 + '0',LINE1(12));
    clcd_putch(sec%10 + '0',LINE1(13));
         
    //PRINT OPTION
    clcd_print(" 4.Start/Resume",LINE2(0));
    clcd_print(" 5.Pause",LINE3(0));
    clcd_print(" 6.Stop",LINE4(0));
         
    if(sec==0 && min ==0)
    {
        clear_screen();
        clcd_print("  Timer Up !!", LINE2(0));
        BUZZER = ON ;
        __delay_ms(5000); // 3sec
        BUZZER = OFF;
        FAN = OFF;
        //switching off timer2
        TMR2ON =OFF;
        clear_screen();
        operation_flag = MENU_DISPLAY_SCREEN;
    }
        
}

void clear_screen(void)
{
     clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}

void menu_display_screen(void)
{
   clcd_print("1.Micro",LINE1(0));
   clcd_print("2.Grill",LINE2(0));
   clcd_print("3.Convection",LINE3(0));
   clcd_print("4.Start",LINE4(0));
    
}


void power_on_screen(void)
{   
    //printing bar on LCD line1 and line4
    unsigned char i;
    for(i=0;i<16;i++)
    {
    clcd_putch(BAR, LINE1(i));
    clcd_putch(BAR, LINE4(i));
    __delay_ms(10);
    }
    //printing message on LCD line2 and line3
    clcd_print("  Powering ON   ",LINE2(0));
    clcd_print(" Microwave Oven ",LINE3(0));
     __delay_ms(3000);
}

void set_temp(unsigned char key, int reset_flag)
{
    static unsigned char key_count , blink ,temp;
    static int wait;
    if(reset_flag == MODE_RESET)
    {
        key_count =0;
        wait =0;
        blink=0;
        temp=0;
        flag=0;
        key = ALL_RELEASED;
        clcd_print(" SET TEMP(oC) ",LINE1(0));  
        clcd_print(" TEMP = ",LINE2(0));  
        clcd_print("*:CLEAR  #:ENTER",LINE4(0));
    }
    
    
    if((key!= '*')&& (key!='#')&& (key!=ALL_RELEASED))
    {
        key_count++;
        if(key_count <=3) // reading temperature
        {
            temp = temp*10 + key;
        }
        
    }
    else if(key == '*')
    {
        temp =0;
        key_count =0;
    }
    else if(key == '#')
    {
        clear_screen();
        //start pre-heating
        clcd_print("  Pre-Heating ",LINE1(0));
        clcd_print("Time Rem.= ",LINE3(0));
        sec =03;
        TMR2ON =ON; // switching on the timer
        while(sec!=0)
        {
            //clcd_putch(sec/100 + '0',LINE3(11));
            clcd_putch((sec/10)%10 + '0',LINE3(12));
            clcd_putch(sec%10 + '0',LINE3(13));
        }
        if(sec==0)
        {
           clear_screen(); 
           flag=1;
           operation_flag = GRILL_MODE;
            
        }
    }
    if(wait++ ==20)
    {
        wait=0;
        blink=~blink;
        //printing temp on screen
        
       clcd_putch(temp/100 + '0' ,LINE2(9));
       clcd_putch((temp/10)%10 + '0' ,LINE2(10));
       clcd_putch(temp%10 + '0' ,LINE2(11));
        
    }
    if(blink)
    { 
        clcd_print("   ",LINE2(9));   
    }
    
}

void set_time(unsigned char key, int reset_flag)
{
    flag=0;
    static unsigned char key_count , blink_pos , blink;
    static int wait;
    if(reset_flag == MODE_RESET)
    {
        key_count =0;
        sec = 0;
        min =0 ;
        wait =0;
        blink=0;
        blink_pos=0;
        key = ALL_RELEASED;
        clcd_print("SET TIME (MM:SS)",LINE1(0));  
        clcd_print("TIME- ",LINE2(0));  
        clcd_print("*:CLEAR  #:ENTER",LINE4(0));

    }
    
    if((key!= '*')&& (key!='#')&& (key!=ALL_RELEASED))
    {
        key_count++;
        if(key_count <=2) // reading number of sec
        {
            sec = sec*10 + key;
            blink_pos =0;
        }
        else if((key_count >2)&&(key_count <=4))
        {
            min = min*10 +key;
            blink_pos =1;
        }
    }
    else if(key == '*')
    {
        if(blink_pos == 0)
        {
            sec =0;
            key_count =0;
        }
        else if(blink_pos ==1)
        {
            min =0;
             key_count =2;
        }
    }
    else if(key == '#')
    {
        clear_screen();
        operation_flag = TIME_DISPLAY;
        FAN = ON; 
        
        //switching on the timer
        TMR2ON =ON;
    }

    if(wait++ ==30)
    {
        wait=0;
        blink=~blink;
        //printing sec and min on screen
        //min
        clcd_putch(min/10 + '0',LINE2(6));
        clcd_putch(min%10 + '0',LINE2(7));
        clcd_putch(':',LINE2(8));
       //SEC
        clcd_putch(sec/10 + '0',LINE2(9));
         clcd_putch(sec%10 + '0',LINE2(10));
        
    }
    if(blink)
    {
        switch(blink_pos)
        {
            case 0: //sec
                clcd_print("  ",LINE2(9));
                break;
            case 1:
                clcd_print("  ",LINE2(6));
                break;
        }
    }
}