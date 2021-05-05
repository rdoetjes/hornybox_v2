# Basic Collision Avoidance Robot for Raspberry Pi

This is the software to control a Raspberry Pi robot.
It's a vehicle driven by two motors (in my case a tank track). The motors are controlled by an H bridge.
The low-level classes Motor and RangeSensor are easily reusable for your own projects.

Dependencies:
  WiringPi and pthread library.

Use this as an example on how to use C++ for your Robot oprojects instead of Python.

Run make to compile it on your pi (make sure you hae WitingPi and pthreads dependencies installed)
And the application is in build/apps/robot
