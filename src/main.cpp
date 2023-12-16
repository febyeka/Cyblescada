#include <Arduino.h>
#include <wire.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

const String idDevice = "WSL0002";
const int lora = 5;
const int batt = 4;

// Variable Untuk Uplink
String flow = "";
String flowSend = "";
String dataLog = "";

// Variable Logging
int logCyble;
int logLiter;

int cybleSebelumnya;
int literSebelumnya;

// Config Pin Cyble
const int LF_State = 25;

// Cyble Counter
int literCounter = 0;
int CybleCounter_LF;
int SensorState_LF = 0;
int lastSensorState_LF = 0;
double debit;
int countInProcess = 0;

void setup()
{
  // put your setup code here, to run once:
  EEPROM.begin(512);
  Serial.begin(19200);
  // Serial2.begin(19200);
  Serial2.begin(19200, SERIAL_8N1, 16, 17);

  pinMode(LF_State, INPUT);
  pinMode(lora, OUTPUT);
  pinMode(batt, OUTPUT);

  digitalWrite(lora, LOW);
  digitalWrite(batt, HIGH);

  StaticJsonDocument<500> doc;
  EepromStream eepromStream(0, 500);
  deserializeJson(doc, eepromStream);

  if (doc["lf"])
  {
    Serial.println("Loaded doc's LF value.");
    CybleCounter_LF = doc["lf"];
    logCyble = CybleCounter_LF;
    logLiter = doc["lB"];
  }
  else
  {
    Serial.println("No 'LF' variable in eeprom.");
  }

  delay(1000);
  Serial.println("System Ready");
}

void dataUplink()
{

  StaticJsonDocument<152> dataUp;
  dataUp["Type"] = "Flow";
  dataUp["SN"] = idDevice;
  dataUp["Data0"] = String(CybleCounter_LF);
  dataUp["Data1"] = String(debit);
  serializeJson(dataUp, Serial2);
}

void dataDownlink()
{
  if (Serial2.available() > 0)
  {
    StaticJsonDocument<512> datadown;
    DeserializationError error = deserializeJson(datadown, Serial2);
    if (error)
    {
      Serial.print("deserializeJson() failed Downlink: ");
      Serial.println(error.c_str());
      return;
    }
    int list = datadown["list"];           // 1
    const char *gwyid = datadown["gwyid"]; // "MELGSL001"

    const char *downlink_0_id = datadown["downlink"][0]["id"];       // "WSL0001"
    const char *downlink_0_gwyid = datadown["downlink"][0]["gwyid"]; // "MELGSL001"
    if (String(downlink_0_id) == idDevice)
    {
      Serial.print("ID :");
      Serial.println(downlink_0_id);
      dataUplink();
    }
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  // StaticJsonDocument<500> doc;
  // EepromStream eepromStream(0, 500);
  // doc["lf"] = 2763;
  // doc["lfB"] = logCyble;
  // doc["lB"] = logLiter;
  // serializeJson(doc, eepromStream);
  // eepromStream.flush();
  dataDownlink();

  static uint32_t timeProgress = millis();
  static uint32_t timeReadLF = millis();

  SensorState_LF = digitalRead(LF_State);
  if (SensorState_LF != lastSensorState_LF)
  {
    if (SensorState_LF == LOW)
    {
      CybleCounter_LF++;
      StaticJsonDocument<500> doc;
      EepromStream eepromStream(0, 500);

      doc["lf"] = CybleCounter_LF;
      doc["lfB"] = logCyble;
      doc["lB"] = logLiter;
      serializeJson(doc, eepromStream);
      eepromStream.flush();
      literCounter = literCounter + 10;
    }
  }
  
  lastSensorState_LF = SensorState_LF;
  if (countInProcess >= 60)
  {
     Serial.print("stand:");
     Serial.println(CybleCounter_LF);
     Serial.print("liter:  ");
     Serial.println(literCounter);
     debit = literCounter / 60.0;
     Serial.print("debit: ");
     Serial.println(debit);
     literCounter = 0;
     countInProcess = 0;
  }

  if ((millis() - timeReadLF) > 1000)
  {
    timeReadLF = millis();
    if (countInProcess <= 60)
    {
      countInProcess++;
    }
  }

  // if ((millis() - timeProgress) > 60000)
  // {
  //   timeProgress = millis();

  //   // dataUplink();
  //   Serial.print("stand:");
  //   Serial.println(CybleCounter_LF);
  //   Serial.print("liter:  ");
  //   Serial.println(literCounter);
  //   debit = literCounter / 60.0;
  //   Serial.print("debit: ");
  //   Serial.println(debit);
  //   literCounter = 0;
  // }
}
