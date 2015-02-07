#ifndef SonarAndServo_h 
#define SonarAndServo_h 

/**\file	File contains all declarations of Sonar and Servo functions 
*/

#include "MKL46Z4.h"
	

#define NEUTRAL_POSITION 1500	//angle of neutral positoin
#define MAX_LEFT_POSITION 2200	
#define MAX_RIGHT_POSITION 800

#define ECHO_PIN	0		//PORT B 
#define TRIGGER_PIN	1		//PORT B
#define SERVO_PIN	2		//PORT B

#define ECHO_CHANNEL 0
#define TRIGGER_CHANNEL 1
#define SERVO_CHANNEL 0

void TPM1_IRQHandler (void);

void SonarandServo_init (void);

void Servo_Move_Left (void);		
void Servo_Move_Right(void);	
void Servo_Move_Neutral(void);
void Servo_Rotate (void);			//rotate by angle defined
void TurnServoOnPosition(int degrees);			//rotate by degree given in deg(0 - 164)
int Servo_Get_Angle_us (void);			//returns angle of current servo position in us
int Servo_Get_Angle_deg (void);			//returns angle of current servo position	in deg
int Sonar_Get_Distance (void);		//returns current distance

void Servo_And_Sonar_Measurement(void);		//measurement with servo
void Only_Sonar_Measurement(void);			//measurement without servo
void Sonar_Measurement_Start (void);		
void Sonar_Measurement_Stop (void);	

int Danger_Detected(int dangerous_distance);
	

#endif
