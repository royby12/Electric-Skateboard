#include <servo.h>
#include <tinygps++.h>
#include <spi.h>
#include <wire.h>
#include <adafruit_gfx.h>
#include <adafruit_ssd1306.h>
//*********************************RC Variables Setup***************************************
Servo ThrottleServo;
 
int ThrottleOutPin = 5; //Goes to ESC
int ThrottleInPin = 7; // connect RC receiver here CH2 - throttle.
int ThrottleValue;     // The value goes pulseIn and then mapped to set the servo PWM
int smoothedThrottleValue = 0;    // smoothed result of the Throttle - This is what the servo will get
int samples = 4;    // amount of samples for smoothing
 
int WheelInPin = 8; // connect RC receiver here CH1 - throttle.
int WheelValue;
 
int ButtonInPin = 6; // connect RC receiver here CH3 - throttle.
int ButtonValue;
//******************************************************************************************
 
//************************************Menu Variables****************************************
int ScreenNumber = 0; //the position of the LCD (switch case menu)
int AmountOfScreens = 10; //how many screens there will be
//******************************************************************************************
 
//***************************************LED Variables**************************************    
int redp = 23;
int greenp = 22;
int bluep = 21;
//******************************************************************************************
 
//***************************************Buzzer PIN*****************************************
int buzzer = 4;
//******************************************************************************************
 
//***************************************Lights PINS to the Relay***************************
int breakLightPin = 12; //Break lights pin
int frontLightPin = 11; //Front lights pin
boolean lightsOn = false; //to check the light state
//******************************************************************************************
 
//***************************************Current & Voltage Sensor***************************
int voltPin = A0;
int ampPin = A1;
float Volt; //Analog 0 - Volt before conversion - Volt = Volt/12.99; //180 Amp board calculation
float Amp; //Analog 1 - Amp before conversion - Amp = Amp/3.7; //180 Amp board calculation
float maxAmp = 0; //Save the maximum Ampere 
float Watt; //Calculation of Volt*Amp
float maxWatt; //Save the maximum Watt 
float minVolt = 3.4*8; //27.2 volts - calculation of 3.4 for cell
 
//These vars will be stored as strings and hold the Volt and Amp data to display them in the OLED
char VoltChar[5];
char AmpChar[5];
char maxAmpChar[5];
//********************************************************************************************
 
//***************************************Force Sensor*****************************************
int forcePin = A2;
int forceValue;  //Call to - forceValue = analogRead(forcePin);
boolean forceOn = false;
//*********************************************************************************************
 
//***************************************GPS Variables*****************************************
TinyGPSPlus gps; // The TinyGPS++ object
int maxSpeed = 0; //Store the maximum GPS speed
static const uint32_t GPSBaud = 9600; //Baud rate
//*********************************************************************************************
 
//***************************************OLED Variables****************************************
Adafruit_SSD1306 display(4); //attach the OLED
boolean printed = false; //Print only once when on the menu
//*********************************************************************************************
 
//***************************************Speed limit Variables*********************************
int speedLimit = 1700;  //Speed limit is limitation to the smoothedThrottleValue in microseconds
boolean speedLimitOn = false; //ON/OFF the speed limit
//*********************************************************************************************
 
//***************************************Force Control Variables*********************************
const int buttonPin = 2;     // the number of the pushbutton pin
const int ledPin =  20;  // red LED on the panel
boolean forceControl = false; //ON/OFF the force control
int button_delay = 0; //Long press short press detection
//*********************************************************************************************
 
//***************************************Cruz Control Variables*********************************
boolean cruzOn = false; //ON/OFF the Cruz control
int cruzValue; //to overwrite throttle value 
//*********************************************************************************************
 
//***************************************Curve*************************************************
float curve = 0;
//*********************************************************************************************
 
 
//boolean remoteOff = true; // Optional - require to press ch3 to start the program
 
