#include <ArduinotechGSMShield.h>
#include <SoftwareSerial.h>
#define LED 7
#define SERVIS_NUMBER "739822476"

//Vytvo�en� instance t��dy AGS

AGS modul(1);
uint8_t infoStatus;
String number;

//definice kombinac� kontakt�
String status[8] =
{ 
	"K1=1,K2=1,K3=1",
	"K1=1,K2=1,K3=0",
	"K1=1,K2=0,K3=1",
	"K1=1,K2=0,K3=0",
	"K1=0,K2=1,K3=1",
	"K1=0,K2=1,K3=0",
	"K1=0,K2=0,K3=1",
	"K1=0,K2=0,K3=0" 
};
bool SMSsent;
String SMScontent;
String oldContent;
uint16_t analog;

//Vstupn� parametry a inicializace shieldu

void setup()
{
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	modul.begin();
	digitalWrite(LED, HIGH);
	oldContent = status[0];
}

/*Hlavn� smy�ka � m��� nap�t� na analogov�m vstupu A0 
p�i jeho zm�n� vyhodnot� stav a ode�le SMS.
Pak je hl�d�n p��chod p��choz�ho vol�n� a na SERVIS_NUMBER,
je odesl�na SMSka s aktu�ln�m stavem kontakt� - analogov�ho vstupu A0,
pokud se volaj�c� ��slo shoduje se SERVIS_NUMBER
*/


void loop()
{
	//m�� A0 a zji��uj kombinace, nachystej obsah SMS
	analog = analogRead(A0);
	if (analog < 340) SMScontent = status[0];
	if ((analog < 400) && (analog > 360)) SMScontent = status[1];
	if ((analog < 435) && (analog > 390)) SMScontent = status[2];
	if ((analog < 540) && (analog > 495)) SMScontent = status[3];
	if ((analog < 490) && (analog > 450)) SMScontent = status[4];
	if ((analog < 620) && (analog > 580)) SMScontent = status[5];
	if ((analog < 720) && (analog > 670)) SMScontent = status[6];
	if (analog > 1000) SMScontent = status[7];
	
	//porovnej obsah p�ede�l� SMS a pokud je zm�na, ulo� ji a ode�li
	if (oldContent != SMScontent)
	{
		oldContent = SMScontent;
		modul.sendSMS(SERVIS_NUMBER, SMScontent);
	}

	
	infoStatus = modul.checkCallAndSMS();
	if (infoStatus == 1)
	{
		number = modul.getNumber();
		Serial.println("Call from:" + number);
		modul.callEnd();

		if (number == SERVIS_NUMBER)
		{
			modul.sendSMS(SERVIS_NUMBER, SMScontent);
		}
		else modul.callEnd();
	}
	delay(500);
}

