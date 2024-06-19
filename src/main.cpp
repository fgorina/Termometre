#include <Arduino.h>
#include <M5StickCPlus2.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG_T true

const int oneWireBus = 26;
int ntDevices;

typedef union
{
  DeviceAddress address;
  uint64_t id;
} DeviceId;

DeviceId tempDeviceAddress;

DeviceId tempDevices[5] = {
    {.id = (uint64_t)4198535122781495592},
    {.id = (uint64_t)11176943009740906792},
    {.id = (uint64_t)8501594867311599912},
    {.id = (uint64_t)369306671541149992},
    {.id = (uint64_t)13078325212068733224}};

const String deviceNames[] = {
    "Engine",
    "Exhaust",
    "Engine Room",
    "Water",
    "Outside"};

int currentDevice = 0;
String currentDeviceName = deviceNames[0];
// 1-Wire and Temperature

OneWire oneWire(oneWireBus);
DallasTemperature tSensors(&oneWire);

int lookupDevice(DeviceId device)
{

  for (int i = 0; i < 5; i++)
  {
    if (device.id == tempDevices[i].id)
    {
      return (i);
    }
  }
  return -1;
}

char *uintToStr(const uint64_t num, char *str)
{
  uint8_t i = 0;
  uint64_t n = num;

  do
    i++;
  while (n /= 10);

  str[i] = '\0';
  n = num;

  do
    str[--i] = (n % 10) + '0';
  while (n /= 10);

  return str;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  auto cfg = M5.config();
  StickCP2.begin(cfg);
  StickCP2.Lcd.setRotation(1);

  int textsize = StickCP2.Display.width() / 60;
  if (textsize == 0)
  {
    textsize = 1;
  }
  textsize = 4;
  Serial.println(textsize);
  StickCP2.Display.setTextSize(textsize);

  tSensors.begin();

  ntDevices = tSensors.getDeviceCount();

  if (DEBUG_T)
  {
    Serial.print("Found ");
    ntDevices;
    Serial.println("devices.");
  }

  for (int i = 0; i < ntDevices; i++)
  {
    tSensors.getAddress(tempDeviceAddress.address, i);
    if (DEBUG_T)
    {
      int j = lookupDevice(tempDeviceAddress);
      if (j != -1)
      {
        String deviceName = deviceNames[j];

        Serial.print("    Device ");
        Serial.print(j);
        Serial.print(" ");
        Serial.print(deviceName);
        Serial.print(tempDeviceAddress.id);
        Serial.print("  ");
        Serial.println();
      }
    }
  }
}

float old_tc = 0.0;
int oldCurrentDevice = -1;

void loop()
{
  char buff[25];
  char buff2[25];
  StickCP2.update();
  tSensors.requestTemperatures();
  DeviceAddress oldDeviceAddress;

  

  if (StickCP2.BtnA.wasPressed())
  {
    StickCP2.Speaker.tone(2000, 20);

    Serial.println("Pushed Button");
  }

  if (StickCP2.BtnA.wasReleased())
  {
    StickCP2.Speaker.tone(4000, 20);
    oldCurrentDevice = currentDevice;

    currentDevice = (currentDevice + 1) % ntDevices;
    
    if (tSensors.getAddress(tempDeviceAddress.address, currentDevice))
    {

      int j = lookupDevice(tempDeviceAddress);
      if (j != -1)
      {
        currentDeviceName = deviceNames[j];
      }
      else
      {
        currentDeviceName = "Unknown";
      }
    }
    Serial.println("Released Button");
  }

  if (tSensors.getAddress(tempDeviceAddress.address, currentDevice))
  {
    int j = lookupDevice(tempDeviceAddress);
    if (j != -1)
    {
      currentDeviceName = deviceNames[j];
    }
    else
    {
      currentDeviceName = "Unknown";
    }
    float tc = tSensors.getTempC(tempDeviceAddress.address);

    if (fabs(tc - old_tc) > 0.1 || oldCurrentDevice != currentDevice)
    {
      StickCP2.Display.clear();
      uintToStr(tempDeviceAddress.id, buff);
      strncpy(buff2, buff, 10);

      StickCP2.Display.setTextSize(2);
      StickCP2.Display.drawCenterString(buff2, StickCP2.Display.width() / 2, 6);
      strcpy(buff2, buff + 10);
      StickCP2.Display.drawCenterString(buff2, StickCP2.Display.width() / 2, 26);

      dtostrf(tc, 3, 1, buff);
      sprintf(buff2, "%s C", buff);
      StickCP2.Display.setTextSize(4);
      StickCP2.Display.drawCenterString(buff2, StickCP2.Display.width() / 2, StickCP2.Display.height() / 2 - 10);
      sprintf(buff2, "%d", currentDevice);
      StickCP2.Display.setTextSize(2);
      StickCP2.Display.drawString(currentDeviceName, 6, StickCP2.Display.height() - 25);
      StickCP2.Display.drawRightString(buff2, StickCP2.Display.width() - 6, StickCP2.Display.height() - 25);
      old_tc = tc;
      oldCurrentDevice = currentDevice;

      if (DEBUG_T)
      {
        Serial.print("Device ");
        Serial.print(currentDevice);
        Serial.print(" ");
        Serial.print(currentDeviceName);

        Serial.print(" (");

        Serial.print(tempDeviceAddress.id);
        Serial.print(") ");
        Serial.print(tc);
        Serial.println(" ºC");
      }
    }
  }

  vTaskDelay(100);
}
