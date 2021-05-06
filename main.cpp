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
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

using namespace cv;
using namespace std;
using namespace chrono;

#define msLongTapHorn 300
#define msShortTapHorn 300
#define relayPin 1
#define secondsBetweenHonks 30

static void deamon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/home/pi/hornybox_v2");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("hornybox", LOG_PID, LOG_DAEMON);
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

void hornDoubleTap(int msShortTap, int msLongTap){
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

void snapPictures(VideoCapture *cap, string picsFolder, int numberOfPics){
  Mat snapShot;
  for (int i=0; i< numberOfPics; ++i){
    cap->read(snapShot);
    imwrite(picsFolder + "/" + to_string( time(nullptr) ) + "_" + to_string(i) + ".jpg", snapShot);
  }
}

int main(){
  //turn into a deamon process
  deamon();

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
    nextPossibleHonkTime = time(nullptr) + secondsBetweenHonks;

    // check if we succeeded
    if (!cap.isOpened()) {
      syslog (LOG_ALERT, "Camera could not be found or opened!!!");
      std::cerr << "ERROR! Unable to open camera\n";
      return -1;
    }

    //setup haar qualifiers for frontal and profile detecion
    CascadeClassifier frontalFace = CascadeClassifier();
    frontalFace.load("cascades/haarcascade_frontalface_alt_tree.xml");

    CascadeClassifier profileFace = CascadeClassifier();
    profileFace.load("cascades/haarcascade_profileface.xml");

    //main processing loop
    while(1) {
      cap.read(frame);

      if( frame.empty() ) break; // end of video stream
	
      currentTime = std::time(nullptr);
      if ( (currentTime >= nextPossibleHonkTime) && 
           (detectFrontalAndProfile(&frame, &frontalFace, &profileFace) > 0) ) {
        std::thread t1(hornDoubleTap, msShortTapHorn, msLongTapHorn);
        std::thread t2(snapPictures, &cap, "./pictures", 10);
        t1.join();
        t2.join();
        nextPossibleHonkTime = time(nullptr) + secondsBetweenHonks;
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
