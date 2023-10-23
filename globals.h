#ifndef GLOBALS_H
#define GLOABALS_H

#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

/* function prototypes */
void		PaintImage();
void		UpdateDisplay();
int		    ReadVideo(char* file_name);
void        AllocateStruct(int frame_count);
void		ModifyPoint(int xmouse, int ymouse, int xchange, int ychange);
void		ReadVideoFrame();
void		InitializeDataVariables();
LRESULT		CALLBACK ChangeFFSpeed(HWND, UINT, WPARAM, LPARAM); /* change FF speed */
void		DrawPoint(int xval, int yval, int mode);
void		DrawLine(int startX, int startY, int endX, int endY, int mode);
void		DeletePoint(int x_change, int y_change);
void		LoadNextFrame();
void		InterpolateFrames();
void		InterpolateFramesBackwards();
void		DataToCSV();
void		LoadCSVData(char* file_name);
void		UpdateMode(int mode, int flag);
void		storeGTData(vector<int>& xVector);
void		FillGT(int startX, int startY, int endX, int endY, vector<int>& xVector);
void		storeGTInterpolationData(vector<int>& xVector, int* xvals, int* yvals, int frameIndex);
void		saveDeletedPointsInit();
void		saveDeletedPoint(int index);
void		removeDeletedPoint();
void		ResizeFrame(cv::Mat* img);
void		applyMask();
void		fillROI(bool, int);
cv::Mat		createMask(bool first_second, cv::Mat frame, std::string color = "green");
void		drawLineMask(int, int, int, int);

/* global variables */
//HINSTANCE	  hInst;				/* pointer to program instance (need for dialog boxes) */
extern HWND			  MainWnd;				/* main window */
extern char		      DataFilename[320];	/* file name of the user selected file */
extern char		      CurrentPath[320];		/* path where files were last read/saved */
extern int			  VideoSyncOffset;		/* milliseconds between data[0] and first video frame */
extern unsigned char *disp_image;			/* image from video to display */
extern int			  DISPLAY_ROWS;			/* size of video image */
extern int			  DISPLAY_COLS;			/* size of video image */
//extern char		      NameList[999][320];	/* master list of data filenames, lets user scroll through them */
extern int			  TotalFilenames;		/* 0=>no master list available; otherwise the total count */
extern int			  FrameIndex;			/* index of data currently being displayed (units are samples, 1/30 Hz) */
extern int			  PlayJump;				/* amount of samples to jump during each timer */
extern int			  FFspeed;				/* user-definable amount of frames to skip per iteration during fast forward */
extern int			  PlayCountdown;		/* used to play brief video sequences, halting when it counts down to zero */
extern int			  TotalData;			/* total #samples in the accelerometer data file */
extern int			  nFrames;
//extern int			  *deletedPointsIndexes;
extern vector<int>    deletedPointsIndexes;
extern int			  deletedPointsCount;
extern int			  selected_point;		/* indices of the point closest to where the user has clicked */			  

/* Flags */
extern bool			  VideoLoaded;			/* 0=>no; 1=>yes (file exists and is opened in ffmpeg library) */
extern bool			  changeFFSpeed;
extern bool			  changeInterpolationLength;
extern bool			  saveIndicator;
extern bool			  InterruptError;
extern bool			  disableBackInterp;
extern int			  compareReady;			// Flag that controls the routine for comparing labels from two different labelers

constexpr int TIMER_SECOND = 1;				/* ID of timer used during playback */
constexpr int POINT_SIZE = 7;				// Radius size for labeled points
constexpr int HORIZON = 275;				// Project variable

#define SQR(x)				((x)*(x))		// macro for square

/*
	Strcuture to hold ground truth data.

	int x -> x coordinates for points on a frame up to 10 points 
	int y -> y coordinates for points on a frame up to 10 points
	int point_count -> number of points placed on a frame
	bool manual -> indicates whether a frame was worked on manually or automatically (false = automatic)
*/
struct GroundTruth
{
	int x[10];								/* Holds x-values for display points (up to 10 points) */
	int y[10];								/* Holds y-values for display points */
	int point_count{ 0 };					/* Stores the amount of display points */
	bool manual = false;					/* Records whether the frame was adjusted manually or not */
};

extern GroundTruth* pointData;				// Extending global variable pointData defined on display.cpp
extern GroundTruth* pointData_2;

#endif // !GLOBALS_H