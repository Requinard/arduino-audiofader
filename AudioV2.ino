#include <LiquidCrystal.h>
#include <Wire.h>
#include <math.h>
#include <string>
#include <stdlib.h>

#include "main.h"

//GLOBALE VARIABLE. DONT HURT ME SANDER
LiquidCrystal scherm(12, 11, 6, 4, 3, 2);
//

///Setup regelt initialisatie van alle pins.
void setup()
{
	Wire.begin();
	Serial.begin(9600);
	pinMode(A0, INPUT);
}

void loop()
{
	///Variabelen initaliseren.
  unsigned char uitgangMic = 0;
  unsigned char uitgangPC = 255;

  State staat = UIT;

  ///Settings initaliseren
  Settings settings = {0 };


  settings.delayType = LIN;
  settings.fadeTime = 1000;
  settings.speakDelay = 2000;

  //Scherm initialiseren
  LiquidCrystal scherm(12, 11, PIN2, PIN3, PIN4,PIN5);
  scherm.clear();
  scherm.begin(16, 2);
  scherm.print("      AIAS      ");

  Schermstate schermState = FADE;

  delay(2000);

  Serial.println("Succesvol geintialiseerd.");
  
  while(1)
  {
	  //Seriële data verzenden
		char serialBuffer[100] = {0};

		Serial.readBytes(serialBuffer, 100);
		switch(serialBuffer[0])
		{
		//r in request
		case 'r':
			//s in settings
			if(serialBuffer[1]=='s')
			{
				Serial.println("settings");
				Serial.println(settings.fadeTime);
				Serial.println(settings.speakDelay);
				Serial.println(settings.delayType);
			}
			//v in values
			if(serialBuffer[1] == 'v')
			{
				Serial.write(uitgangMic);
				Serial.write(uitgangPC);
			}
			break;
		case 'w':
			if(serialBuffer[1] == 'f')
			{
				char numberbuffer[4] = {0};
				for(short i = 2; i < 2+4; i++)
				{
					numberbuffer[i-2] = serialBuffer[i];
					settings.fadeTime = atoi(numberbuffer);
				}
			}
			else if(serialBuffer[1] == 's')
			{
				char numberbuffer[4] = {0};
				for(short i = 2; i < 2+4; i++)
				{
					numberbuffer[i-2] = serialBuffer[i];
					settings.speakDelay = atoi(numberbuffer);
				}
			}
			else if(serialBuffer[1] == 't')
			{
				if(serialBuffer[2] == '0')
					settings.delayType = LIN;
				else if(serialBuffer[2] == '1')
					settings.delayType = LOGPO;
				else if(serialBuffer[2] == '2')
					settings.delayType = LOGNE;
				else if(serialBuffer[2] == '3')
					settings.delayType = TANH;
			}
			break;
		default:
			break;
		}
		//einde seriele data

		//Begin audio
		bool signaalIn = inputChanged();

		if(staat == UIT && signaalIn)
		{
			Serial.println("Microfoon gaat aan, PC gaat uit.");
			if(changeAudio(true, &uitgangMic, &uitgangPC, settings))
				staat = AAN;
		}
		if(staat == AAN && !signaalIn)
		{
			Serial.println("Microfoon gaat uit, PC gaat aan.");
			if(changeAudio(false, &uitgangMic, &uitgangPC, settings))
				staat = UIT;
		}
		//Einde audio

		//Begin met schermfuncties
		regelScherm(&settings, &schermState);
		//einde schermfuncties
  }
	  
}
// AUDIO
//Kijkt wat er op de A0 pin staat.
bool inputChanged()
{
	if(digitalRead(A0) == HIGH)
		return true;
	if(digitalRead(A0) == LOW)
		return false;
}
//Verandert het geluid.
bool changeAudio(bool mode, unsigned char *uitgangMic, unsigned char *uitgangPC, Settings settings)
{
	bool succes = true;
	bool noInterrupt = true;

	switch(mode)
	{
	case 1:
		for(short i = 0; i <= 255; i++)
		{
			/*if(!inputChanged())
			{
				was het toch maar ruis?
				Serial.println("Input is veranderd toen machine Microfoon aan zou zetten.");

				*uitgangMic = 0;
				*uitgangPC = 255;

				sendWire(uitgangMic, uitgangPC);

				succes = false;

				break;
			}*/
			if(1)
			{
				switch(settings.delayType)
				{
				case LIN:
					*uitgangMic += 1;
					*uitgangPC -= 1;
					break;
				case LOGPO:
					*uitgangMic = 120*log10(i/(double)2);
					*uitgangPC = 255 - 120*log10(i/(double)2);
					break;
				case LOGNE:
					*uitgangMic= 255-(105*log10(-1*i+255));
					*uitgangPC = 105*log(-1*i+255);
				case TANH:
					*uitgangMic = tanh((i-128)/50)*128+128;
					*uitgangPC = 255-(tanh((i-128)/50)*128+128);
				default:
					Serial.print("Error in fade.");
					break;
				}

				sendWire(uitgangMic, uitgangPC);
			}

			delay(settings.fadeTime/(long)255);

			//Serial.println("Een bij microfoon opgeteld");
		}
		if(succes)
		{
			Serial.println("Succesvol microfoon aangezet.");
			*uitgangMic = 255;
			*uitgangPC = 0;
			sendWire(uitgangMic, uitgangPC);
		}
		return succes;
		break;
	case 0:
		//Kijkt voor de speakdelay, neemt anders af.
		for(short j = 0; j < 20; j++)
		{
			if(inputChanged())
			{
				succes = false;
				Serial.println("Iemand heeft gesproken tijdens speakdelay. Microfoon gaat niet uit.");
				break;
			}
			else
				succes = true;

			delay(settings.speakDelay/(long)20);
		}
		if(succes)
		{
			for(short i = 0; i <= 255; i++)
			{
				//Geen spraak tijdens delay? Gaan met die hap
				switch(settings.delayType)
				{
				case LIN:
					*uitgangMic -= 1;
					*uitgangPC += 1;
					break;
				case LOGPO:
					*uitgangMic = 255 - 120*log10(i/(double)2);
					*uitgangPC = 120*log10(i/(double)2);
					break;
				case LOGNE:
					*uitgangMic= 105*log10(-1*i+255);
					*uitgangPC = 255-(105*log10(-1*i+255));
				case TANH:
					*uitgangMic = 255-(tanh((i-128)/50)*128+128);
					*uitgangPC = tanh((i-128)/50)*128+128;
				default:
					Serial.print("Error in fade.");
					break;
				}

				sendWire(uitgangMic, uitgangPC);

				delay(settings.fadeTime/(long)255);
			
				//Serial.println("een van microfoon afgetrokken");
			}
		}
		if(succes && !noInterrupt)
		{
			//shit son we done he'
			Serial.println("Microfoon succesvol uitgezet");
			*uitgangMic = 0;
			*uitgangPC = 255;
			sendWire(uitgangMic, uitgangPC);
		}
		return succes;
		break;
	default:
		Serial.println("Fout in ChangeAudio.");
		break;
	}

	return succes;
}
//Schrijft de databytes naar I2C
void sendWire(unsigned char *uitgangMic, unsigned char *uitgangPC)
{
	Wire.beginTransmission(1);
	Wire.write(*uitgangMic);
	Wire.endTransmission(1);

	Wire.beginTransmission(2);
	Wire.write(*uitgangPC);
	Wire.endTransmission(1);

	Serial.println("wirevalues");
	Serial.write(*uitgangMic);
	Serial.write(*uitgangPC);
}
// EINDE AUDIO

// SCHERM
void regelScherm(Settings *settings, Schermstate *schermState)
{
	updateScherm(settings, schermState);
}
//Print het scherm
void updateScherm(Settings *settings, Schermstate *schermState)
{
	switch(*schermState)
	{
	case FADE:
		scherm.clear();
		scherm.print("Fadetime");
		scherm.print(settings->fadeTime);
		break;
	case DELAY:
		scherm.clear();
		scherm.println("Speakdelay");
		scherm.println(settings->speakDelay);
		break;
	case TYPE:
		scherm.clear();
		scherm.println("Fadetype");

		if(settings->delayType == LOGPO)
			scherm.println("LOGPO");
		else if (settings->delayType == LIN)
			scherm.println();
		break;
	}
}
// EIND SCHERM