#ifndef _CLOUDSPEECHCLIENT_H
#define _CLOUDSPEECHCLIENT_H
#include <WiFiClientSecure.h>
#include "Audio.h"

enum Authentication {
  USE_ACCESSTOKEN,
  USE_APIKEY
};

class CloudSpeechClient {
  WiFiClientSecure clientsec;
  void PrintHttpBody2(Audio* audio);
  Authentication authentication;

public:
  CloudSpeechClient(Authentication authentication);
  ~CloudSpeechClient();
  const char* Transcribe(Audio* audio);
};

#endif // _CLOUDSPEECHCLIENT_H
