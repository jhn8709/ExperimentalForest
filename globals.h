
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
	void		DataToCSV();
	void		LoadCSVData(char *file_name);


#define TIMER_SECOND		1								/* ID of timer used during playback */

#include <opencv2/video.hpp>

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

/* Flags */
extern bool			  VideoLoaded;			/* 0=>no; 1=>yes (file exists and is opened in ffmpeg library) */
extern bool			  changeFFSpeed;
extern bool			  changeInterpolationLength;

//extern char		  GTFilename[320];		/* file that contains the ground truth steps */

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

struct GroundTruth
{
	int x[10];
	int y[10];
	int point_count{0};
};
extern GroundTruth *pointData;

