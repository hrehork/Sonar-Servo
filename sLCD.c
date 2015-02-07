#include "MKL46Z4.h" //biblioteka na str KEIL
#include "sLCD.h"
//wszystkie uint8/16/32_t normalnie w bibliotece <stdint.h> [Standard Integer Types]

void sLCD_Init(void){
	
//podanie zegara portom wykorzystywanym w obsludze wyswietlacza(B,C,D,E) i modulowi sLCD

SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | 
	             SIM_SCGC5_PORTE_MASK | SIM_SCGC5_SLCD_MASK;
//wstepne wylaczenie i reset kontrolera sterownika sLCD, GCR[GENERAL CONTROL REGISTER]
LCD->	GCR |= LCD_GCR_PADSAFE_MASK;//wlaczenie glownej blokady, Pad Safe State enable. force safe state on LCD pad controls 
LCD->GCR &= ~LCD_GCR_LCDEN_MASK;//na wszelki wypadek clear LCD Enable bit w trakcie konfiguracji
	//tylko bit LCDEN 0, pozostale 1 wiec logiczne AND

//konfiguracja multiplekserow do operacji portow jako kontroler LCD, MUX(0) - pin analog - o to nam chodzi
  PORTD->PCR[0] = PORT_PCR_MUX(0u);		// PTD0
	PORTE->PCR[4] = PORT_PCR_MUX(0u);		// PTE4
	PORTB->PCR[23] = PORT_PCR_MUX(0u); 	// PTB23
	PORTB->PCR[22] = PORT_PCR_MUX(0u);	// PTB22
	PORTC->PCR[17] = PORT_PCR_MUX(0u);	// PTC17
	PORTB->PCR[21] = PORT_PCR_MUX(0u);	// PTB21
	PORTB->PCR[7] = PORT_PCR_MUX(0u);		// PTB7
	PORTB->PCR[8] = PORT_PCR_MUX(0u);		// PTB8
	PORTE->PCR[5] = PORT_PCR_MUX(0u);		// PTE5
	PORTC->PCR[18] = PORT_PCR_MUX(0u);	// PTC18
	PORTB->PCR[10] = PORT_PCR_MUX(0u);	// PTB10
	PORTB->PCR[11] = PORT_PCR_MUX(0u);	// PTB11

//konfiguracja rejestrow LCD 
LCD->GCR = //GCR kontroluje wiekszosc opcji LCD
LCD_GCR_RVTRIM(0x00)| //Regulated Voltage Trim(nastroic, adjust) 
//RVEN(Regulated Voltage Enable) jest wylaczony ale i tak ustawiamy(trim) na znana nam wartosc(0) 
LCD_GCR_CPSEL_MASK | //choose charge pump (set to 1) or bias resistor [to czy to 1/3internal bias is forced]
//bias - skladowa stala; A charge pump is a kind of DC to DC converter that uses capacitors as energy
//storage elements to create either a higher or lower voltage power source
LCD_GCR_LADJ(0x03) | //Load adjust[dostosowanie ladunku] przy CPSEL=1 wart. 11(0x03) slowest clock source 
										 //for charge pump, wolniejszy zegar, ale wiekszy ladunek pojemnosciowy
LCD_GCR_VSUPPLY_MASK | //Voltage Supply Control Drive VLL3 externally from VDD or drive VLL internally from Vireg
LCD_GCR_ALTDIV(0x00) | //LCD AlternateClock Divider 0 Divide factor = 1 (No divide)
LCD_GCR_SOURCE_MASK | //LCD Clock Source Select 1 Selects output of the alternate clock source selection as the LCD clock source.
LCD_GCR_LCLK(0x01) | //LCD Clock Prescaler Used as a clock divider to generate the SLCD frame frequency.
LCD_GCR_DUTY(0x03); //Use 4 BP (1/4 duty cycle). (Default) mamy 4 cyfry(4 backplanes), wiec 1/4 duty cycle
/*A duty cycle is the percentage of one period in which a signal is active!
SLCD frame frequency is defined as the number of times the LCD segments are energized per second. [str  892 RM]
The SLCD base clock frequency is the SLCD frame frequency multiplied by the number of back plane phases that are being generated.
*/

/*konfiguracja migotania[to flicker(noun to plomyk)] wyswietlacza
The LCD clock is the basis for the calculation of the LCD controller blink frequency. The
LCD controller blink frequency is equal to the LCD clock (GCR[LCLK]) divided by the
factor selected by the AR[BRATE[2:0]] bits.*/
LCD->AR = LCD_AR_BRATE(0x03);

//konfiguracja rejestru FDCR[Fault Detect Control Register] - same 0 wylaczamy detekcje bledow
LCD->FDCR = 0x00000000;
//aktywowanie 12 pinow do kontroli wyswietlaczem frontplane 2 resjestry p 32bity [LCD Pin ENable]
//np.P40 to PEN[1] = LCD_PEN_PEN[1<<8] 40-32=8
LCD->PEN[0] =
LCD_PEN_PEN(1u<<7) |  //LCD_P7
LCD_PEN_PEN(1u<<8) |  //LCD_P8
LCD_PEN_PEN(1u<<10) |	//LCD_P10
LCD_PEN_PEN(1u<<11) |	//LCD_P11
LCD_PEN_PEN(1u<<17) |	//LCD_P17
LCD_PEN_PEN(1u<<18) |	//LCD_P18
LCD_PEN_PEN(1u<<19);	//LCD_P19
LCD->PEN[1] =
LCD_PEN_PEN(1u<<5) | 	//LCD_P37
LCD_PEN_PEN(1u<<6) |	//LCD_P38
LCD_PEN_PEN(1u<<8) |	//LCD_P40
LCD_PEN_PEN(1u<<20) |	//LCD_P52
LCD_PEN_PEN(1u<<21);   //LCD_P53

//skonfigurowanie 4 pinow plaszczyzny tylnej[backplane]
LCD->BPEN[0] = 
LCD_BPEN_BPEN(1u<<18) | //COM3, P18
LCD_BPEN_BPEN(1u<<19);	 //COM2, P19
LCD->BPEN[1] = 
LCD_BPEN_BPEN(1u<<8) | //COM0, P40
LCD_BPEN_BPEN(1u<<20);  //COM1, P52
/*
kofiguracja rejestrow przebiegow [Waveform register] - 4 aktywne[COM0(P40), COM1(P52), COM2(P19), COM3(P18)] reszta nie
konfiguracja polega na ronomiernym rozlozeniu faz 
							H		G		F		E		D		C		B		A
COM0 WF40 		0		0		0		1		0		0		0		1
COM1 WF52 		0		0		1		0		0		0		1		0
COM2 WF19 		0		1		0		0		0		1		0		0
COM3 WF18 		1		0		0		0		1		0		0		0
poniewaz sterownik moze obslugiwac 8 cyfr,a my mamy 4, wiec dzielimy na 2 plaszczyzny [A-D] i [H-E]
ktore maja po 4 bity i sa symetryczne COM0 najpierw to phase A and E, COM1 phase B and F etc.
jeden rejestr LCD->WF[] miesci 4 rejestry po 8 bitow, 64 piny LCD [64 male rejestry]
*/
LCD->WF[0] =
LCD_WF_WF0(0x00) | //P0
LCD_WF_WF1(0x00) | //P1
LCD_WF_WF2(0x00) | //P2
LCD_WF_WF3(0x00);  //P3

LCD->WF[1] =
LCD_WF_WF4(0x00) | //P4
LCD_WF_WF5(0x00) | //P5
LCD_WF_WF6(0x00) | //P6
LCD_WF_WF7(0x00);  //P7

LCD->WF[2] =
LCD_WF_WF8(0x00) | //P8
LCD_WF_WF9(0x00) | //P9
LCD_WF_WF10(0x00) | //P10
LCD_WF_WF11(0x00);  //P11

LCD->WF[3] =
LCD_WF_WF12(0x00) | //P12
LCD_WF_WF13(0x00) | //P13
LCD_WF_WF14(0x00) | //P14
LCD_WF_WF15(0x00);  //P15

LCD->WF[4] =
LCD_WF_WF16(0x00) | //P16
LCD_WF_WF17(0x00) | //P17
LCD_WF_WF18(0x88) | //P18 COM3 phase H and D, wiec 10001000 =0x88
LCD_WF_WF19(0x44);  //P19 COM2 phase G and C, wiec 01000100 =0x44

LCD->WF[5] =
LCD_WF_WF20(0x00) | //P20
LCD_WF_WF21(0x00) | //P21
LCD_WF_WF22(0x00) | //P22
LCD_WF_WF23(0x00);  //P23

LCD->WF[6] =
LCD_WF_WF24(0x00) | //P24
LCD_WF_WF25(0x00) | //P25
LCD_WF_WF26(0x00) | //P26
LCD_WF_WF27(0x00);  //P27

LCD->WF[7] =
LCD_WF_WF28(0x00) | //P28
LCD_WF_WF29(0x00) | //P29
LCD_WF_WF30(0x00) | //P30
LCD_WF_WF31(0x00);  //P31

LCD->WF[8] =
LCD_WF_WF32(0x00) | //P32
LCD_WF_WF33(0x00) | //P33
LCD_WF_WF34(0x00) | //P34
LCD_WF_WF35(0x00);  //P35

LCD->WF[9] =
LCD_WF_WF36(0x00) | //P36
LCD_WF_WF37(0x00) | //P37
LCD_WF_WF38(0x00) | //P38
LCD_WF_WF39(0x00);  //P39

LCD->WF[10] =
LCD_WF_WF40(0x11) | //P40 COM0 phase E and A, wiec 00010001 =0x11
LCD_WF_WF41(0x00) | //P41
LCD_WF_WF42(0x00) | //P42
LCD_WF_WF43(0x00);  //P43

LCD->WF[11] =
LCD_WF_WF44(0x00) | //P44
LCD_WF_WF45(0x00) | //P45
LCD_WF_WF46(0x00) | //P46
LCD_WF_WF47(0x00);  //P47

LCD->WF[12] =
LCD_WF_WF48(0x00) | //P48
LCD_WF_WF49(0x00) | //P49
LCD_WF_WF50(0x00) | //P50
LCD_WF_WF51(0x00);  //P51

LCD->WF[13] =
LCD_WF_WF52(0x22) | //P52 COM1 phase F and B, wiec 00100010 =0x22
LCD_WF_WF53(0x00) | //P53
LCD_WF_WF54(0x00) | //P54
LCD_WF_WF55(0x00);  //P55

LCD->WF[14] =
LCD_WF_WF56(0x00) | //P56
LCD_WF_WF57(0x00) | //P57
LCD_WF_WF58(0x00) | //P58
LCD_WF_WF59(0x00);  //P59

LCD->WF[15] =
LCD_WF_WF60(0x00) | //P60
LCD_WF_WF61(0x00) | //P61
LCD_WF_WF62(0x00) | //P62
LCD_WF_WF63(0x00);  //P63

//koniec konfiguracji wiec clear PADSAFE 
LCD->GCR &= ~LCD_GCR_PADSAFE_MASK; //same 1 tylko bit PADSAFE wyzerowany, zatem logiczne AND
LCD->GCR |= LCD_GCR_LCDEN_MASK;//wlaczenie wyswietlacza

}
//*****************************************************************************************************
void sLCD_set (uint8_t value, uint8_t digit){
/*value - wyswietlana wartosc 
digit pozycja na ktorej ma byc wyswietlona - TU NUMERUJEMY DIGIT OD 1[bo to dla uzytkownika]
wiec inny wzor np. 2*Digit-2 i 2*Digit-1[pamietaj, ze 2 kolumny obsluguja segmenty 1 cyfry
np. 1 cyfra - segmenty 5 [pin P37] i 6[P17] w tablicy LCD_FRONT_PIN to pozycje 0 i 1 ->te pozycje chcemy uzyskac
1*2-2=0 OK i 1*2-1=1 OK -wzor sprawdzony		
tworzymy wzory cyfr	
np. WF8B[37]  f g e d  f g e d
    WF8B[18]  a b c dp a b c dp
37 i 18 to nr pinow KL46 odpowiadajce pinom 5 i 6 w LCD_s401 na pozycjach 0 i 1 w LCD_FRONT_PIN
przypisujemy wartosci np. LCD_S_D czyli 0x11 bo to COM0, a tam segment D, i np LCD_S_E 0x22 bo to COM1 [fazy itd, juz ustawilismy wczesniej]	
	*/ 
if(digit > 4)
{sLCD_Error(WRONG_POSITION); }
else{
	switch(value){	
		case 0x00: //zeby wyswietlic 0 zapalamy segmenty a,b,c,d,e,f
		LCD->WF8B[ LCD_Front_Pin[((2*digit)-2)] ]= (LCD_S_D | LCD_S_E | LCD_S_F);
		LCD->WF8B[ LCD_Front_Pin[((2*digit)-1)] ]= (LCD_S_A | LCD_S_B | LCD_S_C );
    break;		
	  
	  case 0x01:
		LCD->WF8B[LCD_Front_Pin[(2*digit)-2]] = (LCD_CLEAR);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
    break;		
	
		case 0x02:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B);
		break;
		
	  case 0x03:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
		break;
	
		case 0x04:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
		break;
		
	  case 0x05:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_D | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_C);
		break;
		
	  case 0x06:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_C);
		break;
	
		case 0x07:	
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_CLEAR);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
		break;
	
		case 0x08:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
		break;
	
		case 0x09:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
		break;
	
		case 0x0A:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_E | LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
		break;
	
		case 0x0B:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_C);
		break;
	
		case 0x0C:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_F);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
		break;
	
		case 0x0D:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
		break;
	
		case 0x0E:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E | LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
		break;
		
		case 0x0F:
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_E | LCD_S_F | LCD_S_G);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
		break;
		
		default:
		sLCD_Error(WRONG_VALUE);
		break;
  }
 }
}
//*****************************************************************************************************
//obsluga kropek 
 void sLCD_dp(uint8_t toggle, uint8_t digit){
//toggle 1 wlacza, 0 wylacza
//digit - na ktorej pozycji, 4 pozycja to colon[dwukropek] 	
	switch (toggle){	 
	 case 1: 	LCD->WF8B[LCD_Front_Pin[(2*digit)-1] ] |= LCD_S_DP;
            break;
	 case 0: LCD ->WF8B[LCD_Front_Pin[(2*digit)-1] ] &= (~LCD_S_DP); 
 }	
}
//*****************************************************************************************************
//wyswietla napis Err i  kod bledu
 void sLCD_Error(uint8_t errcode){
//LITERA E
LCD->WF8B[LCD_FRONT0] = (LCD_S_D | LCD_S_E | LCD_S_F |LCD_S_G); 
LCD->WF8B[LCD_FRONT1] = LCD_S_A ;
//jedno r
LCD->WF8B[LCD_FRONT2] = (LCD_S_E | LCD_S_G);	
LCD->WF8B[LCD_FRONT3] = 0; //czyszcze pozostalosc tego co wczesniej na wyswietlaczu mogloby byc
//drugie r
LCD->WF8B[LCD_FRONT4] = (LCD_S_E | LCD_S_G);
LCD->WF8B[LCD_FRONT5] = 0; //czyszcze pozostalosc tego co wczesniej na wyswietlaczu mogloby byc
//tworze kod bledu WRONG_POSITION=1 WRONG_VALUE=2 WRONG_FUNCTION=3
switch(errcode)
	{
	 case 1:
		sLCD_set(0,4);
	 break;
	 
	 case 2:
		sLCD_set(1,4);
	 break;
	 
	 case 3:
		sLCD_set(2,4);	 
	}	
}
//***************************************************************************************************** 
//funkcja do wyswietlania wartosci w zadanym formacie
void sLCD_hdob(uint8_t choose,int value) //hexadecimal=1 decimal=2 octal=3 binary=4
{
switch(choose)
	{
	case 1:
				if(value >0xffff)
					{sLCD_Error(WRONG_VALUE);}
				else{
					sLCD_set(value/0x1000, 1); 
					sLCD_set((value%0x1000)/0x100, 2);
					sLCD_set((value%0x100)/0x10, 3);
					sLCD_set(value%0x10, 4);
				  }
				break;
	case 2:
				if(value>9999)
					{sLCD_Error(WRONG_VALUE);}
				else{
					sLCD_set(value/1000, 1); //w komentarzu nie uzywajac operacji %
					sLCD_set((value%1000)/100,2);//( (decimal-(decimal/1000)*1000)/100, 2);
					sLCD_set((value%100)/10 ,3);//( (decimal-(decimal/100)*100)/10, 3);
					sLCD_set(value%10,4);//( (decimal - (decimal/10)*10)/1  ,4);	
				  }
				break;	
	case 3:
				if(value>07777)
					{sLCD_Error(WRONG_VALUE);}
				else{
					sLCD_set( value/01000 ,1);
					sLCD_set( (value%01000)/0100 ,2);
					sLCD_set( (value%0100)/010 ,3);
					sLCD_set( value%010 ,4);
				  }
				break;
  
	case 4:
				if(value>15)
					{sLCD_Error(WRONG_VALUE);}	
				else{
					sLCD_set(value/8 ,1);
					sLCD_set( (value%8)/4 ,2);	
					sLCD_set( (value%4)/2 ,3);
					sLCD_set( value%2 ,4);
					}
				break;
	default:
		sLCD_Error(WRONG_FUNCTION);
	}	
	
	
}
//*****************************************************************************************************
/*----------------------------------------------
Simple counter without using buttons
----------------------------------------------*/

void sLCD_SimpleCounter(void)
{
static uint16_t count = 0; //to avoid global variable

if (count >0xffff){count=0;}	
sLCD_hdob(HEX_VALUE, count);
count++;	
	
}
