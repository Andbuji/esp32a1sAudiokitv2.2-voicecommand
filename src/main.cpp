#include "AudioKitHAL.h"
#include "Audio.h"
#include "CloudSpeechClient.h"
#include "WiFi.h"
#include <PubSubClient.h>
#include <WiFiManager.h>

AudioKit kit;

const int BUFFER_SIZE = 1024;
uint8_t buffer[BUFFER_SIZE];

// WakeUp Button
#define button 19

// RGB LEDs for status indication
#define led_1 5
#define led_2 18
#define led_3 23

const char *WIFI_SSID = "YOUR-SSID";
const char *WIFI_PASS = "YOUR-PASSWORD";
// MQTT
const char *mqtt_server = "YOUR-SERVER";
const int mqtt_port = 1883;
const char *mqtt_user = "YOUR-USER";
const char *mqtt_pass = "YOUR-PASSWORD";

WiFiClient espClient;
PubSubClient client(espClient);

int i = 0;
String transcripcion;

void connectWifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi conectado: ");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void grabar()
{
  if (digitalRead(button) == 0)
  {
    digitalWrite(led_1, 0);
    delay(100);
    Serial.println("\r\nRecord start!\r\n");
    Audio *audio = new Audio(ICS43434);
    digitalWrite(led_3, 1);
    unsigned long tiempoInicioIntento = millis();

    audio->Record();
    Serial.print("del record           ");
    Serial.println(millis() - tiempoInicioIntento);

    Serial.println("Recording Completed. Now Processing...");
    CloudSpeechClient *cloudSpeechClient = new CloudSpeechClient(USE_APIKEY);
    const char *result = cloudSpeechClient->Transcribe(audio);
    if (result != nullptr)
    {
      transcripcion = String(result);
    }
    else
    {
      Serial.println("Error: La transcripción es nula.");
      digitalWrite(led_3, 0);
      delay(500);
      digitalWrite(led_2, 1);
      delay(500);
      digitalWrite(led_2, 0);
      delay(500);
      digitalWrite(led_2, 1);
      delay(500);
      digitalWrite(led_2, 0);
    }
    delete cloudSpeechClient;
    delete audio;
    i = 0;
  }
  if (digitalRead(button) == 1)
  {
    delay(1);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect()
{
  const unsigned long tiempoEsperaMaximo = 5 * 60 * 1000; // 5 minutos en milisegundos
  unsigned long tiempoInicioIntento = millis();

  while (!client.connected())
  {
    Serial.print("Conectando al servidor MQTT...");
    if (client.connect("esp32-stt-8067", mqtt_user, mqtt_pass))
    {
      Serial.println("conectado");
    }
    else
    {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }

    // Verificar si ha pasado el tiempo de espera máximo
    if (millis() - tiempoInicioIntento > tiempoEsperaMaximo)
    {
      Serial.println("Tiempo de espera máximo alcanzado. Abortando reconexión.");
      break;
    }
  }
}

void indicadorOK()
{
  digitalWrite(led_3, 0);
  delay(500);
  digitalWrite(led_3, 1);
  delay(500);
  digitalWrite(led_3, 0);
}

void setup()
{
  pinMode(button, INPUT);
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(led_3, OUTPUT);
  Serial.begin(115200);
  connectWifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  auto cfg = kit.defaultConfig(KitInput);
  cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2; // microphone?
  cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;
  cfg.bits_per_sample = AUDIO_HAL_BIT_LENGTH_32BITS;
  cfg.buffer_count = 16;
  cfg.buffer_size = 60;
  kit.begin(cfg);
}

void loop()
{
  client.loop();
  if (client.connected() == false)
  {
    reconnect();
  }

  digitalWrite(led_1, 1);

  if (i == 0)
  {
    Serial.println("Press button");
    i = 1;
  }

  delay(500);

  grabar();
  transcripcion.toLowerCase();

  // LUCES

  if (transcripcion == "encender luz 1" || transcripcion == "encender luz uno")
  {
    if (client.publish("test/esp32/rele1", "1"))
    {
      indicadorOK();
    }

    transcripcion = "";
  }
  else if (transcripcion != "")
  {
    digitalWrite(led_3, 0);
    delay(500);
    digitalWrite(led_2, 1);
    delay(500);
    digitalWrite(led_2, 0);
    delay(500);
    digitalWrite(led_2, 1);
    delay(500);
    digitalWrite(led_2, 0);
    transcripcion = "";
  }
}
