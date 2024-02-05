#include <CapacitiveSensor.h>
 
CapacitiveSensor cs = CapacitiveSensor(8, 2);    
void setup()                    
{
  Serial.begin(9600);
}
 
void loop()                      
{
  Serial.println(cs.capacitiveSensorRaw(30)); 
  delay(100); 
}