void setup(){
  /*while(remoteOff){ // Optional - require to press ch3 to start the program
    if (pulseIn(ButtonInPin, HIGH, 20000) > 1800){
        remoteOff = false;
    }
    Serial.println("Your remote is OFF");
  }*/
 
  Serial.begin(9600); //For Debugging and/or GPS
  ThrottleServo.attach(ThrottleOutPin);  // attaches the servo on pin ThrottleOutPin to the servo object
   
  pinMode(redp, OUTPUT);   //RGB LED setup
  pinMode(greenp, OUTPUT);
  pinMode(bluep, OUTPUT);
   
  pinMode(ThrottleInPin, INPUT);  //RC Receiver inputs 
  pinMode(WheelInPin, INPUT);
  pinMode(ButtonInPin, INPUT);
   
  pinMode(buzzer, OUTPUT); // set a pin for buzzer output
   
  pinMode(breakLightPin, OUTPUT); // set the relay lights pins
  pinMode(frontLightPin, OUTPUT);
   
  pinMode(ledPin, OUTPUT);      //Force Control initialize - Panel
  pinMode(buttonPin, INPUT);  
   
  Serial1.begin(GPSBaud);//Start the Serial with the GPS Baud
  /*//Initial the GPS in case it's battery is dead
  Serial.write("$PMTK220,300*2D\r\n");//updates every 300 milliseconds
  Serial.write("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n");//Send the GGA and RMC sentences
  Serial.write("$PMTK251,9600*17\r\n");//Set lower baud rate
  Serial.begin(9600);//Begin new baud rate*/
   
  setColor(0,0,255);//Just blink the Green LED for two seconds
   
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.setTextColor(WHITE);//Set the color to white 
  //Say Hi to myself :)
  display.clearDisplay(); 
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("Hello");
  display.setTextSize(5);
  display.println("Roy");
  display.display(); 
  delay(3000);
   
  setColor(0,0,0); //turn the LED off
}
 
