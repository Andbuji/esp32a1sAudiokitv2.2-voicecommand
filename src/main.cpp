#include "AudioKitHAL.h"
#include "Audio.h"
#include "CloudSpeechClient.h"
#include "WiFi.h"
#include <PubSubClient.h>
#include <WiFiManager.h>

AudioKit kit;

const int BUFFER_SIZE = 1024;
uint8_t buffer[BUFFER_SIZE];

// IR Sensor acting as WakeUp Button
#define button 19

// RGB LEDs for status indication
#define led_1 5
#define led_2 18
#define led_3 23

const char *WIFI_SSID = "IoTLab";
const char *WIFI_PASS = "iotm@ch1";
// MQTT
const char *mqtt_server = "192.168.200.246";
const int mqtt_port = 1883;
const char *mqtt_user = "mqttiotlab6";
const char *mqtt_pass = "iotm@ch1";

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
    if (client.publish("proto-aranda/esp32/rele1", "1"))
    {
      indicadorOK();
    }

    transcripcion = "";
  }
  else if (transcripcion == "apagar luz 1" || transcripcion == "apagar luz uno")
  {
    if (client.publish("proto-aranda/esp32/rele1", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender luz pasillo 1" || transcripcion == "encender luz pasillo uno")
  {
    if (client.publish("proto-aranda/esp32/rele2", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar luz pasillo 1" || transcripcion == "apagar luz pasillo uno")
  {
    if (client.publish("proto-aranda/esp32/rele2", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender luz pasillo 2" || transcripcion == "encender luz pasillo dos")
  {
    if (client.publish("proto-aranda/esp32/rele3", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar luz pasillo 2" || transcripcion == "apagar luz pasillo dos")
  {
    if (client.publish("proto-aranda/esp32/rele3", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender luz pasillo 3" || transcripcion == "encender luz pasillo tres")
  {
    if (client.publish("proto-aranda/esp32/rele4", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar luz pasillo 3" || transcripcion == "apagar luz pasillo tres")
  {
    if (client.publish("proto-aranda/esp32/rele4", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender luz pasillo 4" || transcripcion == "encender luz pasillo cuatro")
  {
    if (client.publish("proto-aranda/esp32/rele5", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar luz pasillo 4" || transcripcion == "apagar luz pasillo cuatro")
  {
    if (client.publish("proto-aranda/esp32/rele5", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender luz pasillo 5" || transcripcion == "encender luz pasillo cinco")
  {
    if (client.publish("proto-aranda/esp32/rele6", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar luz pasillo 5" || transcripcion == "apagar luz pasillo cinco")
  {
    if (client.publish("proto-aranda/esp32/rele6", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  // ENCHUFES
  else if (transcripcion == "encender enchufe 1" || transcripcion == "encender enchufe uno")
  {
    if (client.publish("iot/tomacorriente/1", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 1" || transcripcion == "apagar luz enchufe uno")
  {
    if (client.publish("iot/tomacorriente/1", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender enchufe 2" || transcripcion == "encender enchufe dos")
  {
    if (client.publish("iot/tomacorriente/2", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 2" || transcripcion == "apagar luz enchufe dos")
  {
    if (client.publish("iot/tomacorriente/2", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender enchufe 3" || transcripcion == "encender enchufe tres")
  {
    if (client.publish("iot/tomacorriente/3", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 3" || transcripcion == "apagar luz enchufe tres")
  {
    if (client.publish("iot/tomacorriente/3", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender enchufe 4" || transcripcion == "encender enchufe cuatro")
  {
    if (client.publish("iot/tomacorriente/4", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 4" || transcripcion == "apagar luz enchufe cuatro")
  {
    if (client.publish("iot/tomacorriente/4", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender enchufe 5" || transcripcion == "encender enchufe cinco")
  {
    if (client.publish("iot/tomacorriente/5", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 5" || transcripcion == "apagar luz enchufe cinco")
  {
    if (client.publish("iot/tomacorriente/5", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender enchufe 6" || transcripcion == "encender enchufe seis")
  {
    if (client.publish("iot/tomacorriente/6", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 6" || transcripcion == "apagar luz enchufe seis")
  {
    if (client.publish("iot/tomacorriente/6", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender enchufe 7" || transcripcion == "encender enchufe siete")
  {
    if (client.publish("iot/tomacorriente/7", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 7" || transcripcion == "apagar luz enchufe siete")
  {
    if (client.publish("iot/tomacorriente/7", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "encender enchufe 8" || transcripcion == "encender enchufe ocho")
  {
    if (client.publish("iot/tomacorriente/8", "0"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  else if (transcripcion == "apagar enchufe 8" || transcripcion == "apagar luz enchufe ocho")
  {
    if (client.publish("iot/tomacorriente/8", "1"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  // LUCES GRUPO GARCIA

  else if (transcripcion == "encender luces")
  {
    if (client.publish("Utmach/Tics/Iot/Proyecto5/Luces", "true"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }
  else if (transcripcion == "apagar luces")
  {
    if (client.publish("Utmach/Tics/Iot/Proyecto5/Luces", "false"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }

  // TOMACORRIENTES GRUPO GARCIA

  else if (transcripcion == "encender tomacorrientes")
  {
    if (client.publish("Utmach/Tics/Iot/Proyecto5/Tomacorrientes", "true"))
    {
      indicadorOK();
    }
    transcripcion = "";
  }
  else if (transcripcion == "apagar tomacorrientes")
  {
    if (client.publish("Utmach/Tics/Iot/Proyecto5/Tomacorrientes", "false"))
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
