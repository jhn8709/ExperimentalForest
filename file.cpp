#define _CRT_SECURE_NO_WARNINGS	  /* disable compiler warnings for standard C functions like strcpy() */
#pragma warning(disable:4244)	  /* disable compiler warnings for possible loss of data */
#pragma warning(disable:4267)	  /* disable compiler warnings for possible loss of data */


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <math.h>
#include "globals.h"
#include <string.h>
#include <iostream>

/* OpenCV includes */
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

using namespace cv;
using namespace std;

bool			VideoLoaded{ FALSE };		/* 0=>no; 1=>yes (file exists and is opened in ffmpeg library) */
//bool			InterruptError{ FALSE };
unsigned char*	disp_image;					/* image from video to display */
int				DISPLAY_ROWS{ 0 };			/* size of video image */
int				DISPLAY_COLS{ 0 };			/* size of video image */
VideoCapture	capture;
int				TotalData;					/* total frames from the video file */
int				nFrames{150};
vector<int>		Xlist, Ylist, Xlist2, Ylist2, Xpoints, Ypoints;
vector<Mat>		images; // stores all the frames from the video
bool			imageloaded{ FALSE };

int ReadVideo(char* file_name)
{
	capture.open(file_name);
	if (!capture.isOpened()) 
		{
			//error in opening the video input
			cerr << "Unable to open file!" << endl;
			return 0;
		}

	VideoLoaded = TRUE;
	TotalData = (int)(capture.get(CAP_PROP_FRAME_COUNT));
	DISPLAY_ROWS = ((int)(capture.get(CAP_PROP_FRAME_HEIGHT)) < 720) ? (int)(capture.get(CAP_PROP_FRAME_HEIGHT)) : 720;
	DISPLAY_COLS = ((int)(capture.get(CAP_PROP_FRAME_WIDTH)) < 1280) ? (int)(capture.get(CAP_PROP_FRAME_WIDTH)) : 1280;
	
	AllocateStruct(TotalData);
	//disp_image = (unsigned char*)calloc(DISPLAY_ROWS * DISPLAY_COLS * 3, 1);

	return(1);
}