void loop(){
 
  if(!forceControl){// use the remote if force control is off
    ThrottleValue = pulseIn(ThrottleInPin, HIGH, 20000); //read throttle value  
  }else{ //use the force sensor if force control is off
    while(analogRead(forcePin)>850){
        if(ThrottleValue>speedLimit)
            ThrottleValue = speedLimit;
        ThrottleServo.writeMicroseconds(ThrottleValue);
        ThrottleValue++;
        delay(60);
    }
    ThrottleValue=1530 ;
 }
 smoothedThrottleValue = smoothedThrottleValue + ((ThrottleValue - smoothedThrottleValue)/samples); //Smooth calculation of the value
  
 if (smoothedThrottleValue >= 1500 && curve != 0){//Curve the throttle value
    smoothedThrottleValue = fscale( 1500, 2000, 1500, 2000, smoothedThrottleValue, curve);
 }
  
  if (cruzOn){//set to throttle the Cruz value if Cruz Control is on
    if (smoothedThrottleValue<=cruzValue){smoothedThrottleValue=cruzValue;}//change smoothedThrottleValue to the Cruz value
    else{cruzOn = false; printed = false; cruzValue = 0;}//turn OFF the cruz vontrol if throttle exceed the cruz value
  }
   
  if(speedLimitOn){  //Check if Speed limit is on and change smoothedThrottleValue accordingly
    if(smoothedThrottleValue>=speedLimit){smoothedThrottleValue = speedLimit;}//check if throttle exceed the limit value
  }
    
  if(forceOn){ //Check if force is activated and write to throttle
    forceValue = analogRead(forcePin);// Read the force sensor
    if(forceValue>100){ //Check for force
        ThrottleServo.writeMicroseconds(smoothedThrottleValue); // Write to Throttle if there is force
    }else{ThrottleServo.writeMicroseconds(1000);}  //In case there is no force, stop the skateboard
  }
  else{ThrottleServo.writeMicroseconds(smoothedThrottleValue);} //Write to Throttle if force is OFF
   
  if(lightsOn && smoothedThrottleValue < 1400){digitalWrite(breakLightPin, HIGH);}//Turn ON the Break Lights
  else if(lightsOn){digitalWrite(breakLightPin, LOW);} //Turn OFF the Break Lights
   
  if (digitalRead(buttonPin) == HIGH) { //toggle the forceControl option or change screens through button
    button_delay = 0; // Reset the 'timer' to detect long or short press
    while(digitalRead(buttonPin) == HIGH){//start counting the time
        button_delay++;
        delay(1);
    }
    if(button_delay <= 500){ //on short press increase the screen #
        ScreenNumber++;
        ScreenNumber=((ScreenNumber % AmountOfScreens) + AmountOfScreens) % AmountOfScreens;  //Always positive Modulo
        printed = false;//Print to the OLED on screen change
    }else{ //On long press toggle the forceControl option
        if(forceControl==false){//turn ON the forceControl
            forceControl = true;
            digitalWrite(ledPin, HIGH);
            forceOn = false;
        }
        else{ //turn OFF the forceControl
            forceControl = false;
            digitalWrite(ledPin, LOW);
            forceOn = true; //Turn off the force on limitation
        }
    }
  }
   
  //Measure the current and voltage
  Volt = analogRead(voltPin);
  Amp = analogRead(ampPin);
  //Convert the voltage and the amp
  Volt = Volt/19.3;//12.99 for 5V; //180 Amp board 
  Amp = Amp/5.7;//3.7 for 5V; //180 Amp board
  if(Volt<=minVolt){setColor(255,0,0);buzz(buzzer, 2500 , 50);ScreenNumber = 2;printed = false;}//Low voltage warning with RED LED, BUZZ and set to display voltage
  else{setColor(0,255,0);}//Green LED if BATT is OK
  if(Amp>maxAmp){maxAmp = Amp; maxWatt = Volt*maxAmp;} //Save the Maximum Ampere and Watt
   
  //Read from GPS
  while (Serial1.available() > 0)
    gps.encode(Serial1.read());
  if(gps.speed.kmph()>maxSpeed){maxSpeed = gps.speed.kmph();} //Save the Maximum Speed
 
  ButtonValue = pulseIn(ButtonInPin, HIGH, 20000); //Listen to the Button value
  WheelValue = pulseIn(WheelInPin, HIGH, 20000); // Listening to the Wheel value and setting what need to be displayed
   
  if (WheelValue > 1800){  //increase the ScreenNumber Value for the menu
    ScreenNumber++;
    ScreenNumber=((ScreenNumber % AmountOfScreens) + AmountOfScreens) % AmountOfScreens;  //Always positive Modulo
    printed = false;//Print to the OLED on screen change
    delay(300);
  }else if(WheelValue <1200){  //decrease the ScreenNumber Value
    ScreenNumber--;
    ScreenNumber=((ScreenNumber % AmountOfScreens) + AmountOfScreens) % AmountOfScreens; //Always positive Modulo
    printed = false;//Print to the OLED on screen change
    delay(300); 
  }
   
  //Print the screen # for debugging 
  Serial.print("screen # :");
  Serial.println(ScreenNumber);
   
  /*Menu items:
    0 - BUZZER   - empty screen
    1 - ON/OFF   - Lights
    2 - BATTERY  - Voltage
    3 - CURRENT  - Amp
    4 - POWER    - Watt
    5 - MAXIMUMS - Max Amp, Watt and Speed
    6 - GPS      - Speed , direction and satellite in view - Alway update screen 
    7 - Time and Date - Alway update screen
    8- Cruz control
    9- settings
        0 - ON/OFF   - Force sensor
        1 - ON/OFF   - Speed limit
        2 - Change the curve value
        3 - Add / remove Debugging screen (10)
        4 - Exit
    10- Debugging Screen
  */
   
  switch (ScreenNumber) { //Select the screen to display
    case 0: //BUZZER - empty screen
        //Print to the OLED blank screen
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay(); // Clear the buffer.
            display.display();  //Display whatever in the buffer
            printed = true;
        }
        if (ButtonValue > 1800){  //Buzz when pressed 
            buzz(buzzer, 2500 , 100); // buzz the buzzer on pin 'buzzer' at 500Hz for 150 milliseconds
        }
      break;
    case 1:  // ON/OFF the lights
        if(!printed){  //Print only once if status wasn't changed
            display.clearDisplay(); 
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("LIGHTS:");
            display.setTextSize(4);
            if(lightsOn){display.println("ON");}
            else{display.println("OFF");}
            display.display();  
            printed = true;
        }
        if (ButtonValue > 1800){  //Change the 'lightsOn' variable alternately
            if(lightsOn){
                lightsOn = false;
                digitalWrite(frontLightPin, LOW); //Turn the Front lights OFF
                digitalWrite(breakLightPin, LOW); //Turn the Break lights OFF
            }else{
                lightsOn = true;
                digitalWrite(frontLightPin, HIGH);  //Turn the Front lights ON
            }
            printed = false; //It will print to the changes to the display in the next round
            delay(300);
        }
        break;
    case 2:  // BATTERY - Voltage
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("BATTERY:");
            display.setTextSize(3);
            display.println(dtostrf(Volt,5, 2, VoltChar));
            display.setTextSize(2);
            display.println("Volts");
            display.display();
            printed = true;
        }
        if (ButtonValue > 1800){  //Click the button to refresh the screen
            printed = false; //It will print to the changes to the display in the next round
        }
        break;
    case 3:  // CURRENT - Amp
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("CURRENT:");
            display.setTextSize(3);
            display.println(dtostrf(Amp,5, 2, AmpChar));
            display.setTextSize(2);
            display.println("Amp");
            display.display();
            printed = true;
        }
        if (ButtonValue > 1800){  //Click the button to refresh the screen
            printed = false; //It will print to the changes to the display in the next round
        }
        break;
    case 4:  // POWER - WATT
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("POWER:");
            display.setTextSize(3);
            display.println((int)(Volt*Amp));
            display.setTextSize(2);
            display.println("WATT");
            display.display();
            printed = true;
        }
        if (ButtonValue > 1800){  //Click the button to refresh the screen
            printed = false; //It will print to the changes to the display in the next round
        }
        break;
    case 5:  // Max Amp and Watt
        if(!printed){//Print only once if 'printed' status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("Maximums:");
            display.print(dtostrf(maxAmp,5, 2, maxAmpChar));
            display.println(" A");
            display.print((int)maxWatt);//it crash the program
            display.println(" W");
            display.print(maxSpeed);//it crash the program
            display.println(" km/h");
            display.display();
            printed = true;
        }
        if (ButtonValue > 1800){  //Click the button to refresh the screen
            printed = false; //It will print to the changes to the display in the next round
        }
        break;
    case 6: // GPS Speed information
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.print("GPS: ");
            display.setTextSize(1);
            display.print("in view:");
            display.println(gps.satellites.value());
            display.setCursor(0,20);
            display.setTextSize(4);
            display.print((int)gps.speed.kmph()); //Speed in KMH
            display.setTextSize(1);
            display.println("KM/H");
            display.setCursor(85,43);
            display.setTextSize(2);
            display.println(TinyGPSPlus::cardinal(gps.course.value())); 
            display.display();
            //printed = true;
        }
        if (ButtonValue > 1800){  //Click the button to refresh the screen
            printed = false; //It will print to the changes to the display in the next round
        }
        break;
    case 7: // Show Time and Date
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("CLOCK:");
                display.print(gps.date.day());
                display.print(F("/"));
                display.print(gps.date.month());
                display.print(F("/"));
                display.print(gps.date.year());
                display.println();
                  
                if (gps.time.hour() < 10) display.print(F("0"));
                display.print(gps.time.hour()+2);//IL time
                display.print(F(":"));
                if (gps.time.minute() < 10) display.print(F("0"));
                display.print(gps.time.minute());
                display.print(F(":"));
                if (gps.time.second() < 10) display.print(F("0"));
                display.print(gps.time.second());
                display.println();
                 
            display.display();
            //printed = true;
        }   
        if (ButtonValue > 1800){  //Click the button to refresh the screen
            printed = false; //It will print to the changes to the display in the next round
        }
        break;
    case 8: // Cruz control
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("CRUZ:");
            display.setTextSize(4);             
            if(cruzOn){
                display.print(map(cruzValue, 1500, 2000, 0, 100)); // mapping the speedlimit to percent
                display.println("%"); 
            }
            else{display.println("OFF");}
            display.display();
            printed = true;
        }   
        if (ButtonValue > 1800){  //Click the button to refresh the screen
            cruzOn = true;
            cruzValue = smoothedThrottleValue; //save the throttle value as the cruz value
            printed = false; //It will print to the changes to the display in the next round
            delay(500);
        }
        break;
    case 9: // Settings
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("Settings:");
            display.setTextSize(1);
            display.println("ON/OFF Force sensor");
            display.println("ON/OFF Speed limit");
            display.println("Curve Value");
            display.println("Debug Mode");  
            display.display();
            printed = false;
        }
        if (ButtonValue > 1800){  //get inside the menu
                delay(300); //to give some time until I'll stop press the button
                int settingsMenu = 0;
                int AmountOfSettings = 5;
                boolean exit = false;
                while(!exit){  
                      ButtonValue = pulseIn(ButtonInPin, HIGH, 20000); //Listen to the Button value
                      WheelValue = pulseIn(WheelInPin, HIGH, 20000); // Listening to the Wheel value and setting what need to be displayed
  
                      if (WheelValue > 1800){  //increase the ScreenNumber Value for the menu
                        settingsMenu++;
                        settingsMenu=((settingsMenu % AmountOfSettings) + AmountOfSettings) % AmountOfSettings;  //Always positive Modulo
                        printed = false;//Print to the OLED on screen change
                        delay(300);
                      }else if(WheelValue <1200){  //decrease the settingsMenu Value
                        settingsMenu--;
                        settingsMenu=((settingsMenu % AmountOfSettings) + AmountOfSettings) % AmountOfSettings; //Always positive Modulo
                        printed = false;//Print to the OLED on screen change
                        delay(300); 
                      }
                       
                      switch (settingsMenu) { //Select the screen to display
                        case 0: //ON/OFF Force sensor
                            if(!printed){//Print only once if status wasn't changed
                                display.clearDisplay();
                                display.setTextSize(2);
                                display.setCursor(0,0);
                                display.println("FORCE:");
                                display.setTextSize(4);
                                if(forceOn){display.println("ON");}
                                else{display.println("OFF");}
                                display.display();
                                printed = true;
                            }
                            if (ButtonValue > 1800){  //Change the 'forceOn' variable alternately
                                if(forceOn){
                                    forceOn = false;
                                }else{
                                    forceOn = true;
                                }
                                printed = false; //It will print to the changes to the display in the next round
                                delay(300);
                            }
                          break;
                        case 1: // ON/OFF the Speed Limit
                            if(!printed){//Print only once if status wasn't changed
                                display.clearDisplay();
                                display.setTextSize(2);
                                display.setCursor(0,0);
                                display.println("LIMIT:");
                                display.setTextSize(4);
                                if(speedLimitOn){
                                    display.println("ON");
                                    display.setTextSize(1);
                                    display.print(map(speedLimit, 1500, 2000, 0, 100)); // mapping the speedlimit to percent
                                    display.println("%");
                                    }
                                else{display.println("OFF");}
                                display.display();
                                printed = true;
                            }
                            if (ButtonValue > 1800){  //Change the 'speedLimitOn' variable alternately
                                if(speedLimitOn){ //Turn OFF the speed limit
                                    speedLimitOn = false;
                                }else{ //If it's OFF then Set the speed limit value first and then turn it ON
                                    delay(300); //to give some time until I'll stop press the button
                                    //setting the speed limit value
                                    while(pulseIn(ButtonInPin, HIGH, 20000) < 1800){  //Need to press CH3 to set the value and exit this loop
                                        WheelValue = pulseIn(WheelInPin, HIGH, 20000);//Read the Wheel to increase or decrease the Speedlimit value
                                          if (WheelValue > 1800){  //increase the speedLimit Value
                                            speedLimit-=5;
                                            if(speedLimit<1500){speedLimit=2000;} //limit the lower boundary to 1500
                                          }else if(WheelValue <1200){  //decrease the speedLimit Value
                                            speedLimit+=5;
                                            if(speedLimit>2000){speedLimit=1500;}
                                          }
                                        //Display the changes to the speedLimit value
                                        display.clearDisplay();
                                        display.setTextSize(2);
                                        display.setCursor(0,0);
                                        display.println("LIMIT:");
                                        display.setTextSize(1);
                                        display.println("Use the wheel");
                                        display.setTextSize(4);
                                        display.print(map(speedLimit, 1500, 2000, 0, 100)); // mapping the speedlimit to percent
                                        display.println("%"); 
                                        display.setTextSize(1);
                                        display.println("Press CH3 to set");
                                        display.display();              
                                    }
                                    speedLimitOn = true; //Turn the speed limit on after setting the value
                                }
                                printed = false; //It will print to the changes to the display in the next round
                                delay(300);
                            }
                            break;
                        case 2: // change the curve
                            if(!printed){//Print only once if status wasn't changed
                                display.clearDisplay();
                                display.setTextSize(2);
                                display.setCursor(0,0);
                                display.println("CURVE:");
                                display.setTextSize(3);
                                display.println(abs(curve));
                                display.display();
                                printed = true;
                            }
                            if (ButtonValue > 1800){  //Change the Curve value
                                    delay(300); //to give some time until I'll stop press the button
                                    //setting the speed limit value
                                    while(pulseIn(ButtonInPin, HIGH, 20000) < 1800){  //Need to press CH3 to set the value and exit this loop
                                        WheelValue = pulseIn(WheelInPin, HIGH, 20000);//Read the Wheel to increase or decrease the Speedlimit value
                                          if (WheelValue > 1800){  //increase the speedLimit Value
                                            curve-=0.1;
                                            if(curve<-1.5){curve=0;} //limit the lower boundary to 1500
                                          }else if(WheelValue <1200){  //decrease the speedLimit Value
                                            curve+=0.1;
                                            if(curve>0){curve=-1.5;}
                                          }
                                        //Display the changes to the speedLimit value
                                        display.clearDisplay();
                                        display.setTextSize(2);
                                        display.setCursor(0,0);
                                        display.println("CURVE:");
                                        display.setTextSize(1);
                                        display.println("Use the wheel");
                                        display.setTextSize(3);
                                        display.println(abs(curve)); // mapping the speedlimit to percent 
                                        display.setTextSize(1);
                                        display.println("Press CH3 to set");
                                        display.display();              
                                    }
 
                                printed = false; //It will print to the changes to the display in the next round
                                delay(300);
                            }
                            break;
                        case 3: // Add / remove Debugging screen
                            if(!printed){//Print only once if status wasn't changed
                                display.clearDisplay();
                                display.setTextSize(2);
                                display.setCursor(0,0);
                                display.println("Debug:");
                                display.setTextSize(4);
                                if(AmountOfScreens == 11){display.println("ON");}
                                else{display.println("OFF");}
                                display.display();
                                printed = true;
                            }
                            if (ButtonValue > 1800){  //Change the 'forceOn' variable alternately
                                if(AmountOfScreens == 11){
                                    AmountOfScreens = 10;
                                }else{
                                    AmountOfScreens = 11;
                                }
                                printed = false; //It will print to the changes to the display in the next round
                                delay(300);
                            }
                            break;
                        case 4: //Exit
                            if(!printed){//Print only once if status wasn't changed
                                display.clearDisplay();
                                display.setTextSize(3);
                                display.setCursor(10,20);
                                display.println("Exit");
                                display.display();
                                printed = true;
                            }
                            if (ButtonValue > 1800){  //Change the 'forceOn' variable alternately
                                exit = true;
                                printed = false; //It will print to the changes to the display in the next round
                                delay(300);
                            }
                          break;
                    }
                }
 
            printed = false; //It will print to the changes to the display in the next round
            delay(300);
        }
        break;
    case 10: // Debugging Screen
        if(!printed){//Print only once if status wasn't changed
            display.clearDisplay();
            display.setTextSize(2);
            display.setCursor(0,0);
            display.println("Debuging:");
            display.setTextSize(1);
            display.print("Smoothed T Val:");
            display.println(smoothedThrottleValue);
            display.print("CH2 Val: ");
            display.println(pulseIn(ThrottleInPin, HIGH, 20000));
            display.print("Force Val: ");
            display.println(analogRead(forcePin));
            display.print("Volt Pin: ");
            display.println(analogRead(voltPin));
            display.print("Amp Pin: ");
            display.println(analogRead(ampPin));            
            display.display();
            printed = false;
        }
        if (ButtonValue > 1800){  //refresh the screen
            printed = false; //It will print to the changes to the display in the next round
        }
        break;
    default: 
        Serial.println("Fail");
        Serial.println(ScreenNumber);
        // if nothing else matches, do the default
        // default is optional
  }
}
 
