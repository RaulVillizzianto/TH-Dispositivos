//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

const int INTERVALO_ENTRE_TRABAJO = 2500;
const int TAMANIO_MAXIMO_BUFFER = 64;
const int LARGO_NOMBRE_DISPOSITIVO = 24;

const byte CANAL_ESCRITURA[6] = "00001";
const byte CANAL_LECTURA[6] = "00002";

char mensaje[TAMANIO_MAXIMO_BUFFER]; 
char mensajeString[TAMANIO_MAXIMO_BUFFER];

const int ESTABLECER_NOMBRE_DISPOSITIVO = 1;
const int SENSOR_MOVIMIENTO_ACTIVADO = 1;
const int HEARTBEAT = 0;
const int ENCENDER_SENSOR_MOVIMIENTO = 2;
const int APAGAR_MOVIMIENTO_ACTIVADO = 3;

void setup()
{
    inicializar();
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  delay(250);
  establecerReceptor();
  while (radio.available())
  {
    radio.read(&mensaje, TAMANIO_MAXIMO_BUFFER);
    Serial.println(mensaje);
  }
  delay(250);
  while (Serial.available () > 0)
  {
    process_data(Serial.readString());
  }
}


void process_data (String data)
{
  establecerTransmisor();
  bool resultado = radio.write(data.c_str(), TAMANIO_MAXIMO_BUFFER);
  sprintf(mensajeString, "%s", resultado ? "true" : "false");
  Serial.println(mensajeString);
} 
void inicializar()
{

  //radio.begin();
  while (!Serial); 
  {
    Serial.begin(9600);
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  
  }
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
  radio.openWritingPipe(CANAL_ESCRITURA); // 00001
  radio.openReadingPipe(1, CANAL_LECTURA); // 00002
  Serial.println("DEBUG:inicializado");
}
void establecerReceptor()
{
  radio.startListening();
}
void establecerTransmisor()
{
  radio.stopListening();
}