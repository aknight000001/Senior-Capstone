#include <Wire.h>
#include "SparkFun_Qwiic_Keypad_Arduino_Library.h"
#include <LiquidCrystal_I2C.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"
#include <SparkFun_I2C_Mux_Arduino_Library.h>

KEYPAD keypad1; // Create instance of the keypad object
LiquidCrystal_I2C lcd(0x27,20,4); // Set up the LCD's number of columns and rows
QWIICMUX myMux; // Create instance of the Qwiic Mux object

boolean entered = false;
const int NUM_MEASUREMENTS = 50; // Number of measurements to take

int numDigs = 0;

float wtValHun = 0;
bool hunEnt = false;

float wtValTen = 0;
bool tenEnt = false;

float wtValOne = 0;
bool oneEnt = false;

float wtValTenth = 0;
bool tenthEnt = false;

float wtValHundreth = 0;
bool hundrethEnt = false;

float patientWt = 0;

NAU7802 scale1;
NAU7802 scale2;

void(* resetFunc) (void) = 0;


void setup(void) {
  Wire.begin();
  //Wire.setClock(400000);
  Serial.begin(9600);
  
  // Initialize MUX
  if (myMux.begin() == false) {
    Serial.println("Mux not detected. Freezing...");
    while (1);
  }
  Serial.println("Mux detected");

  myMux.setPort(2);
  // Initialize Keypad
  if (keypad1.begin() == false) {
    Serial.println("Keypad does not appear to be connected. Please check wiring. Freezing...");
    while (1);
  }
  Serial.println("Keypad connected");

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize Scales
  myMux.setPort(0);
  if (!scale1.begin()) {
    Serial.println("Scale 1 not detected.");
  }

  myMux.setPort(2); // Assuming port 2 for scale2, adjust as necessary
  if (!scale2.begin()) {
    Serial.println("Scale 2 not detected.");
  }

  LiquidCrystal_I2C lcd(0x27,20,4);
  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on the backlight
  lcd.setCursor(0, 0);
  //lcd.print("Qwiic Scale Demo");  // Display a message on the LCD
  delay(4000);

  // Initialize the Qwiic Scales
  //scale2.begin();
}

float poundsToGrams(float pounds) {
    const float conversionFactor = 453.592; // 1 pound is approximately equal to 453.592 grams
    return pounds * conversionFactor;
}


bool takeMeasurementsAndDisplay() {
  float totalWeight1 = 0.0;
  float totalWeight2 = 0.0;
  

  myMux.setPort(2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Taking Measurements");  // Display a message on the LCD

  myMux.setPort(0); // Select the first scale
  // Take 1000 measurements for scale 1
  for (int i = 0; i < NUM_MEASUREMENTS; i++) {
    float weight1 = scale1.getWeight(); // Get weight from the first scale
    totalWeight1 += weight1;
    Serial.print(totalWeight1);
    Serial.print("  ");
    Serial.println(i);
    delay(10); // Wait for a short delay between measurements to avoid overwhelming the scale
  }

  myMux.setPort(2); // Select the second scale
  // Take 1000 measurements for scale 2
  for (int i = 0; i < NUM_MEASUREMENTS; i++) {
    float weight2 = scale2.getWeight(); // Get weight from the second scale
    totalWeight2 += weight2;
    Serial.print(totalWeight2);
    Serial.print("  ");
    Serial.println(i);
    delay(10); // Wait for a short delay between measurements to avoid overwhelming the scale
  }

  // Calculate average weight for each scale
  float averageWeight1 = totalWeight1 / NUM_MEASUREMENTS; //grams
  float averageWeight2 = totalWeight2 / NUM_MEASUREMENTS; //grams

  averageWeight1 = (averageWeight1 / 1000) * 9.81; //Convert to N
  averageWeight2 = (averageWeight2 / 1000) * 9.81; //Convert to N

  float magnitude = calculateVectorMagnitude(averageWeight2, averageWeight1);
  float angle = calculateVectorAngle(averageWeight2, averageWeight1);

  myMux.setPort(2);
  lcd.init();
  lcd.backlight();
  // Display the average weights on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vector Magnitude (N): ");
  lcd.setCursor(0,1);
  lcd.println(magnitude);
  
  lcd.setCursor(0, 2);
  lcd.print("Vector Angle (deg): ");
  lcd.setCursor(0,3);
  lcd.println(angle);

  lcd.setCursor(8,3);
  lcd.print("(# = repeat)");
  entered = false;
  while(entered == false)
  {
    keypad1.updateFIFO();  // necessary for keypad to pull button from stack to readable register
  char button = keypad1.getButton();

  if (button == -1)
  {
    Serial.println("No keypad detected");
    delay(1000);
  }
  else if (button != 0)
  {
    if (button == '#'){
      return true;
    }
    else{
      delay(20000);
      return false;
    }
  }
  }

//  // Variables to hold total weights for averaging
//  float totalWeight1 = 0.0;
//  float totalWeight2 = 0.0;
//  const int numMeasurements = 50; // Number of measurements for averaging
//  
//  // Take measurements from scale 1
//  myMux.setPort(0); // Switch to scale 1
//  for (int i = 0; i < numMeasurements; i++) {
//    float weight = scale1.getWeight();
//    totalWeight1 += weight;
//    delay(100); // Delay between measurements
//  }
//  
//  // Take measurements from scale 2
//  myMux.setPort(2); // Switch to scale 2
//  for (int i = 0; i < numMeasurements; i++) {
//    float weight = scale2.getWeight();
//    totalWeight2 += weight;
//    delay(100); // Delay between measurements
//  }
//  
//  // Calculate averages
//  float averageWeight1 = totalWeight1 / numMeasurements;
//  float averageWeight2 = totalWeight2 / numMeasurements;
//  
//  // Display the average weights on the LCD
//  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print("Avg Wt 1: ");
//  lcd.print(averageWeight1, 2); // Display 2 decimal places
//  lcd.print(" g");
//  
//  lcd.setCursor(0, 1);
//  lcd.print("Avg Wt 2: ");
//  lcd.print(averageWeight2, 2); // Display 2 decimal places
//  lcd.print(" g");
//
//  // Pause before next loop iteration or further actions
//  delay(5000); // Display results for 5 seconds
}


void waitForStartCommand() {
  boolean startCommandReceived = false;
  while (!startCommandReceived) {
    keypad1.updateFIFO();  // necessary for keypad to pull button from stack to readable register
      char button = keypad1.getButton();
      if (button == '*') {
        startCommandReceived = true;
      }
    delay(100); // Polling delay
  }
}

float convertGramsToNewtons(float grams) {
  // Assuming Earth's gravity of 9.81 m/s^2 for conversion
  return grams * 9.81 / 1000; // Convert grams to kilograms, then multiply by Earth's gravity
}

void askForRepeat() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Repeat?");
  lcd.setCursor(0,1);
  lcd.print("*=Yes; #=No");
  
  boolean decisionMade = false;
  while (!decisionMade) {
    //if (keypad1.updateFIFO()) {
      char button = keypad1.getButton();
      if (button == '*') {
        decisionMade = true; // Will cause loop() to restart measurement cycle
      } else if (button == '#') {
        lcd.clear();
        lcd.print("Goodbye!");
        delay(2000);
        // Optionally implement a more definitive end, such as entering a low-power state
        decisionMade = true;
        while(1); // Halt program - replace with low-power or end logic as needed
      }
    //}
    delay(100); // Polling delay
  }
}

