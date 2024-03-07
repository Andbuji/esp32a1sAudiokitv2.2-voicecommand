#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>
CloudSpeechClient::CloudSpeechClient(Authentication authentication)
{
  this->authentication = authentication;
  clientsec.setCACert(root_ca);
  if (!clientsec.connect(server, 443))
    Serial.println("Connection failed!");
}

String ans;

CloudSpeechClient::~CloudSpeechClient()
{
  clientsec.stop();
}

void CloudSpeechClient::PrintHttpBody2(Audio *audio)
{
  String enc = base64::encode(audio->paddedHeader, sizeof(audio->paddedHeader));
  enc.replace("\n", ""); 
  clientsec.print(enc);     // HttpBody2
  char **wavData = audio->wavData;
  for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j)
  {
    enc = base64::encode((byte *)wavData[j], audio->dividedWavDataSize);
    enc.replace("\n", ""); 
    clientsec.print(enc);  
  }
}

const char* CloudSpeechClient::Transcribe(Audio *audio)
{
  unsigned long tiempoInicioIntento = millis();

  String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"es-EC\"},\"audio\":{\"content\":\"";
  String HttpBody3 = "\"}}\r\n\r\n";
  int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader)) * 4 / 3; // 4/3 is from base64 encoding
  String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());
  String HttpHeader;
  HttpHeader = String("POST /v1/speech:recognize?key=") + ApiKey + String(" HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");
  clientsec.print(HttpHeader);
  clientsec.print(HttpBody1);
  PrintHttpBody2(audio);
  clientsec.print(HttpBody3);
  String My_Answer = "";
  while (!clientsec.available())
  ;

  while (clientsec.available())
  {
    char temp = clientsec.read();
    My_Answer = My_Answer + temp;
  }

  // Serial.print("My Answer - ");
  // Serial.println(My_Answer);
  int postion = My_Answer.indexOf('{');
  Serial.println("");
  ans = My_Answer.substring(postion);
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, ans);

  // Verificar errores de deserialización
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  // Extraer el valor de "transcript"
  const char *transcript = doc["results"][0]["alternatives"][0]["transcript"];

  // Imprimir el valor de "transcript"
  Serial.print("Transcript: ");
  Serial.println(transcript);
  Serial.println(millis()-tiempoInicioIntento);
  return transcript;
}