/*
	@brief Extracts the frame at FrameIndex and stores the image in disp_image.
*/
void ReadVideoFrame()
{
	static Mat img;
	capture.set(CAP_PROP_POS_FRAMES, FrameIndex);
	capture.read(img);
	ResizeFrame(&img);
	disp_image = img.data;
	return;
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
		/*if (lostP_inc > 0)
		{
			InterruptError = TRUE;
			break;
		}*/

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
		/*if (lostP_inc > 0)
		{
			InterruptError = TRUE;
			break;
		}*/

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

/*
	@brief Load the images from the video file and store them in the images vector.
*/
void LoadImageFromVideo() {
	Mat img;
	HDC hDC = GetDC(MainWnd);
	char text[100];
	//for (int i = 0; i < TotalData; i++) {  // Only for testing purposes
	for (int i = 0; i < 100; i++) {
		capture.set(CAP_PROP_POS_FRAMES, i);
		capture.read(img);
		ResizeFrame(&img);
		images.push_back(img);
		//sprintf(text, "Loading ... %0.2f / 100%%", (float)i /TotalData * 100);  // Only for testing purposes
		sprintf(text, "Loading ... %0.2f / 100%%", (float)i /100 * 100);
		TextOut(hDC, DISPLAY_COLS + 20, 250, (LPCSTR)text, strlen(text));
	}
	ReleaseDC(MainWnd, hDC);
	imageloaded = TRUE;
}

/*
	@brief Thread that loads the images from the video file.
*/
unsigned int __stdcall ImageLoadingThread(void* data) {
	LoadImageFromVideo();
	return 0;
}

/*
	@brief Starts the thread that loads the images from the video file.
*/
void StartImageLoadingThread() {
	/*HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ImageLoadingThread, NULL, 0, NULL);
	HDC hDC = GetDC(MainWnd);
	char text[100];
	if (hThread == nullptr) {
		cout << "Error creating thread" << endl;
	}
	else {
		sprintf(text, "Loading ... 0/100%%");
		TextOut(hDC, DISPLAY_COLS + 20, 250, (LPCSTR)text, strlen(text));
	}
	ReleaseDC(MainWnd, hDC);*/
	LoadImageFromVideo();
}

/*
	@brief Based on the ground truth data forms the trail on the image. disp_image stores the processed data.
*/
void applyMask() {
	static Mat img = images[FrameIndex];
	//cv::imshow("Trail", img);
	/*capture.set(CAP_PROP_POS_FRAMES, FrameIndex);
	capture.read(img);
	ResizeFrame(&img);*/

	/* Find the width of the trail for every labeled point
	   Draw Lines through
	   Apply mask over the area in the image and then apply the mask again wrt the second label
	*/
	fillROI(TRUE, FrameIndex);
	fillROI(FALSE, FrameIndex);
	img = createMask(TRUE, img, "green");  // Directly applies the mask to the image
	img = createMask(FALSE, img, "red");
	disp_image = img.data;
	//cv::imshow("Trail", img);
	return;
}

void fillROI(bool file, int frameIndex) {
	int max_width = 375;
	int max_y = 720;
	int min_y = HORIZON;	// Represents "Horizon"
	float slope = (float)max_width / (max_y - min_y);

	if (file) {
		int point_count = pointData[FrameIndex].point_count; 
		int width;

		Xlist.clear();
		Ylist.clear();
		for (int i = 0; i < (point_count-1); i++) {  
			drawLineMask(pointData[FrameIndex].x[i], pointData[FrameIndex].y[i], pointData[FrameIndex].x[i + 1], pointData[FrameIndex].y[i + 1]);

			// This section determines which pixels are part of the ROI(region of interest) from the labeled points
			for (int j = 0; j < Xpoints.size(); j++) {
				width = max_width - (max_y - Ypoints[j]) * slope;
				width = floor(width / 2);
				Xlist.push_back(Xpoints[j]);
				Ylist.push_back(Ypoints[j]);

				for (int k = 1; k < width; k++) {
					Xlist.push_back(Xpoints[j] + k);
					Ylist.push_back(Ypoints[j]);
					Xlist.push_back(Xpoints[j] - k);
					Ylist.push_back(Ypoints[j]);
				}

			}
		}
	}
	else {
		int point_count = pointData_2[FrameIndex].point_count; 
		int width;

		Xlist2.clear();
		Ylist2.clear();
		for (int i = 0; i < (point_count-1); i++) {  
			drawLineMask(pointData_2[FrameIndex].x[i], pointData_2[FrameIndex].y[i], pointData_2[FrameIndex].x[i+1], pointData_2[FrameIndex].y[i+1]);


			// This section determines which pixels are part of the ROI(region of interest) from the labeled points
			for (int j = 0; j < Xpoints.size(); j++) {
				width = max_width - (max_y - Ypoints[j]) * slope;
				width = floor(width / 2);
				Xlist2.push_back(Xpoints[j]);	
				Ylist2.push_back(Ypoints[j]);

				for (int k = 1; k < width; k++) {
					Xlist2.push_back(Xpoints[j] + k);
					Ylist2.push_back(Ypoints[j]);
					Xlist2.push_back(Xpoints[j] - k);
					Ylist2.push_back(Ypoints[j]);
				}

			}
		}
	}
}

void drawLineMask(int startX, int startY, int endX, int endY) {
	int dx = endX - startX;
	int dy = endY - startY;
	double Xinc, Yinc, X, Y;
	int steps;

	if (abs(dx) > abs(dy)) {
		steps = abs(dx);
	}
	else {
		steps = abs(dy);
	}

	Xinc = dx / (float)steps;
	Yinc = dy / (float)steps;
	X = startX;
	Y = startY;

	Xpoints.clear();
	Ypoints.clear();
	for (int i = 0; i < steps; i++) {
		Xpoints.push_back(round(X));
		Ypoints.push_back(round(Y));
		X += Xinc;
		Y += Yinc;
	}
}

cv::Mat createMask(bool first_second, cv::Mat frame, std::string color) {
	cv::Mat mask(frame.size(), CV_8U, cv::Scalar(0));
	std::vector<cv::Point> roi_corners;

	if (first_second) {
		if (Xlist.size() == 0 || Ylist.size() == 0) {
			return frame;
		}
		// Define the coordinates of the region of interest (roi_corners)
		for (size_t i = 0; i < Xlist.size(); ++i) {
			roi_corners.push_back(cv::Point(Xlist[i], Ylist[i]));
		}
	}
	else {
		if (Xlist2.size() == 0 || Ylist2.size() == 0) {
			return frame;
		}
		// Define the coordinates of the region of interest (roi_corners)
		for (size_t i = 0; i < Xlist.size(); ++i) {
			roi_corners.push_back(cv::Point(Xlist2[i], Ylist2[i]));
		}
	}
	
	std::vector<std::vector<cv::Point>> roi_polygons = { roi_corners };
	cv::fillPoly(mask, roi_polygons, cv::Scalar(255));

	cv::Scalar mask_color;
	if (color == "green") {
		mask_color = cv::Scalar(0, 255, 0);
	}
	else if (color == "blue") {
		mask_color = cv::Scalar(255, 0, 0);
	}
	else if (color == "red") {
		mask_color = cv::Scalar(0, 0, 255);
	}

	cv::Mat darkened_mask;
	cv::bitwise_and(mask, mask, darkened_mask, mask = mask);

	cv::Mat overlay(frame.size(), frame.type(), cv::Scalar(0));
	frame.copyTo(overlay, darkened_mask);
	
	cv::Mat color_mask; // mask where darkened_mask is not zero
	cv::compare(darkened_mask, 0, color_mask, cv::CMP_NE);

	overlay.setTo(mask_color, color_mask); // apply the color to the overlay
	
	cv::Mat masked_overlay;
	cv::bitwise_and(overlay, overlay, masked_overlay, mask = mask);

	cv::Mat output;
	cv::addWeighted(frame, 1.0, masked_overlay, 0.25, 0.5, output);

	return output;
}