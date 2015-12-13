// OpencvSpout.cpp: define el punto de entrada de la aplicación de consola.
//
#include "stdafx.h"
#include <iostream>
#include "Glew\glew.h"
#include "GL\freeglut.h"
#include "opencv2\opencv.hpp"
#include "Opencv2Opengl.h"

using namespace std;
using namespace cv;
#define USE_GLEW
int main(int argc, char **argv)
{
	

	
	cv::Mat gray = cv::Mat();
	Mat img;
	VideoCapture cap;
	Opencv2Spout conversor(argc, argv,640,480,false);
	char nombreReceiver[256];
	strcpy_s(nombreReceiver, 256, "opencvReceiver");
	bool receiverExists = conversor.initReceiver(nombreReceiver);
	if (receiverExists)
		cout << "Receiver " << nombreReceiver << " detected " << endl;
	else
	{
		cout << "No receiver detected, tacking Camera in slot 0." << endl;
		cap = VideoCapture(0); // open the default camera
		if (!cap.isOpened())  // check if we succeeded
			return -1;
	}
	if (true)
		for (;;)
		{
			if (receiverExists)
				img = conversor.receiveTexture();
			else
				cap >> img; // get a new frame from camera
			if (cvWaitKey(30)>0)
				break;
			else
			{
				conversor.draw(img,true);
			}

		}

	return 0;
}

