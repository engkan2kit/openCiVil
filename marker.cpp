//Marker.cpp
//This is an include file of the main program.

#include "marker.h"

Marker::Marker(){
}

Marker::Marker(string name){
	setType(name);
	if (name == "yellow"){
		setHSVmin(cv::Scalar(25, 50, 131));
		setHSVmax(cv::Scalar(40, 213, 221));
		setColour(cv::Scalar(230, 245, 50));
	}
}

Marker::~Marker(){
}

int Marker::getXpos(){
	return Marker::xPos;
}

void Marker::setXpos(int x){
	Marker::xPos = x;
	xPos = x;
}

int Marker::getYpos(){
	return Marker::yPos;
}
void Marker::setYpos(int y){
	Marker::yPos = y;
	yPos = y;
}

cv::Scalar Marker::getHSVmin(){
	return Marker::HSVmin;
}

cv::Scalar Marker::getHSVmax(){
	return Marker::HSVmax;
}

void Marker::setHSVmin(cv::Scalar min){
	Marker::HSVmin = min;
}

void Marker::setHSVmax(cv::Scalar max){
	Marker::HSVmax = max;
}
