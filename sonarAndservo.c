
/** \file	File contains all definitions of Sonar and Servo functions 
    *\brief     TPM1->CH0 PTB0 Echo	TPM1->CH1 PTB1 Trigg	TPM2->CH0 PTB2 Servo
    *\author Grzegorz Kosiec and Maciej Malczyk
*/
/*
!WARNING
Pins, which can not be used for Servo[in High State during Reset, and loading the program, so they can destroy servo
PTC1
PTE23
PTA4
PTD5
*/
#include "SonarAndServo.h"
#include "MKL46Z4.h"
#include "sLCD.h"

//static before global variable means that it can't be used from different file by "extern"
static int Measurement_With_Servo = 0;					//1 - with servo, 0 - without
static volatile int Distance;										//to read

/**\brief Initialization of Clocks, TPM, TPM channels and pin used.
*/
void SonarandServo_init(void)
{
						/* PINS */
	    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
			PORTB->PCR[ECHO_PIN] &=~PORT_PCR_MUX_MASK;		//cleaning just in case
			PORTB->PCR[ECHO_PIN] |=PORT_PCR_MUX(3);
			PORTB->PCR[TRIGGER_PIN] &=~PORT_PCR_MUX_MASK;		
			PORTB->PCR[TRIGGER_PIN] |=PORT_PCR_MUX(3);
			PORTB->PCR[SERVO_PIN] &=~PORT_PCR_MUX_MASK;	
			PORTB->PCR[SERVO_PIN] |=PORT_PCR_MUX(3);
						/* END OF PINS*/
	
	//**********************************************************************************************************************
	//*************************** basic tpm config for TPM1 and TPM2
	//**********************************************************************************************************************

	SIM->SCGC6 |=SIM_SCGC6_TPM1_MASK;					//TPM1 CLOCK ON
	SIM->SOPT2 |=SIM_SOPT2_TPMSRC_MASK;	//set TPM source as MCG Internal Reference clock
	SIM->SCGC6 |=SIM_SCGC6_TPM2_MASK;					//TPM2 CLOCK ON

	MCG->C1 = MCG_C1_CLKS(0x1);  //Internal reference clock is selected as MCGOUTCLK
	MCG->C2 |= MCG_C2_IRCS_MASK ;	//Fast internal reference clock selected. 2MHz
	MCG->C1 |= MCG_C1_IRCLKEN_MASK ;			//ENABLE A REFERENCE CLOCK 
		
	TPM1->SC &= ~TPM_SC_CPWMS_MASK;//I clean this bit, so TPM1 in the up-counting mode.  
	TPM1->SC |=  TPM_SC_CMOD(1)  | TPM_SC_PS(1);//prescaler divide factor = 2 so f=2MHz/2=1MHz Pulse duration 1/1MHz=1us
	TPM1->SC &=  ~TPM_SC_TOIE_MASK; //timer overflow interrupt disable
	TPM1->CNT |= TPM_CNT_COUNT(0);			
	TPM1->MOD = 32200;				//20MS+[IN WORST CASE] MIN SERVO PERIOD+2.2MS[MAX SERVO IMPULS]=32.2MS
	                          //TURN SERVO BEFORE NEXT TRIGGER!!! 
		
	TPM2->SC &= ~TPM_SC_CPWMS_MASK;
	TPM2->SC |=  TPM_SC_CMOD(1)  | TPM_SC_PS(1);
	TPM2->CNT |= TPM_CNT_COUNT(0);		
	TPM2->MOD = 10000; //min servo 	period
/*END OF GENERAL TPM CONFIG*/
 //*************************************   setting the servo channel, creating impulse, epwm high true pulses   *********************************************
	TPM2->CONTROLS[SERVO_CHANNEL].CnSC &= ~TPM_CnSC_MSA_MASK & ~TPM_CnSC_ELSA_MASK ;					// epwm high true pulses
	TPM2->CONTROLS[SERVO_CHANNEL].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;				
	TPM2->CONTROLS[SERVO_CHANNEL].CnV = NEUTRAL_POSITION;		


//*************************************   setting the TRIGGER channel, creating impulse, epwm high true pulses   ********************************************
	TPM1->CONTROLS[TRIGGER_CHANNEL].CnSC &= ~TPM_CnSC_MSA_MASK & ~TPM_CnSC_ELSA_MASK ;			
	TPM1->CONTROLS[TRIGGER_CHANNEL].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;				
	TPM1->CONTROLS[TRIGGER_CHANNEL].CnV = 10;		
	

//**************************************** ECHO channel input capture  **********************************************************
	
	TPM1->CONTROLS[ECHO_CHANNEL].CnSC &= ~TPM_CnSC_MSA_MASK & ~TPM_CnSC_MSB_MASK;	 
	TPM1->CONTROLS[ECHO_CHANNEL].CnSC |= TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK;																					
	TPM1->CONTROLS[ECHO_CHANNEL].CnSC |= TPM_CnSC_CHIE_MASK | TPM_CnSC_CHF_MASK;		
	
	NVIC_ClearPendingIRQ(TPM1_IRQn);				//the only interrupt genretedy overall
	NVIC_EnableIRQ(TPM1_IRQn);	
	NVIC_SetPriority(TPM1_IRQn, 3);
}

//*************************************************************************************************************************

/**\brief Rotates servo towards the left side by 12 degrees.
*/
void Servo_Move_Left (void)
{
		if (TPM2->CONTROLS[SERVO_CHANNEL].CnV < MAX_LEFT_POSITION)
			{	
				TPM2->CONTROLS[SERVO_CHANNEL].CnV +=100; //1400us is 165 degrees 100us is 12 degrees
			}
}

