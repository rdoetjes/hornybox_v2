#include "vision.hpp"

void vision::drawCascadeMatches(Mat *frame, vector<Rect> n, double scale){
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


void vision::setupCam(VideoCapture *cap, int deviceID, int apiID){
  cap->open(deviceID + apiID);
  cap->set(CAP_PROP_BUFFERSIZE, 1);
  cap->set(CAP_PROP_FPS, 3);
}

int vision::getNrOfCascadeMatches(Mat *frame, vector<CascadeClassifier> *cascades){
  Mat process;
  vector<Rect> n;

  resize(*frame, process, Size(), 0.5, 0.5, INTER_LINEAR ); 
  cvtColor(process, process, COLOR_BGR2GRAY);
  equalizeHist( process, process );
 
  for(auto cascade : *cascades)
  {
    cascade.detectMultiScale(process, n, 1.1, 3, 0|CASCADE_SCALE_IMAGE, Size(30, 30) ); 
    if (n.size() > 0) return n.size(); //this speeds up the process
  } 

  return 0;
}


void vision::snapPictures(VideoCapture *cap, string picsFolder, int numberOfPics){
  Mat snapShot;
  for (int i=0; i< numberOfPics; ++i){
    cap->read(snapShot);
    imwrite(picsFolder + "/" + to_string( time(nullptr) ) + "_" + to_string(i) + ".jpg", snapShot);
  }
}
