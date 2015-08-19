//Marker.h
#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
using namespace std;
//using namespace cv;

class Marker
{
public:
	Marker(void);
	~Marker(void);

	Marker(string name);

	int getXpos();
	void setXpos(int x);

	int getYpos();
	void setYpos(int Y);

	cv::Scalar getHSVmin();
	cv::Scalar getHSVmax();

	void setHSVmin(cv::Scalar min);
	void setHSVmax(cv::Scalar max);

	string getType(){ return type; }
	void setType(string t){ type = t; }

	cv::Scalar getColour(){
		return Colour;
	}
	void setColour(cv::Scalar c){
		Colour = c;
	}

private:
	
	int xPos, yPos;
	string type;
	cv::Scalar HSVmin, HSVmax;
	cv::Scalar Colour;
};