void setColor(int R, int G, int B){ // RGB LEG with analogWrite (This is not available when servo attached, this is just preparation for new board)
    R=255-R;
    G=255-G;
    B=255-B;
     
    analogWrite(redp, R); 
    analogWrite(greenp, G); 
    analogWrite(bluep, B);
}
void buzz(int targetPin, long frequency, long length) {  //BUZZ function
  long delayValue = 1000000/frequency/2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length/ 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to 
  //// get the total number of cycles to produce
 for (long i=0; i < numCycles; i++){ // for the calculated length of time...
    digitalWrite(targetPin,HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin,LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait againf or the calculated delay value
  }
}
float fscale( float originalMin, float originalMax, float newBegin, float newEnd, float inputValue, float curve){
 
  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;
 
 
  // condition curve parameter
  // limit range
 
  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;
 
  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output 
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function
 
  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }
 
  // Zero Refference the values
  OriginalRange = originalMax - originalMin;
 
  if (newEnd > newBegin){ 
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd; 
    invFlag = 1;
  }
 
  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float
 
  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine 
  if (originalMin > originalMax ) {
    return 0;
  }
 
  if (invFlag == 0){
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;
 
  }
  else     // invert the ranges
  {   
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange); 
  }
 
  return rangedValue;
}
</adafruit_ssd1306.h></adafruit_gfx.h></wire.h></spi.h></tinygps++.h></servo.h>
