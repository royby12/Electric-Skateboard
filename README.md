# Electric-Skateboard

Detailed build story can be found at:
http://www.tecprojects.com/Electric_Skateboard/index.html

# Hardware
the sensors I used are:
* Current and Voltage sensor
* RGB LED indicator that I use to alert low battery or any other use
* OLED Display to see all data like speed, cruse, limit, lights and more
* GPS to determine speed, direction and display clock
* Pressure sensor to sense if I am still on the skateboard and other aplications
* Buzzer to buzz on low voltage
* Relay switch to control lights
* RC Receiver to control throttle with the remote
* Teensy 3.1 as a micro controller, this was the only one strong enough to hold all the sensors

# Menu
Each menu item is displayed on the OLED screen. These are the menu items:
* BUZZER - empty screen
* Lights ON/OFF
* BATTERY - Voltage
* CURRENT - Amp
* POWER - Watt
* MAXIMUMS - Max Amp, Watt and Speed
* GPS - Speed ,direction and satellite in view
* Time and Date
* Cruz control
* Force sensor ON/OFF - use for safety - it will stop the skateboard when I am not on it
* Speed limit ON/OFF
* Change the curve value
* Add/Remove Debugging screen
* Debugging Screen

# Libraries
I have used a several libraries in the software:
* Servo.h
* TinyGPS++.h
* SPI.h
* Wire.h
* Adafruit_GFX.h
* Adafruit_SSD1306.h
