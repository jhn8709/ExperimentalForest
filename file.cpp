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

bool			VideoLoaded{ FALSE };		/* 0=>no; 1=>yes (file exists and is opened in ffmpeg library) */
bool			InterruptError{ FALSE };
unsigned char*	disp_image;					/* image from video to display */
int				DISPLAY_ROWS{ 0 };			/* size of video image */
int				DISPLAY_COLS{ 0 };			/* size of video image */
VideoCapture	capture;
int				TotalData;					/* total frames from the video file */
int				nFrames{150};

//Mat img;

void ResizeFrame(Mat* img);



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
	DISPLAY_ROWS = ((int)(capture.get(CAP_PROP_FRAME_HEIGHT)) < 720) ? (int)(capture.get(CAP_PROP_FRAME_HEIGHT)) : 720;
	DISPLAY_COLS = ((int)(capture.get(CAP_PROP_FRAME_WIDTH)) < 1280) ? (int)(capture.get(CAP_PROP_FRAME_WIDTH)) : 1280;
	
	AllocateStruct(TotalData);
	//disp_image = (unsigned char*)calloc(DISPLAY_ROWS * DISPLAY_COLS * 3, 1);

return(1);
}


int ReadVideoFrame()
{
	static Mat img;
	capture.set(CAP_PROP_POS_FRAMES, FrameIndex);
	capture.read(img);
	ResizeFrame(&img);
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
	int lostP_inc = 0; // amount of points that need manual adjustment
	uint i;
	HDC hDC;
	char text[100];
	int KLTFrameIndex{ FrameIndex };

	/* new addition 6/25/2023 */
	int Ymax = 695; // Ymin is set to MaxValue-KLTWindowSize = 720-15 = 705
	int Xmax;
	float slope, y_intercept;

	/* new addition 9/7/2023 */
	int XBoundHigh = 1260; // X boundary is set to 1280-20 = 1260
	int XBoundLow = 20; // X boundary is set to 0+20 = 20
	int YBounded, lastIndex, secondLastIndex;

	if (pointData[FrameIndex].point_count < 1)
	{
		return;
	}

	Mat old_frame, old_gray;
	//capture >> old_frame;
	capture.set(CAP_PROP_POS_FRAMES, KLTFrameIndex);
	capture.read(old_frame);
	ResizeFrame(&old_frame);


	KLTFrameIndex++;
	cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
	vector<Point2f> p0(pointData[FrameIndex].point_count), p1(pointData[FrameIndex].point_count);
	for (i = 0; i < pointData[FrameIndex].point_count; i++)
	{
		p0[i] = Point2f(pointData[FrameIndex].x[i], pointData[FrameIndex].y[i]);
	}

	hDC = GetDC(MainWnd);
	while (iteration <= nFrames)
	{
		/* Track progress in GUI */
		sprintf(text, "Interpolating Frame %d/%d   ", KLTFrameIndex-FrameIndex, nFrames);
		TextOut(hDC, DISPLAY_COLS + 20, 270, (LPCSTR)text, strlen(text));

		/* clear list of values */
		//fill(pointData[KLTFrameIndex].forward_interpX.begin(), pointData[KLTFrameIndex].forward_interpX.end(), -1);

		/* forward interpolation algorithm */
		Mat frame, frame_gray;
		capture.set(CAP_PROP_POS_FRAMES, KLTFrameIndex);
		capture.read(frame);
		//ResizeFrame(&frame);
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
				continue;
			}
			if ( (p0[i].y > Ymax-10) && (i > 0) )
			{
				lostP_inc++;
			}
			
		}
		if (lostP_inc > 0)
		{
			InterruptError = TRUE;
			break;
		}

		// Now update the previous frame and previous points
		old_gray = frame_gray.clone();
		p0 = good_new;

		// Correct the first point if needed
		if ( (p0[0].y > Ymax) && (p0.size() > 1) )
		{
			if (p0[0].x == p0[1].x)
			{
				Xmax = p0[0].x;
			}
			else
			{
				slope = (p0[1].y - p0[0].y) / (p0[1].x - p0[0].x);
				y_intercept = p0[1].y - slope * p0[1].x;
				Xmax = round( ((Ymax - y_intercept) / slope) );
			}
			p0[0].x = Xmax;
			p0[0].y = Ymax;
		}

		// Correct the last point if needed
		lastIndex = p0.size() - 1;
		secondLastIndex = p0.size() - 2;
		if (p0[lastIndex].x > XBoundHigh)
		{
			slope = (p0[lastIndex].y - p0[secondLastIndex].y) / (p0[lastIndex].x - p0[secondLastIndex].x);
			y_intercept = p0[secondLastIndex].y - slope * p0[secondLastIndex].x;
			YBounded = round(slope * XBoundHigh + y_intercept);

			p0[lastIndex].x = XBoundHigh;
			p0[lastIndex].y = YBounded;
		}
		else if (p0[lastIndex].x < XBoundLow)
		{
			slope = (p0[lastIndex].y - p0[secondLastIndex].y) / (p0[lastIndex].x - p0[secondLastIndex].x);
			y_intercept = p0[secondLastIndex].y - slope * p0[secondLastIndex].x;
			YBounded = round(slope * XBoundLow + y_intercept);

			p0[lastIndex].x = XBoundLow;
			p0[lastIndex].y = YBounded;
		}


		int xvals[10], yvals[10]; /* new addition 8/31/2023 */
		for (i = 0; i < p0.size(); i++)
		{
			pointData[KLTFrameIndex].x[i] = round(p0[i].x);
			pointData[KLTFrameIndex].y[i] = round(p0[i].y);
			 
			/* algorithm that includes backwards interpolation might be needed here */

			xvals[i] = round(p0[i].x);
			yvals[i] = round(p0[i].y);

		}
		pointData[KLTFrameIndex].point_count = (int)p0.size();
		
		//storeGTInterpolationData(pointData[KLTFrameIndex].forward_interpX, xvals, yvals, KLTFrameIndex);
		iteration++;
		KLTFrameIndex++;
	}
	ReleaseDC(MainWnd, hDC);

}

