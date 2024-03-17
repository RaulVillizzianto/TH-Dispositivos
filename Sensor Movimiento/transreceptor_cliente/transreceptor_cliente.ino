//Include Libraries
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
const int TAMANIO_MAXIMO_BUFFER = 64;
const int LARGO_NOMBRE_DISPOSITIVO = 24;

const byte CANAL_ESCRITURA[6] = "00002";
const byte CANAL_LECTURA[6] = "00001";

//Mensajes al servidor
const int MENSAJE_HEARTBEAT = 0;
const int MENSAJE_SENSOR_MOVIMIENTO_ACTIVADO = 1;
const int MENSAJE_OBTENER_SENSOR_MOVIMIENTO = 2;

//Ordenes
const int ORDEN_HEARTBEAT = 0;
const int ORDEN_ENCENDER_SENSOR_MOVIMIENTO = 1;
const int ORDEN_APAGAR_SENSOR_MOVIMIENTO = 2;
const int ORDEN_OBTENER_SENSOR_MOVIMIENTO = 3;

bool sensorMovimientoEncendido = false;

char nombreDispositivo[LARGO_NOMBRE_DISPOSITIVO] = "SM0002";
char contenido[TAMANIO_MAXIMO_BUFFER];

bool escuchando = false;
bool ledEncendido = false;

void setup()
{
  inicializar();
}
void loop()
{
  verificarMensajesEntrantes();
  verificarSensorMovimiento();
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
        case ORDEN_ENCENDER_SENSOR_MOVIMIENTO:
        {
          sensorMovimientoEncendido = true;
          digitalWrite(LED_PIN, LOW);  
          obtenerSensorMovimiento();
          digitalWrite(LED_PIN, HIGH);  
          delay(1000);                     
          digitalWrite(LED_PIN, LOW);  
          delay(1000);    
          break;
        }
        case ORDEN_APAGAR_SENSOR_MOVIMIENTO:
        {
          sensorMovimientoEncendido = false;
          digitalWrite(LED_PIN, LOW);  
          obtenerSensorMovimiento();
          for(int i = 0; i < 2; i++);
          {
              digitalWrite(LED_PIN, HIGH);  
              delay(250);                     
              digitalWrite(LED_PIN, LOW);  
              delay(250);    
    
          }
          break;
        }
        case ORDEN_OBTENER_SENSOR_MOVIMIENTO:
        {
          obtenerSensorMovimiento();
          break;
        }
      }
    } 
  }
}

void obtenerSensorMovimiento()
{
  char mensajeString[TAMANIO_MAXIMO_BUFFER];
  char t[128];
  sprintf(mensajeString, "%d|%s,%d",MENSAJE_OBTENER_SENSOR_MOVIMIENTO,nombreDispositivo,int(sensorMovimientoEncendido));
  sprintf(t, "obtenerSensorMovimiento => %s",mensajeString);
  enviarInformacion(mensajeString);
}
void notificarMovimiento()
{
  char mensajeString[TAMANIO_MAXIMO_BUFFER];
  sprintf(mensajeString, "%d|%s",MENSAJE_SENSOR_MOVIMIENTO_ACTIVADO,nombreDispositivo);
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

int state = LOW;             // by default, no motion detected
int val = 0;   

void verificarSensorMovimiento()
{
  if(sensorMovimientoEncendido)
  {

    val = digitalRead(SENSOR_PIN);   // read sensor value
                 

    if (val == HIGH) {      
        if(state == LOW)
        {
          for(int i = 0; i < 5; i++)
          {
              digitalWrite(LED_PIN, HIGH);  
              delay(300);                     
              digitalWrite(LED_PIN, LOW);  
              delay(300);                     

          }
          notificarMovimiento();
          state = HIGH;
        }  // check if the sensor is HIGH  
    } 
    else {
        if (state == HIGH){
          state = LOW;       // update variable state to LOW
      }
    }
  }
}

