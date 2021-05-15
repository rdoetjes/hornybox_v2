#include <wiringPi.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <vector>
#include <string.h> 
#include <time.h>

#include "vision.hpp"
#include "hornio.hpp"
#include "deamon.hpp"
#include "webservice.hpp"
#include "global.hpp"

using namespace std;
using namespace chrono;

void faceDetect(){
  static VideoCapture cap;		             //default pi camera
  static Mat frame;			                   //unaltered frame from camera
  const static int deviceID = 0;           // 0 = open default camera
  const static int apiID = cv::CAP_ANY;    // 0 = autodetect default API
  const static string picPath = "./web/pictures";
  static time_t nextPossibleHonkTime = 0;  // when will new honk be sounded
  static time_t currentTime = 0;           // current time
  static vector<CascadeClassifier> cascades;

  vision::setupCam(&cap, deviceID, apiID);
	
  try{
    // we wait the nextPossibleHonkTime milliseconds plus current time to arm the system
    nextPossibleHonkTime = time(nullptr) + secondsBetweenHonks;

    // check if we succeeded
    if (!cap.isOpened()) {
      syslog (LOG_ALERT, "Camera could not be found or opened!!!");
      std::cerr << "ERROR! Unable to open camera\n";
      exit(1);
    }

    //setup haar qualifiers for frontal and profile detecion
    CascadeClassifier frontalFace = CascadeClassifier();
    frontalFace.load("cascades/haarcascade_frontalface.xml");
    cascades.push_back(frontalFace);

    //main processing loop
    while(1) {
      cap.read(frame);
	
      if( frame.empty() ) break; // end of video stream

      currentTime = std::time(nullptr);
      if (( FACE_DETECT == 1) && (currentTime >= nextPossibleHonkTime) && (vision::getNrOfCascadeMatches(&frame, &cascades) > 0) ) {
        std::thread t1(hornio::hornDoubleTap, msShortTapHorn, msLongTapHorn);
        std::thread t2(vision::snapPictures, &cap, picPath, 5);
        t1.join();
        t2.join();

        nextPossibleHonkTime = time(nullptr) + secondsBetweenHonks;
      }
      waitKey(1);
    }   
  }catch(const Exception &e){
    syslog (LOG_ALERT, e.what());
    cerr << e.what() << endl;
    exit(1);
  }
  cap.release();
}

int main(){
  //turn into a deamon process
  deamon("/home/pi/hornybox_v2");

  //setup the GPIO
  hornio::setupPins();

  //run detection thread
  std::thread detectThread(faceDetect);

  //set up webservices
  webserver ws = create_webserver(8080);
  webservice hwr;
  ws.register_resource("/honk", &hwr, true);
  ws.register_resource("/facedetect", &hwr, true);
  syslog (LOG_INFO, "Webservice is listening on port: 8080");

  ws.start(true); 

  return 0;
}
