# 1 "/workspaces/IMU_Test/IMU_Test.ino"

# 3 "/workspaces/IMU_Test/IMU_Test.ino" 2
# 4 "/workspaces/IMU_Test/IMU_Test.ino" 2
# 5 "/workspaces/IMU_Test/IMU_Test.ino" 2
# 6 "/workspaces/IMU_Test/IMU_Test.ino" 2
# 7 "/workspaces/IMU_Test/IMU_Test.ino" 2



# 9 "/workspaces/IMU_Test/IMU_Test.ino"
WiFiUDP udpDevice;
uint16_t localUdpPort = 3302; // Puerto de escucha del propio sensor
uint16_t UDPPort = 4000; // Puerto al que enviará la información el sensor.
                         // El cliente (Unity) debería estar escuchando este puerto.
const char *unityIP = "192.168.25.12";

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float offsetArr[3] = {0.0F};

void getOffset(float *offset) {
  float offsetX, offsetY, offsetZ;
  M5.IMU.getAccelData(&offsetX, &offsetY, &offsetZ);
  offset[0] = offsetX;
  offset[1] = offsetY;
  offset[2] = offsetZ;
}

void drawGrid() {
  M5.Lcd.drawLine(41, 120, 279, 120, 0x07FF /*   0, 255, 255 */);
  M5.Lcd.drawLine(160, 1, 160, 239, 0x07FF /*   0, 255, 255 */);
  M5.Lcd.drawCircle(160, 120, 119, 0x07FF /*   0, 255, 255 */);
  M5.Lcd.drawCircle(160, 120, 60, 0x07FF /*   0, 255, 255 */);
}

void drawSpot(int ax, int ay) {
  static int prevx = 0;
  static int prevy = 0;
  int x, y;
  x = map(((ax)<(-300)?(-300):((ax)>(300)?(300):(ax))), -300, 300, 40, 280);
  y = map(((ay)<(-300)?(-300):((ay)>(300)?(300):(ay))), -300, 300, 240, 0);

  M5.Lcd.fillCircle(prevx, prevy, 7, 0x0000 /*   0,   0,   0 */);
  drawGrid();
  M5.Lcd.fillCircle(x, y, 7, 0xFFFF /* 255, 255, 255 */);
  prevx = x;
  prevy = y;

}


void setup() {
  M5.begin(); //Init M5Core.
  M5.IMU.Init(); //Init IMU sensor
  getOffset(offsetArr);
  M5.Axp.SetLed(0);
  /*

    M5.Lcd.setTextColor(

    GREEN,

    BLACK);

    M5.Lcd.setTextSize(2);

  */
# 63 "/workspaces/IMU_Test/IMU_Test.ino"
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
  udpDevice.begin(localUdpPort);
}

void loop() {

  static uint32_t imuReadTime = 0;
  static uint32_t printIMUTime = 0;

  uint32_t currentTime = millis();

  if ( (currentTime - imuReadTime) > 100) {
    imuReadTime = currentTime;
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    drawSpot((int)((accX - offsetArr[0]) * 1000), (int)((accY - offsetArr[1]) * 1000));
  }

  currentTime = millis();
  if ( (currentTime - printIMUTime ) > 100) {
    printIMUTime = currentTime;
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    int x, y;
    x = (int)((accX - offsetArr[0]) * 1000);
    y = (int)((accY - offsetArr[1]) * 1000);
    printf("accX: %d, accY: %d\n", x, y);
    printf("Vbat:%f/Cbat:%f\n", M5.Axp.GetBatVoltage(), M5.Axp.GetBatCurrent());
    if (y >= 150) {
      auto m = (accX - offsetArr[0]) / (accY - offsetArr[1]);
      printf("m: %f\n", m);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.printf("m: %f", m);


      udpDevice.beginPacket(unityIP, UDPPort);
      udpDevice.write((uint8_t *)&m, 4);
      udpDevice .endPacket();


    }
  }
}

// python -m pip install pyserial
