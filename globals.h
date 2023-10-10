#include <opencv2/video.hpp>
#include <vector>
#include <iostream>

using namespace std;
/* function prototypes */
void		PaintImage();
void		UpdateDisplay();
int		    ReadVideo(char* file_name);
void        AllocateStruct(int frame_count);
void		ModifyPoint(int xmouse, int ymouse, int xchange, int ychange);
int			ReadVideoFrame();
//void		CloseVideoFile();
//int			LoadGT();
void		InitializeDataVariables();
LRESULT		CALLBACK ChangeFFSpeed(HWND, UINT, WPARAM, LPARAM); /* change FF speed */
void		DrawPoint(int xval, int yval, int mode);
void		DrawLine(int startX, int startY, int endX, int endY, int mode);
void		DeletePoint(int x_change, int y_change);
void		LoadNextFrame();
void		InterpolateFrames();
void		InterpolateFramesBackwards();
//void		DataToCSV();
void		LoadCSVData(char* file_name);
void		UpdateMode(int mode, int flag);
void		storeGTData(vector<int>& xVector);
void		FillGT(int startX, int startY, int endX, int endY, vector<int>& xVector);
void		storeGTInterpolationData(vector<int>& xVector, int* xvals, int* yvals, int frameIndex);
void		saveDeletedPointsInit();
void		saveDeletedPoint(int index);
void		removeDeletedPoint();
//void		ResizeFrame(Mat* img);
void		applyMask();
void		fillROI(bool, int);
void		mask(Mat, bool, const char*);
void		drawLine(int, int, int, int);



#define TIMER_SECOND		1								/* ID of timer used during playback */

		/* global variables */
//HINSTANCE	  hInst;				/* pointer to program instance (need for dialog boxes) */
extern HWND		  MainWnd;				/* main window */
extern char		      DataFilename[320];	/* file that contains the accelerometer data (synchronized and collated at 15 Hz) */
//extern char		      VideoFilename[320];	/* file that contains the video */
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
			  

/* Flags */
extern bool			  VideoLoaded;			/* 0=>no; 1=>yes (file exists and is opened in ffmpeg library) */
extern bool			  changeFFSpeed;
extern bool			  changeInterpolationLength;
extern bool			  saveIndicator;
extern bool			  InterruptError;
extern bool			  disableBackInterp;
extern bool			  compareReady;


/* Newer additions 4/2/2023 */
#define POINT_SIZE		7
#define SQR(x) ((x)*(x))	/* macro for square */

//unsigned char *labeled_images;		/* saves disp_image after the frame has been drawn on added 4/7/2023 */

//extern int			 xmouse, ymouse;       /* position of the mouse in x and y coordinates */
//extern int			 x_change, y_change;   /* distance between an established point and where the user has clicked */
extern int           selected_point;		/* indices of the point closest to where the user has clicked */
//extern int			 total_points;			/* total number of points that make up the line drawn */
//extern int           px[2400][10], py[2400][10];       /* arrays to hold x and y coordinates of the points */
//extern int		     PointTotals[2400];

/* int x -> x coordinates for points on a frame up to 10 points 
   int y -> y coordinates for points on a frame up to 10 points
   int point_count -> number of points placed on a frame
   bool manual -> indicates whether a frame was worked on manually or automatically (false = automatic) */
//struct GroundTruth
//{
//	int x[10];
//	int y[10];
//	int point_count{0};
//	bool manual = false;
//};

// added 8/29/23
/* The bottom of the image is 720. We only need to create a path from 720 up to the horizon.
   Each frame will store values from the bottom to the horizon. For this program, we are only
   using 1280x720 video so each path will be at maximum 720-horizon pixels long. */
#define HORIZON 275 

struct GroundTruth
{
	/* The y-values are not being tracked in the ground truth. Since we will have record 
	   data at each possible y-value (where -1 indicates no data), each index in the vector will
	   correspond to a y-value.
	   e.g. human_x(445) corresponds to y-value -> 720
	   human_x(0) corresponds to y-value -> horizon (set to 275) */
	//int human_x[420];
	//int forward_interpX[420];
	//int backwards_interpX[420];
	//int final_x[420];

	int x[10]; /* Holds x-values for display points (up to 10 points) */
	int y[10]; /* Holds y-values for display points */
	int point_count{ 0 }; /* Stores the amount of display points */
	bool manual = false; /* Records whether the frame was adjusted manually or not */

	//vector<int> human_x;
	//vector<int> forward_interpX;
	//vector<int> backward_interpX;
	//vector<int> final_x;
	//
	//GroundTruth() // Constructor
	//{
	//	human_x = vector<int>(445, -1); /* Holds values that are determined by the human labeler */
	//	forward_interpX = vector<int>(445, -1); /* Holds values determined from forwards interpolation */
	//	backward_interpX = vector<int>(445, -1); /* Holds values determined from backwards interpolation */
	//	final_x = vector<int>(445, -1); /* Holds values to be sent to ground truth file */
	//}
};


extern GroundTruth *pointData; // Extending global variable pointData defined on display.cpp

