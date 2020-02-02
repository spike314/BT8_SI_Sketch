

#ifdef LIFEPO4
//Battery Level Set Up (LiFePo4)
#define BAT_RANGE 3.0 // BAT_HIGH - BAT_LOW in mv divided by 100 (to facilitate conversion to percent)
#define BAT_HIGH 3350 // Fully charged battery pack in mV (lifepo4)
#define BAT_LOW 3050  //Drained battery pack in mV (this is fine for controller, but min for BME280 and for the LiFePo4 battery)
#endif

#ifdef CR2032
//Battery Level Set Up (CR2032)
#define BAT_RANGE 17.0 // BAT_HIGH - BAT_LOW in mv divided by 100 (to facilitate conversion to percent)
#define BAT_HIGH 3400  // Fully charged battery pack in mV (CR2032)
#define BAT_LOW 1700   //Drained battery pack in mV
#endif


class BatteryLevel
{
  // Class Member Variables
  // These are initialized at startup
  uint8_t BatInt;
  float Bat;
  float lastBat; // static to preserve from one call of the function to the next.
#ifdef SENDVOLTAGE
  MyMessage Msg_battery;
#endif //SENDVOLTAGE

  // These maintain the current state
  unsigned long previousMillis, intervalBat;

public:
  // Default Constructor
  BatteryLevel()
  {
    intervalBat = 1800000;
    previousMillis = 0;
#ifdef SENDVOLTAGE
    Msg_battery.setSensor(244);
    Msg_battery.setType(V_VOLTAGE);
#endif //SENDVOLTAGE
  }

  BatteryLevel(const uint8_t sensor, unsigned long interval)
  {
    intervalBat = interval;
    previousMillis = 0;
#ifdef SENDVOLTAGE
    Msg_battery.setSensor(sensor);
    Msg_battery.setType(V_VOLTAGE);
#endif //SENDVOLTAGE
  }

  void update(unsigned long current)
  {
    // Store the current millis
    unsigned long currentMillis = current;

    Bat = (float)hwCPUVoltage(); //take voltage measurement
#ifdef MY_DEBUG
    Serial.print("\nBat //  lastBat:  ");
    Serial.print(Bat);
    Serial.print(" //  ");
    Serial.println(lastBat);

#endif // MY_DEBUG

    if (Bat > BAT_LOW)
    {
#ifdef MY_DEBUG
      Serial.println("Bat > BAT_LOW");

#endif // MY_DEBUG
      if ((currentMillis - previousMillis) > intervalBat)
      {
        BatInt = (uint8_t)((Bat - BAT_LOW) / BAT_RANGE);
        sendBatteryLevel(BatInt);
#ifdef SENDVOLTAGE
        send(Msg_battery.set(Bat / 1000.0, 3)); //send battery voltage with 3 decimal places
#endif  //SENDVOLTAGE
        previousMillis = currentMillis;
#ifdef MY_DEBUG
        Serial.print("\nBat //  BatInt:  ");
        Serial.print(Bat);
        Serial.print(" //  ");
        Serial.println(BatInt);
#endif // MY_DEBUG

        lastBat = Bat;
      }
      else
      {
#ifdef MY_DEBUG
        Serial.println("No bat update -not time yet.");
#endif // MY_DEBUG
      }
    }
    else
    { //Bat <= BAT_LOW
      sendBatteryLevel(0);
#ifdef SENDVOLTAGE
      send(Msg_battery.set(Bat / 1000.0, 3)); //send battery voltage with 3 decimal places
#endif  //ID_BATTERY
#ifdef LIFEPO4
      sleep(0); //Go to sleep forever to try and save the battery (bad for LifePo4 to get less than 2.8V)
#endif          //LIFEPO4
    }
  }
};