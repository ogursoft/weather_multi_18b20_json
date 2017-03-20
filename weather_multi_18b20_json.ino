#include <ArduinoJson.h>
#include <BMP085.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

OneWire oneWire(3);// вход датчиков 18b20

BMP085 dps = BMP085();

DallasTemperature ds(&oneWire);

DHT dht(DHTPIN, DHTTYPE);

long Temperature = 0, Pressure = 0, Altitude = 0;

byte qty; // количество датчиков на шине

char buffer[50], name[50];

void setup() {
  Serial.begin(115200);
  ds.begin();

  qty = ds.getDeviceCount();

  dht.begin();

  delay(1000);

  dps.init(MODE_STANDARD, 0, true);  // 132 meters, true = using meter units

}

void loop() {

  int val = 0;
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["station"] = "ogursoft1";

  ds.requestTemperatures(); // считываем температуру с датчиков

  //давление bmp180
  dps.getPressure(&Pressure);
  dps.getAltitude(&Altitude);
  dps.getTemperature(&Temperature);

  //влажность
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);

  // Compute heat index
  // in Fahrenheit!
  float hif = dht.computeHeatIndex(f, h);
  // по Цельсию!
  float hic = dht.computeHeatIndex(t, h, false);

  DeviceAddress deviceAddress;

  // JsonArray& temp = root.createNestedArray("temp");
  if (qty > 0) {
    JsonObject& temp = root.createNestedObject("temp");
    for (int i = 0; i < qty; i++) { // крутим цикл
      JsonArray& temp_ = temp.createNestedArray("temp" + String(i));
      ds.getAddress(deviceAddress, i);
      temp_.add("temp_sensor" + String(i));
      temp_.add("ds18b20");
      temp_.add(printAddress(deviceAddress));
      temp_.add(ds.getTempCByIndex(i));
    }
  }

  //с датчика bmp180
  JsonObject& description_p = root.createNestedObject("pressure");
  description_p["name"] = "pressure_sensor";
  description_p["type"] = "bmp180";
  JsonObject& data_p = description_p.createNestedObject("data");
  data_p["temperature"] = Temperature * 0.1;
  data_p["pressure"] = Pressure / 133.3;


  //с датчика dht22
  // Check if any reads failed and exit early (to try again).
  if (!isnan(h) || !isnan(t) || !isnan(f)) {
    JsonObject& description_h = root.createNestedObject("humidity");
    description_h["name"] = "humidity_sensor";
    description_h["type"] = "dht22";
    JsonObject& data_h = description_h.createNestedObject("data");
    data_h["temperature_cel"] = t;
    data_h["humidity"] = h;
    data_h["temperature_far"] = f;
    data_h["heatindex_cel"] = hic;
    data_h["heatindex_far"] = hif;
  }

  if (Serial.available() > 0) {
    int val = Serial.read();
    //  Serial.println(val, DEC);
    if (val == 49) {
      root.printTo(Serial);
      Serial.println();
    }
  }
}


// function to print a device address
String printAddress(DeviceAddress deviceAddress)
{
  String temprom;
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) ;
    temprom =  temprom + String(deviceAddress[i], HEX);
  }
  return temprom;
}

String sensor_num (int num)
{
  char sensor[16];
  sprintf(sensor, "temp_sensor%d", num);
  return sensor;
}