void promptWt()
{
    lcd.init();                      // initialize the lcd 
    lcd.clear();
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0,0); //col,rom
    lcd.print("Please enter patient");
    lcd.setCursor(0,1); //col,rom
    lcd.print("weight (lbs):");
    lcd.setCursor(0,2); //col,rom
    lcd.print("Press '#' when done");
    lcd.setCursor(14,1);
    

    while(entered == false)
    {
    keypad1.updateFIFO();  // necessary for keypad to pull button from stack to readable register
    char button = keypad1.getButton();

    if (button == -1)
    {
        lcd.print("No keypad detected");
        delay(1000);
    }
    else if (button != 0)
    {
        if (button == '#') entered = true;
        else if (button == '*') lcd.print(".");
        else 
        {
          lcd.print(button-48);
          if (wtValHun == 0 && hunEnt == false)
          {
            wtValHun = button-48;
            hunEnt = true;
          }
          else if (wtValTen == 0 && tenEnt == false)
          {
            wtValTen = button-48;
            tenEnt = true;
          }
          else if (wtValOne == 0 && oneEnt == false)
          {
            wtValOne = button-48;
            oneEnt = true;
          }
          else if (wtValTenth == 0 && tenthEnt == false)
          {
            wtValTenth = button-48;
            tenthEnt = true;
          }
          else if (wtValHundreth == 0 && hundrethEnt == false)
          {
              wtValHundreth = button-48;
              hundrethEnt = true;
          }
        }
    }

    //Do something else. Don't call your Keypad a ton otherwise you'll tie up the I2C bus
    delay(100); //25 is good, more is better
   }

   patientWt = 100*wtValHun + 10*wtValTen + wtValOne + wtValTenth/10 + wtValHundreth/100;

  lcd.clear();
  lcd.setCursor(0,0); //col,row
  lcd.print("Is ");
  lcd.print(patientWt);
  lcd.print(" lbs");
  lcd.setCursor(0,1); //col,row
  lcd.print("correct? (*=Y, #=N)");

  entered = false;
  while(entered == false)
  {
    keypad1.updateFIFO();  // necessary for keypad to pull button from stack to readable register
  char button = keypad1.getButton();

  if (button == -1)
  {
    Serial.println("No keypad detected");
    delay(1000);
  }
  else if (button != 0)
  {
    if (button == '#'){
      resetFunc();
    }
    else if (button == '*'){
      entered = true;
    }
  }

  //Do something else. Don't call your Keypad a ton otherwise you'll tie up the I2C bus
  delay(100); //25 is good, more is better

  patientWt = poundsToGrams(patientWt);
  }
}

float calculateVectorMagnitude(float xForce, float yForce) {
    return sqrt(pow(xForce, 2) + pow(yForce, 2));
}

float calculateVectorAngle(float xForce, float yForce) {
    float angleRadians = atan2(yForce, xForce); // Get the angle in radians
    float angleDegrees = angleRadians * (180.0 / PI); // Convert radians to degrees
    return angleDegrees;
}


void loop() {
  lcd.init();
  lcd.clear();
  lcd.setCursor(0,0); //col,row
  lcd.print("Press * to start");
  waitForStartCommand();
  promptWt(); // Prompt user for patient weight

  bool rep = false;
  rep = takeMeasurementsAndDisplay();
  while(rep = true) {
    rep = takeMeasurementsAndDisplay();
  }
  askForRepeat();
}
