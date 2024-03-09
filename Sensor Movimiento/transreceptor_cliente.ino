//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define PIN_LED 5
#define PIN_CE 9
#define PIN_CSN 8
#define SENSOR_PIN 7
#define DEBUG

//create an RF24 object
RF24 radio(PIN_CE,PIN_CSN);  // CE, CSN

//address through which two modules communicate.
const int INTERVALO_ENTRE_TRABAJO = 1000;
const int TAMANIO_MAXIMO_BUFFER = 64;
const int LARGO_NOMBRE_DISPOSITIVO = 24;

const byte CANAL_ESCRITURA[6] = "00002";
const byte CANAL_LECTURA[6] = "00001";

const int ESTABLECER_NOMBRE_DISPOSITIVO = 1;

const int HEARTBEAT = 0;
const int SENSOR_MOVIMIENTO_ACTIVADO = 1;
const int ENCENDER_SENSOR_MOVIMIENTO = 2;
const int APAGAR_SENSOR_MOVIMIENTO = 3;
const int OBTENER_SENSOR_MOVIMIENTO = 4;

bool sensorMovimientoEncendido = false;

char nombreDispositivo[LARGO_NOMBRE_DISPOSITIVO] = "SM0001";
char mensajeString[TAMANIO_MAXIMO_BUFFER];
char contenido[TAMANIO_MAXIMO_BUFFER];



int state = LOW;             // by default, no motion detected
int val = 0;                 // variable to store the sensor status (value)

void setup()
{
  inicializar();
}
void loop()
{
  verificarMensajesEntrantes();
  verificarSensorMovimiento();
  delay(INTERVALO_ENTRE_TRABAJO);
}

void establecerComoReceptor()
{
  radio.startListening();
}
void establecerComoTransmisor()
{
  radio.stopListening();
}
void inicializar()
{
  #ifdef DEBUG
  while (!Serial); 
  {
    Serial.begin(9600);
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  
  }
  #endif
  if (!radio.begin())
  { 
    while (1)
    {
      Serial.println(F("DEBUG:radio hardware not responding!"));
      delay(1000);
    }
  }
  radio.setDataRate(RF24_2MBPS); // Set the speed of the transmission to the quickest available
  radio.setChannel(124); // Use a channel unlikely to be used by Wifi, Microwave ovens etc
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(CANAL_ESCRITURA); //00002
  radio.openReadingPipe(1, CANAL_LECTURA); //00001
  #ifdef DEBUG
  Serial.println("DEBUG:inicializado");
  #endif
}
void verificarMensajesEntrantes()
{
  establecerComoReceptor();
  while(radio.available())
  {
    
    int codigo_mensaje = 0;
    radio.read(&mensajeString, TAMANIO_MAXIMO_BUFFER);
    sscanf(mensajeString,"%d|%s", &codigo_mensaje, &contenido );
    String cont = contenido;

    String nombreDispositivoApuntado = cont.substring(0, cont.indexOf(','));
    nombreDispositivoApuntado.trim();
    if(strcmp(nombreDispositivoApuntado.c_str(), nombreDispositivo) == 0)
    {
      switch(codigo_mensaje)
      {
        /*case ESTABLECER_NOMBRE_DISPOSITIVO:
        {    
            establecerNombreDispositivo(nombreDispositivoApuntado,  cont.substring(cont.indexOf(',')+1));
            break;
        }*/
        case HEARTBEAT:
        {
          heartBeat();
          break;
        }
        case ENCENDER_SENSOR_MOVIMIENTO:
        {
          sensorMovimientoEncendido = true;
          obtenerSensorMovimiento();
          break;
        }
        case APAGAR_SENSOR_MOVIMIENTO:
        {
          sensorMovimientoEncendido = false;
          obtenerSensorMovimiento();
          break;
        }
        case OBTENER_SENSOR_MOVIMIENTO:
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
  char t[128];
  sprintf(mensajeString, "%d|%s,%d",OBTENER_SENSOR_MOVIMIENTO,nombreDispositivo,int(sensorMovimientoEncendido));
  sprintf(t, "obtenerSensorMovimiento => %s",mensajeString);
  #ifdef DEBUG
  Serial.println(t);
  #endif
  enviarInformacion();
}
void notificarMovimiento()
{
  sprintf(mensajeString, "%d|%s",SENSOR_MOVIMIENTO_ACTIVADO,nombreDispositivo);
  enviarInformacion();
}

void establecerNombreDispositivo(String viejoNombre, String nuevoNombre)
{
  if(strcmp(viejoNombre.c_str(), nombreDispositivo) == 0)
  {
    sprintf(nombreDispositivo, "%s",nuevoNombre.c_str());
  }
}
void heartBeat()
{
  sprintf(mensajeString, "%d|%s",HEARTBEAT,nombreDispositivo);
  enviarInformacion();
}

void enviarInformacion()
{
  delay(250);
  establecerComoTransmisor();
  radio.write(mensajeString,TAMANIO_MAXIMO_BUFFER);
  Serial.println(mensajeString);
  establecerComoReceptor();
    delay(250);

}
void verificarSensorMovimiento()
{
  if(sensorMovimientoEncendido)
  {

    val = digitalRead(SENSOR_PIN);   // read sensor value
    if (val == HIGH) {           // check if the sensor is HIGH
      if (state == LOW) {
        state = HIGH;       // update variable state to HIGH
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
        delay(1000);                      // wait for a second
        digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
        delay(1000);    
        notificarMovimiento();
      }
    } 
    else {
        if (state == HIGH){
          state = LOW;       // update variable state to LOW
      }
    }
  }
}

