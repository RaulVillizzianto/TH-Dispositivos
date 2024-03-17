#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define PIN_LED 5
#define PIN_CE 10
#define PIN_CSN 9
#define SENSOR_PIN 7
#define LED_PIN 8
//#define DEBUG

//create an RF24 object
RF24 radio(PIN_CE,PIN_CSN);  // CE, CSN

//address through which two modules communicate.
const int INTERVALO_ENTRE_TRABAJO = 1000;
const int TAMANIO_MAXIMO_BUFFER = 32;
const int LARGO_NOMBRE_DISPOSITIVO = 24;

const byte CANAL_ESCRITURA[6] = "00002";
const byte CANAL_LECTURA[6] = "00001";

//Mensajes al servidor
const int MENSAJE_HEARTBEAT = 0;
const int MENSAJE_OBTENER_DISPOSITIVO = 2;

//Ordenes
const int ORDEN_HEARTBEAT = 0;
const int ORDEN_ENCENDER_DISPOSITIVO = 1;
const int ORDEN_APAGAR_DISPOSITIVO = 2;
const int ORDEN_OBTENER_DISPOSITIVO = 3;
const int RELE_PIN = 2;

char nombreDispositivo[LARGO_NOMBRE_DISPOSITIVO] = "MR0001";
char contenido[TAMANIO_MAXIMO_BUFFER];

bool estadoDispositivo = false;
bool escuchando = false;
bool ledEncendido = false;

void setup()
{
  inicializar();
}
void loop()
{
  verificarMensajesEntrantes();
}

void establecerComoReceptor()
{
  if(escuchando == false)
  {
    radio.startListening();
    escuchando = true;
  }
}
void establecerComoTransmisor()
{
  radio.stopListening();
  escuchando = false;
}
void inicializar()
{
  pinMode(LED_PIN,OUTPUT);
	pinMode(RELE_PIN, OUTPUT);
  #ifdef DEBUG
  while (!Serial); 
  {
    Serial.begin(9600);
    digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_PIN, LOW);  
  }
    Serial.println("iniciando..");

  #endif
  if (!radio.begin())
  { 
    while (1)
    {
      #ifdef DEBUG
      Serial.println(F("DEBUG:radio hardware not responding!"));
      #endif
      digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
      delay(1000);                      // wait for a second
      digitalWrite(LED_PIN, LOW);   // turn the LED off by making the voltage LOW
      delay(1000); 

    }
  }
  radio.setDataRate(RF24_2MBPS); // Set the speed of the transmission to the quickest available
  radio.setChannel(125); // Use a channel unlikely to be used by Wifi, Microwave ovens etc

  radio.setRetries(7, 15);
  radio.openWritingPipe(CANAL_ESCRITURA); //00002
  radio.openReadingPipe(1, CANAL_LECTURA); //00001
  radio.setPayloadSize(TAMANIO_MAXIMO_BUFFER);
  radio.setPALevel(RF24_PA_MAX,true);
  #ifdef DEBUG
  //  radio.printPrettyDetails();
  Serial.println("DEBUG:inicializado");
  
  #endif
  for(int i = 0; i < 5; i++);
  {
      digitalWrite(LED_PIN, HIGH);  
      delay(500);                     
      digitalWrite(LED_PIN, LOW);  
      delay(500);    
  }
}
void verificarMensajesEntrantes()
{
  establecerComoReceptor();
  if(radio.available())
  {
    char mensajeString[TAMANIO_MAXIMO_BUFFER];
    sprintf(mensajeString,"");
    int codigo_mensaje = 0;
    
    radio.read(&mensajeString, sizeof(mensajeString));
    sscanf(mensajeString,"%d|%s", &codigo_mensaje, &contenido );
    String cont = contenido;

    String nombreDispositivoApuntado = cont.substring(0, cont.indexOf(','));
    nombreDispositivoApuntado.trim();
    if(strcmp(nombreDispositivoApuntado.c_str(), nombreDispositivo) == 0)
    {
      switch(codigo_mensaje)
      {
        case ORDEN_HEARTBEAT:
        {
          heartBeat();
          break;
        }
        case ORDEN_ENCENDER_DISPOSITIVO:
        {
          encenderDispositivo();
          digitalWrite(LED_PIN, LOW);  
          obtenerDispositivo();
          digitalWrite(LED_PIN, HIGH);  
          delay(1000);                     
          digitalWrite(LED_PIN, LOW);  
          delay(1000);    
          break;
        }
        case ORDEN_APAGAR_DISPOSITIVO:
        {
          apagarDispositivo();
          digitalWrite(LED_PIN, LOW);  
          obtenerDispositivo();
          for(int i = 0; i < 2; i++);
          {
              digitalWrite(LED_PIN, HIGH);  
              delay(250);                     
              digitalWrite(LED_PIN, LOW);  
              delay(250);    
    
          }
          break;
        }
        case ORDEN_OBTENER_DISPOSITIVO:
        {
          obtenerDispositivo();
          break;
        }
      }
    } 
  }
}

void apagarDispositivo()
{
  estadoDispositivo = false;
  digitalWrite(RELE_PIN, LOW);
}

void encenderDispositivo()
{
  estadoDispositivo = true;
  digitalWrite(RELE_PIN, HIGH);
}

void obtenerDispositivo()
{
  char mensajeString[TAMANIO_MAXIMO_BUFFER];
  char t[128];
  sprintf(mensajeString, "%d|%s,%d",MENSAJE_OBTENER_DISPOSITIVO,nombreDispositivo,int(estadoDispositivo));
  sprintf(t, "obtenerDISPOSITIVO => %s",mensajeString);
  enviarInformacion(mensajeString);
}

void heartBeat()
{    
  char mensajeString[TAMANIO_MAXIMO_BUFFER];
  sprintf(mensajeString, "%d|%s",MENSAJE_HEARTBEAT,nombreDispositivo);
  enviarInformacion(mensajeString);
}

void enviarInformacion(char *mensajeString)
{
  establecerComoTransmisor();
  radio.write(mensajeString,TAMANIO_MAXIMO_BUFFER);
  #ifdef DEBUG
  char debugMsg[TAMANIO_MAXIMO_BUFFER];

  sprintf(debugMsg, "Salida => %s",mensajeString);
  Serial.println(debugMsg);
  #endif
}


