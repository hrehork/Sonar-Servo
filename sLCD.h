#ifndef LCD_H //straznik naglowka
#define LCD_H

#include "MKL46Z4.h"

#define LCD_N_FRONT 8 //frontplane 8 pinow
#define LCD_N_BACK 4	//backplane 4 piny

/*
makra dla backplane,kazdy znak sterowany jest prez 2 piny(patrz tabelka)teraz robimy makra dla wierszy tabelki
Pin 1 -> (Digit*2-1), Pin 2 -> Digit*2 //TU DIGIT NUMERUJEMY OD 0, BO WIEMY, ZE TAK JEST
			Pin1 		Pin2
COM0   D			 dp/decimal(dziesietny, ale tez po przecinku)
COM1	 E 			 C
COM2	 G			 B
COM3	 F			 A
*/
#define LCD_S_D 0x11 //dla segmentu D to COM0
#define LCD_S_E 0x22 //dla segmentu E COM1
#define LCD_S_G 0x44 //dla segmentu G COM2
#define LCD_S_F 0x88 //dla segmentu F COM3
#define LCD_S_DP 0x11 //dla segmentu dp/DEC COM0
#define LCD_S_C 0x22 //dla segmentu C COM1
#define LCD_S_B 0x44 //dla segmentu B COM2
#define LCD_S_A 0x88 //dla segmentu A COM3
#define LCD_CLEAR 0x00 //CLEAR

//makra dla kazdego pinu frontplane. Pin 5[LCD_S401] czyli teraz bez uwzgledniania COMow O to P37, 1[dawny 6] to P17 itd.
#define LCD_FRONT0 37u
#define LCD_FRONT1 17u
#define LCD_FRONT2 7u
#define LCD_FRONT3 8u
#define LCD_FRONT4 53u
#define LCD_FRONT5 38u
#define LCD_FRONT6 10u
#define LCD_FRONT7 11u
//makra dla kazdego pinu backplane
//#define LCD_BACK0 40
//#define LCD_BACK1 52
//#define LCD_BACK2 19
//#define LCD_BACK3 18

//makra do funkcji s_LCD_ERROR
#define WRONG_POSITION 1
#define WRONG_VALUE 2
#define WRONG_FUNCTION 3

//makra do funkcji sLCD_hdob //hexadecimal=1 decimal=2 octal=3 binary=4
#define HEX_VALUE 1
#define DECIMAL_VALUE 2
#define OCTAL_VALUE 3
#define BINARY _VALUE 4

//zmienne tworzace 2 tablice(pinow frontplane i backplane) 
//z numerami pinow mikrokontrolera KL46 LCD pin [np. P40, P52 itd]
const static uint8_t LCD_Front_Pin[LCD_N_FRONT] ={LCD_FRONT0,LCD_FRONT1,LCD_FRONT2,LCD_FRONT3,
																													LCD_FRONT4,LCD_FRONT5,LCD_FRONT6,LCD_FRONT7};
//const static uint8_t LCD_Back_Pin[LCD_N_BACK] ={LCD_BACK0,LCD_BACK1,LCD_BACK2,LCD_BACK3};
void sLCD_set (uint8_t value, uint8_t digit);
void sLCD_Init(void);	
//obsluga kropek. toggle 1 wlacza, 0 wylacza, digit - na ktorej pozycji, 4 pozycja to colon[dwukropek] 	
void sLCD_dp(uint8_t toggle, uint8_t digit);

//wyswietla napis Err i kod bledu
void sLCD_Error(uint8_t errcode); 

//value uint16_t, bo max wart do wyswietlenia to ffff [16 bitow]
void sLCD_hdob(uint8_t choose,int value); //funkcja do wyswietlania wartosci w zadanym formacie
//Simple counter without using buttons																													
void sLCD_SimpleCounter(void);
#endif
