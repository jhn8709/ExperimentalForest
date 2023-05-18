#define _CRT_SECURE_NO_WARNINGS	  /* disable compiler warnings for standard C functions like strcpy() */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <math.h>
#include "globals.h"
#include <string.h>

/* OpenCV includes */
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
using namespace cv;
using namespace std;

bool VideoLoaded{ FALSE };			/* 0=>no; 1=>yes (file exists and is opened in ffmpeg library) */
unsigned char* disp_image;			/* image from video to display */
int			  DISPLAY_ROWS{ 0 };			/* size of video image */
int			  DISPLAY_COLS{ 0 };			/* size of video image */
VideoCapture  capture;
int			  TotalData;			/* total #samples in the accelerometer data file */
int			  nFrames{150};
//Mat img;



int ReadVideo(char* file_name)
{

	capture.open(file_name);

	TotalData = (int)(capture.get(CAP_PROP_FRAME_COUNT));
	if (!capture.isOpened()) 
	{
		//error in opening the video input
		cerr << "Unable to open file!" << endl;
		return 0;
	}
	VideoLoaded = TRUE;
	DISPLAY_ROWS = (int)(capture.get(CAP_PROP_FRAME_HEIGHT));
	DISPLAY_COLS = (int)(capture.get(CAP_PROP_FRAME_WIDTH));
	AllocateStruct(TotalData);
	//disp_image = (unsigned char*)calloc(DISPLAY_ROWS * DISPLAY_COLS * 3, 1);

return(1);
}


int ReadVideoFrame()
{
	static Mat img;
	capture.set(CAP_PROP_POS_FRAMES, FrameIndex);
	capture.read(img);
	disp_image = img.data;
	
	//strcpy((char*)disp_image, (const char*)img.data);
	//imshow("Frame", img);
	//waitKey(20);

	return(1);

}

void InterpolateFrames() /* Repeat set points to a certain amount of frames. Later this section should have an interpolation algorithm. */
{
	static int nFeatures = 10; /* 10 is the max amount of points allowed to place */
	//static int nFrames = 150;

	int iteration = 1;
	//int X, Y;
	double dx = 0;

	/* new addition 5/8/2023 */
	int lostP_inc = 0;
	uint i;
	HDC hDC;
	char text[100];
	int KLTFrameIndex{ FrameIndex };

	if (pointData[FrameIndex].point_count < 1)
	{
		return;
	}

	Mat old_frame, old_gray;
	//capture >> old_frame;
	capture.set(CAP_PROP_POS_FRAMES, KLTFrameIndex);
	capture.read(old_frame);
	KLTFrameIndex++;
	cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
	vector<Point2f> p0(pointData[FrameIndex].point_count), p1(pointData[FrameIndex].point_count);
	for (i = 0; i < pointData[FrameIndex].point_count; i++)
	{
		p0[i] = Point2f(pointData[FrameIndex].x[i], pointData[FrameIndex].y[i]);
	}

	// forward interpolation
	while (iteration <= nFrames)
	{
		hDC = GetDC(MainWnd);
		sprintf(text, "Interpolating Frame %d/%d  ", KLTFrameIndex-FrameIndex, nFrames);
		TextOut(hDC, DISPLAY_COLS + 20, 250, (LPCSTR)text, strlen(text));
		ReleaseDC(MainWnd, hDC);

		Mat frame, frame_gray;
		capture.set(CAP_PROP_POS_FRAMES, KLTFrameIndex);
		capture.read(frame);
		if (frame.empty())
			break;
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

		// calculate optical flow
		vector<uchar> status;
		vector<float> err;
		TermCriteria criteria = TermCriteria((TermCriteria::COUNT)+(TermCriteria::EPS), 10, 0.03);
		calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err, Size(15, 15), 2, criteria);

		vector<Point2f> good_new;
		for (i = 0; i < p0.size(); i++)
		{
			// Select good points
			if (status[i] == 1) {
				good_new.push_back(p1[i]);

			}
			else
			{
				lostP_inc++;
			}
		}
		if (lostP_inc > 1)
		{
			DeletePoint(pointData[KLTFrameIndex-1].x[0], pointData[KLTFrameIndex-1].x[0]);
			break;
		}

		//for (i = 0; i < PointTotals[FrameIndex]; i++)
		//{
		//	if (0 /* include code to determine if KLT tracking is still working properly */)
		//	{
		//		//X = fl->feature[i]->x;
		//		//Y = fl->feature[i]->y;
		//		px[FrameIndex + iteration][i] = X;
		//		py[FrameIndex + iteration][i] = Y;

		//	}
		//	else
		//	{
		//		lost_point = i;
		//		lostP_inc++;
		//		if (lostP_inc > 1)
		//		{
		//			break;
		//		}
		//	}
		//}
		//if (lostP_inc > 0)
		//{
		//	if (lostP_inc > 1)
		//	{
		//		break;
		//	}
		//	dx = px[FrameIndex + iteration][lost_point + 1] - px[FrameIndex + iteration - 1][lost_point + 1];
		//	dy = py[FrameIndex + iteration][lost_point + 1] - py[FrameIndex + iteration - 1][lost_point + 1] + 2;
		//	px[FrameIndex + iteration][lost_point] = px[FrameIndex + iteration - 1][lost_point] + (int)dx;
		//	py[FrameIndex + iteration][lost_point] = py[FrameIndex + iteration - 1][lost_point] + (int)dy;
		//	if (py[FrameIndex + iteration][lost_point] > DISPLAY_ROWS)
		//	{
		//		py[FrameIndex + iteration][lost_point] = DISPLAY_ROWS;
		//	}
		//}
		//dx = dy = lostP_inc = 0;
		//PointTotals[FrameIndex + iteration] = PointTotals[FrameIndex];

		// Now update the previous frame and previous points
		old_gray = frame_gray.clone();
		p0 = good_new;


		pointData[KLTFrameIndex].point_count = 0;
		for (i = 0; i < p0.size(); i++)
		{
			if ( (lostP_inc == 1) && (i == 0) )
			{
				dx = p0[i].x - pointData[KLTFrameIndex-1].x[1];
				pointData[KLTFrameIndex].x[0] = round(pointData[KLTFrameIndex-1].x[0]+dx);
				pointData[KLTFrameIndex].y[0] = DISPLAY_ROWS;
				pointData[KLTFrameIndex].point_count++;
			}
			if (lostP_inc == 1)
			{
				pointData[KLTFrameIndex].x[i+1] = round(p0[i].x);
				pointData[KLTFrameIndex].y[i+1] = round(p0[i].y);
			}
			else
			{
				pointData[KLTFrameIndex].x[i] = round(p0[i].x);
				pointData[KLTFrameIndex].y[i] = round(p0[i].y);
			}
		}
		pointData[KLTFrameIndex].point_count += (int)p0.size();

		iteration++;
		KLTFrameIndex++;
	}
}
