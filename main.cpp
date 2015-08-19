//Main Program
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include "Marker.h"
#include "GraphUtils.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <time.h>
//#include <Gdiplus.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sys\timeb.h>
#include <ctime>
#endif

//using namespace cv;
using namespace std;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
const int ACTUAL_WIDTH = 780;
const int ACTUAL_HEIGHT = 585;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 10 * 10;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed
}
string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}
void createTrackbars(){
	//create window for trackbars
	cv::namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	sprintf_s(TrackbarName, "V_MIN", V_MIN);
	sprintf_s(TrackbarName, "V_MAX", V_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	cv::createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	cv::createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	cv::createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	cv::createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	cv::createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	cv::createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);
}
void drawObject(vector <Marker> theYellow, cv::Mat &frame, int(&x)[16], int(&y)[16]){
	for (int i = 0; i < theYellow.size(); i++){
		if (i < 16){ 
			x[i] = theYellow.at(i).getXpos() * ACTUAL_WIDTH / FRAME_WIDTH;
			y[i] = theYellow.at(i).getYpos() * ACTUAL_HEIGHT / FRAME_HEIGHT;
		}
		cv::circle(frame, cv::Point(theYellow.at(i).getXpos(), theYellow.at(i).getYpos()), 10, cv::Scalar(255, 0, 0));
		cv::putText(frame, intToString(theYellow.at(i).getXpos() * ACTUAL_WIDTH / FRAME_WIDTH) + "," + intToString(theYellow.at(i).getYpos()* ACTUAL_HEIGHT / FRAME_HEIGHT), cv::Point(theYellow.at(i).getXpos(), theYellow.at(i).getYpos() + 20), cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0, 255, 0));
		//cv::putText(frame, theYellow.at(i).getType(), cv::Point(theYellow.at(i).getXpos(), theYellow.at(i).getYpos() - 30), 1, 2, theYellow.at(i).getColour());
	}
}
void morphOps(cv::Mat &thresh){
	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle
	cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}
void trackFilteredObject(Marker theYellow, cv::Mat threshold, cv::Mat HSV, cv::Mat &cameraFeed, int(&x)[16], int(&y)[16]){
	vector <Marker> yellows;
	cv::Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<cv::Point> > contours;
	vector<cv::Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0; 
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {
				cv::Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;
				//if the area is less than 10 px by 10px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA){
					Marker yellow;
					yellow.setXpos(moment.m10 / area);
					yellow.setYpos(moment.m01 / area);
					yellow.setType(theYellow.getType());
					yellow.setColour(theYellow.getColour());
					yellows.push_back(yellow);
					objectFound = true;
				}
				else objectFound = false;
			}
			//let user know you found an object
			if (objectFound == true){
				//draw object location on screen
				drawObject(yellows, cameraFeed, x, y);
			}
			else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", cv::Point(0, 50), 1, 2, cv::Scalar(0, 0, 255), 2);
		}
	}
}

int main(int argc, char* argv[]){
	//if we would like to calibrate our filter values, set to true.
	bool calibrationMode = false;
	//Matrix to store each frame of the webcam feed
	cv::Mat cameraFeed; 
	cv::Mat threshold;
	cv::Mat HSV;
	if (calibrationMode){
		//create slider bars for HSV filtering
		createTrackbars();
	}
	//video capture object to acquire webcam feed	
	//open capture object at location zero (default location for webcam)
	cv::VideoCapture cap("E:\\test_01.avi");

	// if not success, exit program
	if (!cap.isOpened()){
		cout << "Cannot open the video file" << endl;
		return -1;
	}
	//set height and width of capture frame
	cap.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	double fps = 20;
	int  x[16];
	int  y[16];
	int sec = 0;
	cout << "Frame per seconds : " << fps << endl;
	//create a window called "test_01"
	cv::namedWindow("test_08", CV_WINDOW_AUTOSIZE);		
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	bool second = true;
	ofstream myfile("test_08.xls");
	myfile.is_open();
	clock_t startTime = clock();
	while (1){
		//store image to matrix
		cap.read(cameraFeed);
		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed, HSV, cv::COLOR_BGR2HSV);
		Marker yellow("yellow");
		if (calibrationMode == true){
			//if in calibration mode, we track objects based on the HSV slider values. Filter “HSV Noise”
			cvtColor(cameraFeed, HSV, cv::COLOR_BGR2HSV);
			inRange(HSV, cv::Scalar(H_MIN, S_MIN, V_MIN), cv::Scalar(H_MAX, S_MAX, V_MAX), threshold);
			//inRange(HSV, yellow.getHSVmin(), yellow.getHSVmax(), threshold);
			morphOps(threshold);
			imshow(windowName2, threshold);
			trackFilteredObject(yellow, threshold, HSV, cameraFeed, x, y);
		}
		else{
			cvtColor(cameraFeed, HSV, cv::COLOR_BGR2HSV);
			inRange(HSV, yellow.getHSVmin(), yellow.getHSVmax(), threshold);
			morphOps(threshold);
			imshow(windowName2, threshold);
			trackFilteredObject(yellow, threshold, HSV, cameraFeed, x, y);
		}
		imshow(windowName, cameraFeed);
		cout << clock() << endl;
		if (clock() - startTime > 980){
			sec = sec + 1;
			for (int i = 0; i < 8; i++) myfile << '\n' << sec << '\t' << x[i] << '\t' << y[i] << '\t' << x[i + 8] << '\t' << y[i + 8];
			startTime = clock();
		}
		//wait for 'esc' key press for 30 ms. If 'esc' key is pressed, break loop
		if (cv::waitKey(30) == 27) {
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	//closes the textfile
	myfile.close();
	return 0;	}
