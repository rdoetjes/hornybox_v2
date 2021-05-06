#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
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
#define secondsBetweenHonks 20 

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


void drawCascadeMatches(Mat *frame, vector<Rect> n, double scale){
  int x1, y1, x2, y2;
  Rect r;
  Scalar color = Scalar(255,0,0);

  for(size_t  i=0; i < n.size(); ++i){
    r = n[i];
    x1 = cvRound(r.x*scale);
    x2 = cvRound((r.x + r.width-1)*scale);
    y1 = cvRound(r.y*scale);
    y2 = cvRound((r.y + r.height-1)*scale);
    cv::Rect ROI(Point(x1, y1), Point(x2, y2));
    cv::rectangle(*frame, ROI, color, 2, 0, 0);
  }
}

void setupPins(){
  wiringPiSetup(); //We use the wiringPi GPIO method (read their documentation)
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

void setupCam(VideoCapture *cap, int deviceID, int apiID){
  cap->open(deviceID + apiID);
  cap->set(CAP_PROP_BUFFERSIZE, 1);
  cap->set(CAP_PROP_FPS, 3);
}

int getNrOfCascadeMatches(Mat *frame, CascadeClassifier *frontalFace){
  Mat process;
  vector<Rect> n;

  resize(*frame, process, Size(), 0.5, 0.5, INTER_LINEAR ); 
  cvtColor(process, process, COLOR_BGR2GRAY);
  equalizeHist( process, process );
 
  frontalFace->detectMultiScale(process, n, 1.1, 3, 0|CASCADE_SCALE_IMAGE, Size(30, 30) ); 

  return n.size();
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
  const static int deviceID = 0;           // 0 = open default camera
  const static int apiID = cv::CAP_ANY;    // 0 = autodetect default API
  const static string picPath = "./pictures";
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
    frontalFace.load("cascades/haarcascade_frontalface.xml");

    //main processing loop
    while(1) {
      cap.read(frame);
	
      if( frame.empty() ) break; // end of video stream

      currentTime = std::time(nullptr);
      if ( (currentTime >= nextPossibleHonkTime) && 
           (getNrOfCascadeMatches(&frame, &frontalFace) > 0) ) {

	imwrite(picPath + "/debug_" +  to_string(time(nullptr)) + ".jpg", frame);

        syslog (LOG_INFO, "HONK HONK!");

        std::thread t1(hornDoubleTap, msShortTapHorn, msLongTapHorn);
        std::thread t2(snapPictures, &cap, picPath, 20);
        t1.join();
        t2.join();

        nextPossibleHonkTime = time(nullptr) + secondsBetweenHonks;
      }

      waitKey(1);
    }   
  }catch(const Exception &e){
    syslog (LOG_ALERT, e.what());
    cerr << e.what() << endl;
  }

  cap.release();
  return 0;
}
