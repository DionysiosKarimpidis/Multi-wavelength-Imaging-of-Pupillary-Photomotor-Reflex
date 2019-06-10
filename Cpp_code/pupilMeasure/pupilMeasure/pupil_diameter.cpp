/**
* Program to detect pupil, based on
* http://www.codeproject.com/Articles/137623/Pupil-or-Eyeball-Detection-and-Extraction-by-C-fro
* with some improvements.

#include <C:\Users\Nionios\Desktop\openit\OpenCv2.4.13\opencv\build\include\opencv2\imgproc\imgproc.hpp>;
#include <C:\Users\Nionios\Desktop\openit\OpenCv2.4.13\opencv\build\include\opencv2\highgui\highgui.hpp>;

*/

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <iostream>
#include <string> 
#include <chrono>
#include <thread>

//extra includes to write in txt file
//#include "stdafx.h"
#include "math.h"
#include <Windows.h>
#include <fstream>
#include <string>


int getDiameter(const std::string& frame)
{
	
	int diameter;
	diameter = 0;

	// Load image
	cv::Mat src = cv::imread(frame);
	if (src.empty()) {
		std::cout << "frame file not found" << "\n";
		return -1;
	}

	cv::resize(src, src, cv::Size(640, 480));

	//cv::imshow("image", src);
	//cv::waitKey(0);

	// Invert the source image and convert to grayscale
	cv::Mat gray;
	cv::cvtColor(~src, gray, CV_BGR2GRAY);

	//cv::imshow("image", src);
	//cv::waitKey(0);

	// Convert to binary image by thresholding it
	cv::threshold(gray, gray, 200, 255, cv::THRESH_BINARY);
	
	// Find all contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(gray.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// Fill holes in each contour
	cv::drawContours(gray, contours, -1, CV_RGB(255, 255, 255), -1);

	//for (int i = contours.size()-1; i >=0 ; i--)
	for (int i = 0; i <contours.size() ; i++)
	{
		double area = cv::contourArea(contours[i]);
		cv::Rect rect = cv::boundingRect(contours[i]);
		int radius = rect.width / 2;
	


		// If contour is big enough and has round shape
		// Then it is the pupil
		if (area >= 30 &&
			std::abs(1 - ((double)rect.width / (double)rect.height)) <= 0.2 &&
			std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
		{
			
			cv::circle(src, cv::Point(rect.x + radius, rect.y + radius), radius, CV_RGB(255, 0, 0), 1.8);
			//cv::circle(src, cv::Point(rect.x , rect.y ), radius, CV_RGB(255, 0, 0), 2);
			diameter = 2 * radius;

		}
	}
	/*if (diameter < 100){
		cv::imshow("image", src);
		cv::waitKey(0);
	}*/
	cv::imshow("image", src);
	cv::waitKey(0);
	return diameter;
}

void SaveDiameter(int diam)
{
	std::ofstream diameterFile;
	diameterFile.open("diameter2.txt", std::ios::app | std::ios::ate);
	diameterFile << diam << "\n";
	diameterFile.close();

	diameterFile.seekp(0, std::ios::end); //to ensure the put pointer is at the end
	diameterFile << "test \n";
}

int main(int argc, char** argv)
{
	int d;
	for (int i = 1; i < 334; i++) {
		std::string image = "symeou2_left" + std::to_string(i) + ".jpg";
	//std::string image = "teo3.png";
		d = getDiameter(image);
		std::cout << "diameter of " + image +"  is: " << d << "\n";
		SaveDiameter(d);
	}
	
	//cv::imshow("image", src);
	system("pause");

	return 0;
}