/* added 8/14/2023 */
void InterpolateFramesBackwards()
{
	static int nFeatures = 10; /* 10 is the max amount of points allowed to place */

	int iteration = 1;
	double dx = 0;

	/* new addition 5/8/2023 */
	int lostP_inc = 0; // amount of points that need manual adjustment
	uint i;
	HDC hDC;
	char text[100];
	int KLTFrameIndex{ FrameIndex-1 };

	/* new addition 6/25/2023 */
	int Ymax = 700; // Ymin is set to MaxValue-KLTWindowSize = 720-15 = 705
	int Xmax;
	float slope, y_intercept;

	/* new addition 8/14/2023 */
	int iterationsNeeded = 0;
	float slope_inc = 0;
	float backwards_weight = 0;
	float forwards_weight = 0;

	/* new addition 9/7/2023 */
	int XBoundHigh = 1260; // X boundary is set to 1280-20 = 1260
	int XBoundLow = 20; // X boundary is set to 0+20 = 20
	int YBounded, lastIndex, secondLastIndex;
	bool checkDeletedPoints = false;
	//float y_intercept, slope;

	/* Do not interpolate if: 
		1) there are no points
		2) user is working on first frame
		3) if user has changed the amount of points (this has changed now) */
	if ( (pointData[FrameIndex].point_count < 1) || (FrameIndex == 0) || disableBackInterp )
	{
		return;
	}

	Mat old_frame, old_gray;
	//capture >> old_frame;
	capture.set(CAP_PROP_POS_FRAMES, KLTFrameIndex);
	capture.read(old_frame);
	ResizeFrame(&old_frame);


	//KLTFrameIndex++;
	cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
	vector<Point2f> p0(pointData[FrameIndex].point_count), p1(pointData[FrameIndex].point_count);
	for (i = 0; i < pointData[FrameIndex].point_count; i++)
	{
		p0[i] = Point2f(pointData[FrameIndex].x[i], pointData[FrameIndex].y[i]);
	}

	while (pointData[KLTFrameIndex].manual == false) 
	{
		iterationsNeeded++;
		KLTFrameIndex--;
	}
	slope_inc = 1.0 / (float(iterationsNeeded) + 1.0);
	KLTFrameIndex = FrameIndex - 1;
	backwards_weight = 1 - slope_inc;
	forwards_weight = 1 - backwards_weight;


	hDC = GetDC(MainWnd);
	// backwards interpolation
	while (iteration <= iterationsNeeded)
	{
		sprintf(text, "Interpolating Frame %d/%d   ", iteration, iterationsNeeded);
		TextOut(hDC, DISPLAY_COLS + 20, 270, (LPCSTR)text, strlen(text));

		/* clear list of values */
		//fill(pointData[KLTFrameIndex].backward_interpX.begin(), pointData[KLTFrameIndex].backward_interpX.end(), -1);

		Mat frame, frame_gray;
		capture.set(CAP_PROP_POS_FRAMES, KLTFrameIndex);
		capture.read(frame);
		ResizeFrame(&frame);
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
				continue;
			}
			if ((p0[i].y > Ymax - 10) && (i > 0))
			{
				lostP_inc++;
			}

		}
		if (lostP_inc > 0)
		{
			InterruptError = TRUE;
			break;
		}

		// Now update the previous frame and previous points
		old_gray = frame_gray.clone();
		p0 = good_new;

		lastIndex = p0.size() - 1;
		secondLastIndex = p0.size() - 2;
		// Correct the last point if needed
		if (p0[lastIndex].x > XBoundHigh)
		{
			slope = (p0[lastIndex].y - p0[secondLastIndex].y) / (p0[lastIndex].x - p0[secondLastIndex].x);
			y_intercept = p0[secondLastIndex].y - slope * p0[secondLastIndex].x;
			YBounded = round( slope * XBoundHigh + y_intercept );

			p0[lastIndex].x = XBoundHigh;
			p0[lastIndex].y = YBounded;
		}
		else if (p0[lastIndex].x < XBoundLow)
		{
			slope = (p0[lastIndex].y - p0[secondLastIndex].y) / (p0[lastIndex].x - p0[secondLastIndex].x);
			y_intercept = p0[secondLastIndex].y - slope * p0[secondLastIndex].x;
			YBounded = round( slope * XBoundLow + y_intercept );

			p0[lastIndex].x = XBoundLow;
			p0[lastIndex].y = YBounded;
		}


		// Fix points that drop off from KLT Tracking and record data
		int xvals[10], yvals[10], point_count; /* new addition 8/31/2023 */
		int j = 0; /* added 9/16/23 */
		point_count = pointData[KLTFrameIndex].point_count;
		pointData[KLTFrameIndex].point_count = 0;
		checkDeletedPoints = (point_count > p0.size()) ? true : false;
		for (i = 0; i < p0.size(); i++)
		{

			auto it = std::find(deletedPointsIndexes.begin(), deletedPointsIndexes.end(), i);
			if (it != deletedPointsIndexes.end() && checkDeletedPoints) 
			{
				/* Is in the vector */
				j += 1;
				pointData[KLTFrameIndex].point_count += 1;
			}

			/* Adds new display points if needed else use weighted average to determine display points */
			if ( (i >= point_count) && (p0[i].y >= HORIZON) ) /* Do not add new point if the point is greater than the horizon */
			{
				pointData[KLTFrameIndex].x[j] = round(p0[i].x);
				pointData[KLTFrameIndex].y[j] = round(p0[i].y);
			}
			else if (i >= point_count) {
				pointData[KLTFrameIndex].point_count = -1; /* have the last point not count */
			}
			else
			{
				if (i == 0)
				{
					pointData[KLTFrameIndex].x[j] = round(p0[i].x * backwards_weight + pointData[KLTFrameIndex].x[j] * forwards_weight);
				}
				else
				{
					pointData[KLTFrameIndex].x[j] = round(p0[i].x * backwards_weight + pointData[KLTFrameIndex].x[j] * forwards_weight);
					pointData[KLTFrameIndex].y[j] = round(p0[i].y * backwards_weight + pointData[KLTFrameIndex].y[j] * forwards_weight);
				}
			}
			j += 1;
			xvals[i] = round(p0[i].x);
			yvals[i] = round(p0[i].y);
		}
		pointData[KLTFrameIndex].point_count += (int)p0.size();

		//storeGTInterpolationData(pointData[KLTFrameIndex].backward_interpX, xvals, yvals, KLTFrameIndex);
		iteration++;
		KLTFrameIndex--;
		backwards_weight -= slope_inc;
		forwards_weight += slope_inc;
	}
	ReleaseDC(MainWnd, hDC);
}

void ResizeFrame(Mat *img)
{
	if ((int)(capture.get(CAP_PROP_FRAME_HEIGHT) > 720))
	{
		resize(*img, *img, Size(1280, 720), INTER_LINEAR);
	}
}
