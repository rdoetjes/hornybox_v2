#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include <wiringPi.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <string.h> 
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

using namespace cv;
using namespace std;
using namespace chrono;

#define durationHorn 1000
#define relayPin 1
#define secondsBetweenHonks 30;

int dirExists(const char* const path)
{
    struct stat info;
    int statRC = stat( path, &info );

    if( statRC != 0 )
    {
        if (errno == ENOENT)  { return 0; } // something along the path does not exist
        if (errno == ENOTDIR) { return 0; } // something in path prefix is not a dir
        return -1;
    }
    return ( info.st_mode & S_IFDIR ) ? 1 : 0;
}

int NumberOfCascadeMatches(Mat *process, CascadeClassifier *cascade){
  vector<Rect> n;

  cascade->detectMultiScale(*process, n, 1.1, 2, 0|cv::CASCADE_SCALE_IMAGE, Size(100, 100) );
  return n.size();
}

void setupPins(){
  wiringPiSetup(); //We use the wiringPi GPIO method (read their documentation)
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void setupCam(VideoCapture *cap, int deviceID, int apiID){
  cap->open(deviceID + apiID);
  cap->set(CAP_PROP_BUFFERSIZE, 1);
  cap->set(CAP_PROP_FPS, 2);
}

int detectFrontalAndProfile(Mat *frame, CascadeClassifier *frontalFace, CascadeClassifier *profileFace){
  Mat process;
  cvtColor(*frame, process, COLOR_BGR2GRAY);
  return  NumberOfCascadeMatches(&process, frontalFace) + NumberOfCascadeMatches(&process, profileFace);
}

int main(){
  static VideoCapture cap;		   //default pi camera
  static Mat frame;			   //unaltered frame from camera
  static Mat process;			   //the image on which processing will happen
  const static int deviceID = 0;           // 0 = open default camera
  const static int apiID = cv::CAP_ANY;    // 0 = autodetect default API
  static time_t nextPossibleHonkTime = 0;      // when will new honk be sounded
  static time_t currentTime = 0;         // current time
  
  setupCam(&cap, deviceID, apiID);
	
  try{

    //setup the GPIO
    setupPins();

    // we wait the nextPossibleHonkTime milliseconds plus current time to arm the system
    nextPossibleHonkTime = time(nullptr);

    // check if we succeeded
    if (!cap.isOpened()) {
      std::cerr << "ERROR! Unable to open camera\n";
      return -1;
    }

    //setup haar qualifiers for frontal and profile detecion
    CascadeClassifier frontalFace = CascadeClassifier();
    frontalFace.load("cascades/haarcascade_frontalface_alt_tree.xml");

    CascadeClassifier profileFace = CascadeClassifier();
    profileFace.load("cascades/haarcascade_profileface.xml");

    while(1) {
      cap.read(frame);

      if( frame.empty() ) break; // end of video stream
	
      currentTime = std::time(nullptr);
      if ( (currentTime >= nextPossibleHonkTime) && 
           (detectFrontalAndProfile(&frame, &frontalFace, &profileFace)!=0) ) {

        digitalWrite(1, HIGH);
        std::this_thread::sleep_for(chrono::milliseconds(durationHorn));
        digitalWrite(1, LOW);
        nextPossibleHonkTime = currentTime + secondsBetweenHonks;
      }

      if ( (waitKey(1) & 0xFF) == 'q')
        break;

    }   
  }catch(const Exception &e){
    cerr << e.what() << endl;
  }

  cap.release();
  return 0;
}
