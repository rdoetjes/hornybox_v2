#ifndef VISION_H
#define VISION_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <iomanip>
#include <string.h> 
#include <time.h>
#include <vector>

using namespace cv;
using namespace std;

class vision{
  public:
	static void drawCascadeMatches(Mat *frame, vector<Rect> n, double scale);

	static void setupCam(VideoCapture *cap, int deviceID, int apiID);

	static int getNrOfCascadeMatches(Mat *frame, vector<CascadeClassifier> *cascades);

	static void snapPictures(VideoCapture *cap, string picsFolder, int numberOfPics);
};
#endif
