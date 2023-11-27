#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

void Init_Spiffs();
void Init_AP();
void CAN_Mockup();

#endif