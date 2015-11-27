#include <ArduinotechGSMShield.h>
#include <SoftwareSerial.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#define LED 7
#define VENTILATOR 8
#define SERVICE_NUMBER "739822476"

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

//Vstupn� parametry a inicializace shieldu

void setup()
{
	pinMode(RELE1, OUTPUT);
	digitalWrite(RELE1, LOW);
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	modul.begin();
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

}

//Hlavn� smy�ka � �ek� na p��chod SMS s obsahem Teplota?
//Jakmile je takov� SMS rozpozn�na, zm��� se teplota a ode�le
//se SMSka s obsahem Teplota:xx.x st.C
//Smy�ka hl�d� p�ekro�en� teploty (zde pevn� 30 st.), jakmile
//je teplota p�ekro�en�, ode�le se SMS, zapne se ventil�tor
//pokud se teplota vr�t� pod 25 st., vypne se ventil�tor
//Ventil�tor = RELE1 na D8

void loop()
{
	//m�� periodicky teplotu
	sensors.requestTemperatures();
	teplota = sensors.getTempC(insideThermometer);
	
	infoStatus = modul.checkCallAndSMS();
	if (infoStatus == 2)
	{
		number = modul.getNumber();
		Serial.println("SMS from:" + number);

		if (modul.getSMSContent() == "Teplota?")
		{
			modul.sendSMS(number,"Teplota=" + String(teplota) + " st.C");
		}
	}
	//zkontroluj teplotu a je-li vy��� ne� 30 st., pak po�li SMS
	//nastav p��znak, �e SMS byla posl�na, a� nechod� furt dookola
	//zapni ventil�tor
	//vychozi stav odpov�d� poslan� SMS notification=true - kdyby
	//bylo vychoz� false, po resetu by se hned odeslala SMSka pokud by teplota 
	//byla v norm�lu
	if ((teplota > 30.0)&&(notificationSent == false))
	{
		modul.sendSMS(SERVICE_NUMBER, "Pozor, teplota byla prekrocena!!!");
		notificationSent = true;
		Serial.println F("Teplota prekrocena!!! Odeslana SMS.");
		digitalWrite(VENTILATOR, HIGH);
		Serial.println F("Ventilator byl zapnut");
	}
	//vypni ventilator
	if ((teplota < 24.0) && (notificationSent == false))
	{
		modul.sendSMS(SERVICE_NUMBER, "Teplota v normalu");
		notificationSent = true;
		digitalWrite(VENTILATOR, LOW);
		Serial.println F("Ventilator byl vypnut");
	}
	//sma� p��znak odeslan� SMSky, jakmile se teplota p�ibl��
	//k hranici sepnut� venti�toru
	if ((teplota > 29.0)&&(teplota < 30.0))
	{
		notificationSent = false;
	}
	//pokud je teplota vy��� jak 35 st. na nic ne�ekej
	//a zapni ventil�tor - o�et�en� stavu po restartu
	if (teplota > 35.0) digitalWrite(VENTILATOR, HIGH);
	while (Serial.available() > 0) Serial.read();
	if (modul.isConnected() == false)
	{
		digitalWrite(LED, LOW);
		modul.SIM800Init();
		digitalWrite(LED, HIGH);
	}
	delay(1000);
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
