/*
  The project for taking breath as controller to conduct breath training by using OLED display instution.
  This project using a method called "two step, one breath",
      using MPU-6050 as accelerometer counting step,
      Microphone as breath detector,
      OLED with u8glib as the display output.
*/

#include <Wire.h>
#include <U8glib.h>

U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9); // OLED: SCK = 13, MOSI = 11, CS = 10, A0 = 9
const int MPU_ADDR = 0x68;      // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

// property pins
const short lightOut = 6;
const short button = 7;
const short soundPin = 1;

// system
bool state = false;
bool passedTut = false;
short tutPhase = 4;

// pace
unsigned int startedStep = 0; // for step detect
int count = 0;  // step count
unsigned int startTime = 0;


// breath
int startStep = 0; // the first breath
bool hasBreathed = false; // current step has breathed
bool lastPhaseBreathed = false; // last step has breathed (for varify)
bool breathedRight = true; // if breath is correct or not

void setup()
{
  Serial.begin(9600);
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on.

  // accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B);                 // PWR_MGMT_1 register
  Wire.write(0);                    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // properties
  pinMode(lightOut, OUTPUT);
  pinMode(button, INPUT);
}

void loop()
{
  // changing state when push button
  int buttonOn = digitalRead(button);
  if (buttonOn == HIGH)
  {
    state = !state;
    count = 0;
    startStep = 0;
    delay(200);
  }
  if (state && !passedTut) {
    drawTutorial();


  }
  if (state && passedTut) {
    doSporting();
  }
  if (!state) {
    doResting();
  }
}

void doSporting() {
  digitalWrite(lightOut, HIGH);

  int phase; // 2 breath out, 2 breath in, 4 phases in total.

  // detect step
  bool isMoving = getPace();
  if (isMoving) {
    count++;

    // verify last step's breath when firstly entering the new step
    if (!breathedRight && hasBreathed) { // restart a new loop when re-breathed
      breathedRight = true;
      startStep = count;
    }
    // normal verify
    phase = (count - startStep) % 4;
    lastPhaseBreathed = hasBreathed;
    hasBreathed = false;
    verifyBreath(phase, lastPhaseBreathed);
  } else {
    phase = (count - startStep) % 4;
  }

  // render display content
  draw(phase, breathedRight);

  // after verify, detect breath for this step again.
  bool isBreathing = getIsBreathing();
  if (isBreathing) {
    hasBreathed = true;
  }
}

void doResting() {
  digitalWrite(lightOut, LOW);
  u8g.setFont(u8g_font_helvR12);
  u8g.firstPage();
  do
  {
    u8g.drawStr(20, 20, "Rest");
    u8g.drawStr(20, 40, "Press to start");
  } while (u8g.nextPage());
}

bool verifyBreath(int phase, bool lastPhaseBreathed)
{
  bool cond1 = (phase == 0) && !lastPhaseBreathed;
  bool cond2 = (phase == 1) && !lastPhaseBreathed;
  bool cond3 = (phase == 2) && lastPhaseBreathed;
  bool cond4 = (phase == 3) && lastPhaseBreathed;
  if (cond1 || cond2 || cond3 || cond4) {
    breathedRight = false;
  }
}
/**/
bool getIsBreathing()
{
  int arr[100];
  int sum = 0;
  for (int i = 0; i < 100; i++)
  {
    arr[i] = analogRead(soundPin);
  }
  sort(arr);

  sum = (arr[50] + arr[51]) / 2;

  if (sum > 100) { //baseline is 70
    return true;

  }
  return false;
}

void sort(int myArr[]) // bubble sort
{
  int len = sizeof(myArr) / sizeof(myArr[0]); // get array lenth: total-byte-lenth / first-element-byte-lenth

  for (int i = 0; i < len - 1; i++) // bubble sort
  {
    for (int j = 0; j < len - i - 1; j++)
    {
      if (myArr[j] > myArr[j + 1])
      {
        int temp = myArr[j];
        myArr[j] = myArr[j + 1];
        myArr[j + 1] = temp;
      }
    }
  }
}

/*
  Referenced from: http://www.mschoeffler.de/2017/10/05/tutorial-how-to-use-the-gy-521-module-mpu-6050-breakout-board-with-the-arduino-uno/
*/
bool getPace() { // get pace by reading from accelerometer
  int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
  int16_t gyro_x, gyro_y, gyro_z;                            // variables for gyro raw data
  int16_t temperature;
  int movementArr[11];
  // variables for temperature data
  for (int i = 0; i < 11; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
    Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
    Wire.requestFrom(MPU_ADDR, 7 * 2, true); // request a total of 7*2=14 registers

    // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
    accelerometer_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
    accelerometer_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
    accelerometer_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
    temperature = Wire.read() << 8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
    gyro_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
    gyro_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
    gyro_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)

    movementArr[i] = accelerometer_z; // use z-index value only
  }
  // end referencing
  sort(movementArr);
  int val = movementArr[6];


  if (val > 17000 && startedStep == 0) { //baseline is 4000. when running reading goes down
    startedStep = 1;
    return true;
  } else if (val < 14000) { // reset condition
    startedStep = 0;
  }
  return false;
}


// drawing
void draw(int phase, bool breathedRight)
{
  u8g.firstPage();
  do
  {
    if (breathedRight) {
      drawPhase(phase);
    } else {
      drawAdjust();
    }
    // pace
    char countStr[10];
    sprintf(countStr, "Step: %4d", count);
    u8g.setFont(u8g_font_helvR12);
    u8g.drawStr(5, 15, countStr);
  } while (u8g.nextPage());
}

void drawAdjust() {
  unsigned int timeNow = millis();
  timeNow = timeNow / 500;
  timeNow = timeNow % 2;
  if (timeNow == 0) {
    u8g.setFont(u8g_font_fur20);
    u8g.drawCircle(95, 32, 30);
    u8g.drawStr(5, 45, "Out");
  }
}

void drawPhase(int phase) {
  u8g.setFont(u8g_font_fur20);
  switch (phase)
  {
    case 0: //Out 2
      u8g.drawCircle(95, 32, 15);
      u8g.drawStr(5, 45, "Out");
      break;
    case 1: // In 1
      u8g.drawDisc(95, 32, 15);
      u8g.drawStr(5, 45, "In");
      break;
    case 2: // In 2
      u8g.drawDisc(95, 32, 30);
      u8g.drawStr(5, 45, "In");
      break;
    case 3: // Out 1
      u8g.drawCircle(95, 32, 30);
      u8g.drawStr(5, 45, "Out");
      break;

    default:
      u8g.drawStr(30, 30, "N/A");
  }

}

void drawTutorial() {
  u8g.setFont(u8g_font_4x6);
  preTut();

}

void preTut() {
  u8g.firstPage();
  do
  {
    u8g.drawStr(5, 20, "First let's have a tutorial");
    u8g.drawStr(5, 35, "We are doing 2:2 breathing");
    u8g.drawStr(5, 50, "Take a breath in first");
  } while (u8g.nextPage());

  delay(5000);
  u8g.firstPage();
  do
  {
    u8g.drawStr(5, 20, "We will have 4 cycle a round");
    u8g.drawStr(5, 35, "In step 1, 2, breath out");
    u8g.drawStr(5, 50, "Step 3, 4, breath in");
  } while (u8g.nextPage());

  delay(5000);

  u8g.firstPage();
  do
  {
    u8g.drawStr(5, 20, "Now let's start");
    passedTut = true;
  } while (u8g.nextPage());
  delay(3000);
}
