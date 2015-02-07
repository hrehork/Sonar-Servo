#include "MKL46Z4.h"                   	
#include "sLCD.h"
#include "SonarAndServo.h"

/** \author Grzegorz Kosiec and Maciej Malczyk */


int main(void)
{
	
	sLCD_Init();
	SonarandServo_init();
	Servo_And_Sonar_Measurement();
	Sonar_Measurement_Start();

while(1)
	{
		if(Danger_Detected(100))
		{sLCD_hdob(DECIMAL_VALUE, Servo_Get_Angle_deg());}
		else
		sLCD_hdob(DECIMAL_VALUE, Sonar_Get_Distance());	
	}

}	

