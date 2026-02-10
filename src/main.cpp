/*!
 * @file  mRangeVelocity.cpp
 * @brief Radar measurement demo (C4001) - rewritten from .ino to .cpp
 * @copyright Copyright (c) 2010 DFRobot
 * @license The MIT License (MIT)
 * @author ZhixinLiu
 * @version V1.0
 * @date 2024-02-02
 * @url https://github.com/dfrobot/DFRobot_C4001
 */

#include "C1001Controller.h"

DFRobot_HumanDetection hu(&Serial2);

RTC_DATA_ATTR int bootCount = 0;
#define TIME_TO_SLEEP  60 // in sec
#define uS_TO_S_FACTOR 1000000ULL // omrekenfactor

void setupincommingWiFiforSetup() {
  WiFiClass::mode(WIFI_STA);
  WiFi.begin("esp32", "123456789");
  Serial.print("Connecting to WiFi ");
  while (WiFiClass::status() != WL_CONNECTED) {
    Serial.print('.');
  }
  if (WiFiClass::status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Connected to the WiFi network");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi");
  }
}

void initC1001() {
  Serial.println("Start initialization");
  while (hu.begin() != 0) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Initialization successful");

  Serial.println("Start switching work mode");
  while (hu.configWorkMode(hu.eFallingMode) != 0) {
    Serial.println("error!!!");
    delay(1000);
  }
  Serial.println("Work mode switch successful");

  Serial.print("Current work mode:");
  switch (hu.getWorkMode()) {
    case 1:
      Serial.println("Fall detection mode");
      break;
    case 2:
      Serial.println("Sleep detection mode");
      break;
    default:
      Serial.println("Read error");
  }

  hu.sensorRet();

  hu.dmInstallAngle(0,0,180);
  hu.dmInstallHeight(200);

  hu.configLEDLight(hu.eHPLed, 1);
  delay(100);
  hu.configLEDLight(hu.eHPLed, 0);  // Set HP LED switch, it will not light up even if the sensor detects a person when set to 0.
  delay(100);
  hu.configLEDLight(hu.eHPLed, 1);
  delay(100);
  hu.sensorRet();                   // Module reset, must perform sensorRet after setting data, otherwise the sensor may not be usable

  Serial.print("HP LED status:");
  switch (hu.getLEDLightState(hu.eHPLed)) {
    case 0:
      Serial.println("Off");
      break;
    case 1:
      Serial.println("On");
      break;
    default:
      Serial.println("Read error");
  }

  Serial.print("Existing information:");
  switch (hu.smHumanData(hu.eHumanPresence)) {
    case 0:
      Serial.println("No one is present");
      break;
    case 1:
      Serial.println("Someone is present");
      Serial.print("moving range: ");
      Serial.print(hu.smHumanData(hu.eHumanMovingRange));
      Serial.println();
      Serial.print("Distance: ");
      Serial.print(hu.smHumanData(hu.eHumanDistance));
      Serial.println();
      break;
    default:
      Serial.println("Read error");
  }

  Serial.print("Motion information:");
  switch (hu.smHumanData(hu.eHumanMovement)) {
    case 0:
      Serial.println("None");
      break;
    case 1:
      Serial.println("Still");
      break;
    case 2:
      Serial.println("Active");
      break;
    default:
      Serial.println("Read error");
  }

  Serial.println();
  Serial.println();
}


void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  setupincommingWiFiforSetup();
  if (server.on)
  initC1001();

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {}