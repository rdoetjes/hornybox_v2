#ifndef HORNIO_H
#define HORNIO_H

#include <wiringPi.h>
#include <thread>
#include <chrono>

#define relayPin 1

using namespace std;
using namespace chrono;

class hornio{
  public:
    static void setupPins();
    static void hornDoubleTap(int msShortTap, int msLongTap);
};
#endif
