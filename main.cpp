#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include <wiringPi.h>
#include <iostream>
#include <iomanip>
#include <string.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>

using namespace cv;
using namespace std;


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

int NumberOfCascadeMatches(Mat *frame, Mat *process, CascadeClassifier *cascade, double scale){
  vector<Rect> n;

  cascade->detectMultiScale(*process, n, 1.1, 2, 0|cv::CASCADE_SCALE_IMAGE, Size(100, 100) );
  return n.size();
}

int main(){
  VideoCapture cap;		   //default pi camera
  Mat frame;			   //unaltered frame from camera
  Mat process;			   //the image on which processing will happen
  static int nrBodies = 0;	   //number of torsos found in a frame
  int deviceID = 0;                // 0 = open default camera
  int apiID = cv::CAP_ANY;         // 0 = autodetect default API

  cap.open(deviceID + apiID);

  CascadeClassifier frontalFace = CascadeClassifier();
  frontalFace.load("cascades/haarcascade_frontalface_alt_tree.xml");

  CascadeClassifier profileFace = CascadeClassifier();
  profileFace.load("cascades/haarcascade_profileface.xml");

  // check if we succeeded
  if (!cap.isOpened()) {
    std::cerr << "ERROR! Unable to open camera\n";
    return -1;
  }

  try{
    while(1) {
      cap.read(frame);

      if( frame.empty() ) break; // end of video stream
	
      cvtColor(frame, process, COLOR_BGR2GRAY);
      nrBodies = NumberOfCascadeMatches(&frame, &process, &frontalFace, 1.0);
      nrBodies += NumberOfCascadeMatches(&frame, &process, &profileFace, 1.0);
      cout << nrBodies << endl;

      //wait long that way we don't load the cpu and safe battery
      //if (waitKey(1) >= 0)
      // break;
    }   
  }catch(Exception e){
    cerr << e.what() << endl;
  }

  cap.release();
  return 0;
}
