/*

CAPTEUR DE TEMPÉRATURE ET D’HUMIDITÉ DHT22 SUR ESP8266
AVEC AFFICHAGE SUR UN ÉCRAN OLED
======================================================

# DESCRIPTION DU PROGRAMME
  Ce programme lit la température et l’humidité de l’air à l’aide d’un
  capteur DHT22 et les affiche sur un écran OLED 128×64 I²C à l’aide
  d’un ESP8266. Il envoie aussi les valeurs sur dweet.io à l’aide
  de la bibliothèque https://github.com/gamo256/dweet-esp et les valeurs
  sont visibles (pas longtemps) à l’adresse
  https://dweet.io/follow/THING-NAME
  Plus d’infos à https://dweet.io/

# PARAMÈTRES
  Les paramètres sont définis dans le fichier ESPSettings.h qu’il faut
  créer manuellement et enregistrer dans le même répertoire que ce programme.
  Le fichier doit contenir les informations suivantes :

char* ssid       = "...";
char* password   = "...";
char* THING_NAME = "..."; // pour le Dweet https://dweet.io/follow/THING-NAME

# CÂBLAGE
  Voir la photo à
  http://ouilogique.com/esp8266-dht22-oled/

# BIBLIOTHÈQUES
  Les bibliothèques suivantes sont disponibles dans l’IDE Arduino sous
  Croquis/Inclure une bibliothèque/Gérer les bibiothèques

  - Adafruit SSD1306
  - Adafruit GFX Library
  - Adafruit DHT Unified

  La bibliothèque pour dweeter est disponible ici :
  https://github.com/gamo256/dweet-esp

Octobre 2016, ouilogique.com

*/

#include "ESPSettings.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 16 // OLED_RESET=16 pour ESP8266 // OLED_RESET=4 pour Arduino Nano
Adafruit_SSD1306 display( OLED_RESET );
#if( SSD1306_LCDHEIGHT != 64 )
#error( "Height incorrect, please fix Adafruit_SSD1306.h!" );
#endif

// Broche de données du DHT22
// Si on utilise GPIO 2 ou 16, la LED correspondante de l’ESP flashera à chaque lecture !
#define DHTPIN  14

// Uncomment the type of sensor in use:
// #define DHTTYPE        DHT11     // DHT 11
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
// #define DHTTYPE        DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

#include "dweetESP8266.h"
dweet dweetClient;

#define avecSerial true


void initSerial()
{
  #if avecSerial
    Serial.begin( 115200 );
  #endif
}


void initEcran()
{
  display.begin( SSD1306_SWITCHCAPVCC, 0x3C );
  display.clearDisplay();
  display.setTextColor( INVERSE );
  display.setTextSize( 2 );
  display.setCursor( 35, 0 );
  display.print( F( "DHT22" ) );
  display.display();
}


void initDHT()
{
  dht.begin();

  sensor_t sensor;

  #if avecSerial
    Serial.println( "DHTxx Unified Sensor Example" );
    // Print temperature sensor details.
    dht.temperature().getSensor( &sensor );
    Serial.println( F( "------------------------------------" ) );
    Serial.println( F( "Temperature" ) );
    Serial.print  ( F( "Sensor:       " ) ); Serial.println( sensor.name );
    Serial.print  ( F( "Driver Ver:   " ) ); Serial.println( sensor.version );
    Serial.print  ( F( "Unique ID:    " ) ); Serial.println( sensor.sensor_id );
    Serial.print  ( F( "Max Value:    " ) ); Serial.print  ( sensor.max_value);  Serial.println( F( " *C" ) );
    Serial.print  ( F( "Min Value:    " ) ); Serial.print  ( sensor.min_value);  Serial.println( F( " *C" ) );
    Serial.print  ( F( "Resolution:   " ) ); Serial.print  ( sensor.resolution); Serial.println( F( " *C" ) );
    Serial.println( "------------------------------------" );
    // Print humidity sensor details.
    dht.humidity().getSensor( &sensor );
    Serial.println( F( "------------------------------------" ) );
    Serial.println( F( "Humidity" ) );
    Serial.print  ( F( "Sensor:       " ) ); Serial.println( sensor.name );
    Serial.print  ( F( "Driver Ver:   " ) ); Serial.println( sensor.version );
    Serial.print  ( F( "Unique ID:    " ) ); Serial.println( sensor.sensor_id );
    Serial.print  ( F( "Max Value:    " ) ); Serial.print  ( sensor.max_value );  Serial.println( F( "\x25" ) ); // \x25 = signe %
    Serial.print  ( F( "Min Value:    " ) ); Serial.print  ( sensor.min_value );  Serial.println( F( "\x25" ) ); // \x25 = signe %
    Serial.print  ( F( "Resolution:   " ) ); Serial.print  ( sensor.resolution ); Serial.println( F( "\x25" ) ); // \x25 = signe %
    Serial.println( "------------------------------------" );
  #endif

  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}


