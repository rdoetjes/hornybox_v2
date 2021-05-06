#include "hornio.hpp"

void hornio::setupPins(){
  wiringPiSetup(); //We use the wiringPi GPIO method (read their documentation)
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void hornio::hornDoubleTap(int msShortTap, int msLongTap){
  //curtosy tap, followed by a nice long one
  digitalWrite(1, HIGH);
  std::this_thread::sleep_for(chrono::milliseconds(msShortTap));
  digitalWrite(1, LOW);

  //pause
  std::this_thread::sleep_for(chrono::milliseconds(300));

  digitalWrite(1, HIGH);
  std::this_thread::sleep_for(chrono::milliseconds(msLongTap));
  digitalWrite(1, LOW);
}
