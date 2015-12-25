#include <TimerOne.h>
#include <ArduinotechGSMShield.h>
#include <SoftwareSerial.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#define LED 7
#define VENTILATOR 8
#define SERVICE_NUMBER "739822476"
#define PERIODE 600

//definice OneWire pro p�ipojen� DS18B20
#define ONE_WIRE_BUS 10
#define TEMPERATURE_PRECISION 9 

//definice pro teplom�r
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
float teplota;
String termIN;

//Vytvo�en� instance t��dy AGS

AGS modul(1);
uint8_t infoStatus;
String number;
bool notificationSent;
bool afterReset;
uint32_t ticker;
//T-mobil
String APN = "internet.t-mobile.cz";
//O2 a VODAFONE
//String APN = "internet";
String thingSpeak = "api.thingspeak.com/update?api_key=54DLXE3I1PETR61C&field1=";

//Vstupn� parametry a inicializace shieldu

void setup()
{
	pinMode(RELE1, OUTPUT);
	digitalWrite(RELE1, LOW);
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	modul.begin();
	modul.GPRSInit(APN);
	notificationSent = true;
	digitalWrite(LED, HIGH);

	//teplom�r
	sensors.begin();
	if (sensors.getDeviceCount()>0) Serial.println F("Teplomer nalezen!");
	else Serial.println F("Teplomer NENALEZEN!!!");
	oneWire.reset_search();
	oneWire.search(insideThermometer);
	//nastav rozliseni
	sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
	sensors.requestTemperatures();
	printAddress(insideThermometer);
	printTemperature(insideThermometer);
	Serial.println F("****************************************");
	while (Serial.available() > 0) Serial.read();

	//inicializuj timer
	ticker = 0;
	Timer1.initialize(1000000);
	Timer1.attachInterrupt(timeCounter);

	//prvn� pokus o komunikaci
	sensors.requestTemperatures();
	teplota = sensors.getTempC(insideThermometer);
	Serial.println(thingSpeak + String(teplota,1));
	sendGET(thingSpeak + String(teplota, 0));
	
}

/*Hlavn� smy�ka � zm��� teplotu a pak ji p�es HTTP GET
GPRS prezentuje do kan�lu Thingspeak.com s t�mto data
feed update:
GET api.thingspeak.com/update?api_key=54DLXE3I1PETR61C&field1=xx
xx = teplota
*/

void loop()
{
	//m�� periodicky teplotu
	sensors.requestTemperatures();
	teplota = sensors.getTempC(insideThermometer);
	//ode�li ka�dou hodinu data
	if (ticker == PERIODE)
	{
		ticker = 0;
		sendGET(thingSpeak + String(teplota,1));
	}
}

void sendGET(String get)
{
	if (modul.sendDataGPRS(get) != "COMMUNICATION FAILURE")
	{
		Serial.println F("Data ulozena");
	}
	//nepoda�ilo se zakomunikovat, opakuj za 10s
	//inicializuj modul
	else
	{
		Serial.println F("Chyba komunikace, restart ...");
		ticker = PERIODE - 10;
		modul.SIM800Init();
		modul.GPRSInit(APN);
	}
}
//pro teplom�r
void printAddress(DeviceAddress deviceAddress)
{
	Serial.print F("Adresa teplomeru:");
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
	Serial.println("");
}

void printTemperature(DeviceAddress deviceAddress)
{
	float tempC = sensors.getTempC(deviceAddress);
	Serial.print F("Teplota: ");
	Serial.print(tempC);
	Serial.write(176);
	Serial.println("C");
}
//�asova�
void timeCounter()
{
	ticker++;
}