void initDweet()
{
  dweetClient.wifiConnection( ssid, password );
}


void getTempAndHum()
{
  // Préparation de l’écran OLED
  display.clearDisplay();
  display.setTextColor( INVERSE );
  display.setTextSize( 2 );
  display.setCursor( 35, 0 );
  display.print( F( "DHT22" ) );

  // Préparation Serial
  #if avecSerial
    Serial.print( "\n" );
  #endif


  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent( &event );
  if( isnan( event.temperature ) )
  {
    // Serial
    #if avecSerial
      Serial.println( F( "NaN *C" ) );
    #endif

    // OLED Display
    display.setCursor( 35, 19 );
    display.print( F( "NaN C" ) );
    display.setCursor( 74, 9 );
    display.print( F( "." ) ); // Signe degré (°), simulé avec un point

    // Dweet
    // Pas de dweet si T = NaN
  }
  else
  {
    // Serial
    #if avecSerial
      Serial.print( event.temperature, 1 );
      Serial.println( F( "*C" ) );
    #endif

    // OLED Display
    display.setCursor( 28, 19 );
    display.print( event.temperature, 1 );
    display.print( F( " C" ) );
    display.setCursor( 79, 9 );
    display.print( F( "." ) ); // Signe degré (°), simulé avec un point

    // Dweet
    dweetClient.add( "DHT22_Temperature" , String( event.temperature ) );
  }

  // Get humidity event and print its value.
  // !! Si on demande la température (event.temperature) alors qu’on a
  // utilisé dht.humidity().getEvent( &event );, c’est la valeur de
  // l’humidité que l’on obtient...
  dht.humidity().getEvent( &event );
  if( isnan( event.relative_humidity ) )
  {
    // Serial
    #if avecSerial
      Serial.println( F( "NaN \x25" ) );
    #endif

    // OLED Display
    display.setCursor( 35, 40 );
    display.print( F( "NaN \x25" ) );

    // Dweet
    // Pas de dweet si H = NaN
  }
  else
  {
    // Serial
    #if avecSerial
      Serial.print( event.relative_humidity, 1 );
      Serial.println( F( " \x25" ) ); // signe %
    #endif

    // OLED Display
    display.setCursor( 28, 40 );
    display.print( event.relative_humidity, 1 );
    display.print( F( " \x25" ) ); // signe %

    // Dweet
    dweetClient.add( "DHT22_Relative_Humidity" , String( event.relative_humidity ) );
  }

  // Ajout séparateur de mesures Serial
  #if avecSerial
    Serial.print( "\n" );
  #endif

  // Ajout de fioritures et mise à jour de l’écran OLED
  int16_t px = 0;
  int16_t py = 6;
  int16_t dx = 27;
  display.drawLine( 0, py,   dx, py,   WHITE );
  display.drawLine( 0, py+1, dx, py+1, WHITE );
  display.drawLine( display.width()-1-dx, py,   display.width()-1, py,   WHITE );
  display.drawLine( display.width()-1-dx, py+1, display.width()-1, py+1, WHITE );
  // px = 27;
  // display.drawLine( px, 0, px, display.height()-1, WHITE );
  // px = 100;
  // display.drawLine( px, 0, px, display.height()-1, WHITE );
  display.display();

  // Envoie la valeur de millis() pour le debug
  dweetClient.add( "ESP_millis" , String( millis() ) );

  // Mise à jour des dweets
  dweetClient.sendAll( THING_NAME );
}


void setup()
{
  initSerial();
  initEcran();
  initDHT();
  initDweet();
}


void loop()
{
  static long T1 = millis();
  long dT = millis() - T1;
  if( dT > 10000 )
  {
    getTempAndHum();
    T1 = millis();
    #if avecSerial
      Serial.printf( "\ndT = %d ms\n", dT );
    #endif
  }
}