/**\brief Rotates servo towards the right side by 12 degrees.
*/
void Servo_Move_Right(void)
{
		if (TPM2->CONTROLS[SERVO_CHANNEL].CnV > MAX_RIGHT_POSITION)
		{
			TPM2->CONTROLS[SERVO_CHANNEL].CnV -=100; //1400us is 165 degrees 100us is 12 degrees
		}
}

/**\brief Rotates servo to its neutral position - straight forward.
*/
void Servo_Move_Neutral (void)
{
	TPM2->CONTROLS[SERVO_CHANNEL].CnV = NEUTRAL_POSITION;
}

/**\brief Rotates servo from left to right or from right to left by degrees. Function uses Servo_Move_Right() and Servo_Move_Left() funtions.
*/
void Servo_Rotate(void)
{
	static int Servo_go_left = 0;			//0 - go right 1-go left
	
		if (TPM2->CONTROLS[SERVO_CHANNEL].CnV == MAX_LEFT_POSITION)
		{
			Servo_go_left = 0;
		}
		
	  if (TPM2->CONTROLS[SERVO_CHANNEL].CnV == MAX_RIGHT_POSITION)
		{
			Servo_go_left = 1;
		}
		
	switch(Servo_go_left)
		{
			case 0:
				Servo_Move_Right();
				break;
			
			case	1:
				Servo_Move_Left();
				break;
		}
}

/**\brief Rotates serv by degree given in deg. Function is called with one int parameter.
*\param int degrees
*\warning degrees [0,165] range
*/
void TurnServoOnPosition(int degrees) //turns on position specified by degrees
{
if((degrees<=165)&&(degrees>=0)) 
 {
   TPM2->CONTROLS[SERVO_CHANNEL].CnV =MAX_RIGHT_POSITION + (degrees*85)/10;	//[165stopni - 1400us, degrees stopni - x] 
																											//x=degrees*1400/165=degrees*8.5=(degrees*85)/10
 }																											
}


/**\brief Interrupt handler. Decides whether measure the distance or not. 
*/
void TPM1_IRQHandler (void)
{
static int stop = 0;
static int Time_Measured = 0;		//time between sonar input capture edges
static int rising_edge_value = 0;		//scaled distance not for read	

  if(TPM1->CONTROLS[ECHO_CHANNEL].CnSC & TPM_CnSC_CHF_MASK)
  {
		switch (stop) //0 - measurement start (rising edge) 1-measurement end (falling edge)
		{
				case 0: //measurement start 
					
				rising_edge_value = TPM1->CONTROLS[ECHO_CHANNEL].CnV; 
				TPM1->CONTROLS[ECHO_CHANNEL].CnSC |= TPM_CnSC_CHF_MASK ;
				stop = 1;
				break;
				
				case 1: //measurement has ended
						
					Time_Measured = TPM1->CONTROLS[ECHO_CHANNEL].CnV - rising_edge_value;
				 //odleglosc [cm] = T1 [us] /  58	
				 //odlegosc[mm] =T1[us]/5.8 - double nedeed or as below
				 //odlegosc[mm] =(T1[us]*10)/58	- this is how i've done it 
				  Distance = (Time_Measured * 10 / 58);
					
				  if (Measurement_With_Servo == 1)
									{
										Servo_Rotate();
									}
				
				TPM1->CONTROLS[ECHO_CHANNEL].CnSC |= TPM_CnSC_CHF_MASK;	
				stop = 0;						
				break;
		}
			
  }
	
}



/**\brief Starts the sonar measurement.
*\warning First choose with or without servo bu using Servo_And_Sonar_Measurement() or Only_Sonar_Measurement()
*/
void Sonar_Measurement_Start (void)
{
	TPM1->CONTROLS[ECHO_CHANNEL].CnSC |= TPM_CnSC_CHIE_MASK; //interrupt enable
}

/**\brief Stops the sonar measurement.
*/
void Sonar_Measurement_Stop (void)
{
	TPM1->CONTROLS[ECHO_CHANNEL].CnSC &= ~TPM_CnSC_CHIE_MASK; //interrupt disable
}


/**\brief Function which turns on measurement WITH servo. 
*/
void Servo_And_Sonar_Measurement(void)
{
 Measurement_With_Servo = 1;
}

/**\brief Function which turns on measurement WITHOUT servo. 
*/
void Only_Sonar_Measurement (void)
{
	Measurement_With_Servo = 0;
}

/**\brief Returns the angle of current servo position given in us.
*\return int Angle
*/

int Servo_Get_Angle_us (void)
{
	return TPM2->CONTROLS[SERVO_CHANNEL].CnV - MAX_RIGHT_POSITION;
}

/**\brief Returns the angle of current servo position given in deg. 
[0- max right Position, 165 - max left position, 82 - neutral position ] 
*\return int
*/
int Servo_Get_Angle_deg (void)
{
	return ((TPM2->CONTROLS[SERVO_CHANNEL].CnV - MAX_RIGHT_POSITION + 5) *10/ 85); //+5, to round up, because int cut value after full stop 
}

/**\brief Returns current distance captured by sonar.
*/
int Sonar_Get_Distance (void)
{
	return Distance;
}


/**\brief This function is responsible for taking action after something was detected in the predefined level of accepted distance. 
Currently it stops the servo and measures and returns 1 if sth is detected. When the threat is out of sight ( the distance is above the minimum ) 
servo will be automatically turned on.
*\param int dangerous_distance in milimeters
*\return int object_detected=1 if sth is in given dangerous_distance
*/
int Danger_Detected(int dangerous_distance)
{ int object_detected;
	
	if (Distance <= dangerous_distance )
	{
		Only_Sonar_Measurement();
		object_detected=1;
	}
	else
  {
	  Servo_And_Sonar_Measurement();
		object_detected=0;
	}
	
	return (object_detected);
}
