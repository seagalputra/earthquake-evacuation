#include <PubSubClient.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <cmath>

const char* ssid = "SI4007";
const char* password = "Qud4terbang@";
const char* mqtt_server = "soldier.cloudmqtt.com";
const char* mqtt_username = "mizrsriv";
const char* mqtt_password = "vKGsX3BGaJqa";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;

// sensitivity scale factor respective to full scale setting provided in datasheet 
//const uint16_t AccelScaleFactor = 16384;
const uint16_t AccelScaleFactor = 4096;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
char skala_richter[6];

void setup() {
  Serial.begin(9600);
  Wire.begin(sda, scl);
  MPU6050_Init();
  setup_wifi();
  client.setServer(mqtt_server, 18162);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  double Ax, Ay, Az, A2_x, A2_y, A2_z;
  double Ax_kuadrat, Ay_kuadrat, Az_kuadrat, total_kuadrat, amplitudo, sr;
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  
  //divide each with their sensitivity scale factor
  Ax = (double)AccelX/AccelScaleFactor;
  Ay = (double)AccelY/AccelScaleFactor;
  Az = (double)AccelZ/AccelScaleFactor;

  // ambil nilai accelerometer detik selanjutnya
  delay(1000);

  A2_x = (double)AccelX/AccelScaleFactor;
  A2_y = (double)AccelY/AccelScaleFactor;
  A2_z = (double)AccelZ/AccelScaleFactor;

  Serial.println("Pertama");
  Serial.print("Ax: "); Serial.print(Ax);
  Serial.print("Ay: "); Serial.print(Ay);
  Serial.print("Az: "); Serial.print(Az);
  Serial.println();

  Serial.println("Kedua");
  Serial.print("A2_x: "); Serial.print(A2_x);
  Serial.print("A2_y: "); Serial.print(A2_y);
  Serial.print("A2_z: "); Serial.print(A2_z);
  Serial.println();
  
  /*
  Ax_kuadrat = (double)(pow(Ax,2));
  Ay_kuadrat = (double)(pow(Ay,2));
  Az_kuadrat = (double)(pow(Az,2));
  total_kuadrat = (double)sqrt(Ax_kuadrat + Ay_kuadrat + Az_kuadrat);
  // sr = (double)log(Ax) + 2.56*log(0.01) - 1.67;
  sr = (double)log(Ax) + 3*log(total_kuadrat) - 2.92;

  Serial.print("Ax: "); Serial.print(Ax);
  Serial.print("Ay: "); Serial.print(Ay);
  Serial.print("Az: "); Serial.print(Az);
  Serial.print("Skala richter: "); Serial.println(sr);
  client.publish("tugas-akhir", dtostrf(sr,3,3,skala_richter));
  */
  
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("tugas-akhir");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
