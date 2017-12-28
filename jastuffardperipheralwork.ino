#include <QueueArray.h>
#include <CurieBLE.h>

BLEService tempservice("19B10010-E8F2-537E-4F6C-D104768A1214");
BLEService heartservice("19B10020-E8F2-537E-4F6C-D104768A1214");
QueueArray<int> lastBeats;
QueueArray<double> lastTemperature;

bool beatStarted;
int Signal;
byte heartpin = 2;
double temp = 3;
byte siignal;
int threshold = 550;
unsigned long sumBeats = 0;
double sumTemperature = 0;
unsigned long starttime = 0;
unsigned long beats;
double temper;

// BLE Battery Level Characteristic"
BLEUnsignedCharCharacteristic heartchar("19B10021-E8F2-537E-4F6C-D104768A1214",  // standard 16-bit characteristic UUID
    BLERead | BLENotify);
 BLEUnsignedIntCharacteristic tempchar("19B10011-E8F2-537E-4F6C-D104768A1214", 
    BLERead | BLENotify);// remote clients will be able to
// get notifications if this characteristic changes

int oldBatteryLevel = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the battery level was checked, in ms
long start = 0;
void setup() {
  pinMode(13,OUTPUT);
  pinMode(temp,INPUT);
  pinMode(heartpin,INPUT);
  Serial.begin(9600);      
  // begin initialization
  BLE.begin();
 start = millis();

  /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet
  */
  BLE.setLocalName("UnderMyCare");
  BLE.setAdvertisedService(tempservice); 
  BLE.setAdvertisedService(heartservice);
  tempservice.addCharacteristic(tempchar);
  heartservice.addCharacteristic(heartchar);
  BLE.addService(tempservice); 
  BLE.addService(heartservice);
  

  /* Start advertising BLE.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}
void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if (central) {
    // Turn on LED when connected to device
    digitalWrite(13, HIGH);
    
    start = millis();
    // check the battery level every 200ms
    // as long as the central is still connected:
    while (central.connected()) {

      // Heart rate
      long curr = millis();
      // Every 15 seconds, send the value to the phone
      if(curr - start>=14095 && curr-start<=15005){
        start = curr;
        heartchar.setValue(beats*4);
        beats=0;
      }
      
      // Measure the value
      Signal = analogRead(A0);

      
      // Add the measurements to the queue
      if(lastBeats.count() >= 200){
        sumBeats -= lastBeats.dequeue();
      }
      lastBeats.enqueue(Signal);
      sumBeats+= Signal;

      // Output the cardiograph on the Serial Plotter
//      Serial.print(Signal);
//      Serial.print("\t");
//      Serial.print(sumBeats/200);

      // See if a beat has started
      if(Signal > (sumBeats/200)){
        beatStarted = true;
      }
      else if(beatStarted){
        beats++;
        beatStarted = false;
      }


      // Temperature
      double temperature = analogRead(A1);
      temperature *= 0.48828125; // Magic number

      
      if(lastTemperature.count() >= 20){
        sumTemperature -= lastTemperature.dequeue();
      }
      lastTemperature.enqueue(temperature);
      sumTemperature+= temperature;


      delay(50);

      tempchar.setValue((int)sumTemperature/2);
//      Serial.print("\t");
//      Serial.print(sumTemperature/20);
//      Serial.print("\t");
//      Serial.println(temperature);
  //    tempchar.setValue(temperature);
    }
  }
    // when the central disconnects, turn off the LED:
  digitalWrite(13, LOW);  
}
