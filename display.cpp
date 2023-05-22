
#define _CRT_SECURE_NO_WARNINGS	  /* disable compiler warnings for standard C functions like strcpy() */

#define SQR(x)	((x)*(x))

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <winuser.h>

#include "globals.h"
#include "resource.h"

//#include <opencv2/video.hpp>


int          selected_point;		/* indices of the point closest to where the user has clicked */

GroundTruth *pointData;

void AllocateStruct(int frame_count)
{
	pointData = (GroundTruth*)calloc(frame_count, sizeof(GroundTruth));
}

void PaintImage()

{
PAINTSTRUCT			Painter;
HDC					hDC;
BITMAPINFOHEADER	bm_info_header;
BITMAPINFO			bm_info;
//RECT				rect,fillrect;
char				text[320];


//if (VideoLoaded  &&  disp_image != NULL)
if (VideoLoaded)
  {
  // changed from 15 to 30 since our video is 30fps
  //ReadVideoFrame(TimeIndex*1000/30+VideoSyncOffset,disp_image);	/* *1000/15 converts 15Hz data to milliseconds */

  ReadVideoFrame();

  
  BeginPaint(MainWnd,&Painter);
  hDC=GetDC(MainWnd);
  bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
  bm_info_header.biWidth=DISPLAY_COLS;
  bm_info_header.biHeight=-DISPLAY_ROWS; 
  bm_info_header.biPlanes=1;
  bm_info_header.biBitCount=24; 
  bm_info_header.biCompression=BI_RGB; 
  bm_info_header.biSizeImage=0; 
  bm_info_header.biXPelsPerMeter=0; 
  bm_info_header.biYPelsPerMeter=0;
  bm_info_header.biClrUsed=0; 
  bm_info_header.biClrImportant=0;
  // bm_info.bmiColors=NULL;
  bm_info.bmiHeader=bm_info_header;
  SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
				0, /* first scan line */
				DISPLAY_ROWS, /* number of scan lines */
				disp_image,&bm_info,DIB_RGB_COLORS);

  sprintf(text, "Frame: %d/%d      ", FrameIndex, TotalData); /* new addition 4/16/2023 */
  TextOut(hDC, DISPLAY_COLS+20, 50, (LPCSTR)text, strlen(text));
  sprintf(text, "Keyboard Shortcuts:"); /* new addition 4/18/2023 */
  TextOut(hDC, DISPLAY_COLS + 20, 90, (LPCSTR)text, strlen(text));
  sprintf(text, "Place Points - \"n\""); /* new addition 4/18/2023 */
  TextOut(hDC, DISPLAY_COLS + 20, 110, (LPCSTR)text, strlen(text));
  sprintf(text, "Modify Points - \"m\""); /* new addition 4/18/2023 */
  TextOut(hDC, DISPLAY_COLS + 20, 130, (LPCSTR)text, strlen(text));
  sprintf(text, "Delete Points - \"DEL\""); /* new addition 4/18/2023 */
  TextOut(hDC, DISPLAY_COLS + 20, 150, (LPCSTR)text, strlen(text));
  sprintf(text, "Interpolate %d Frames Ahead - \"i\"", nFrames); /* new addition 4/18/2023 */
  TextOut(hDC, DISPLAY_COLS + 20, 170, (LPCSTR)text, strlen(text));
  sprintf(text, "Save - \"ENTER\""); /* new addition 4/18/2023 */
  TextOut(hDC, DISPLAY_COLS + 20, 190, (LPCSTR)text, strlen(text));
  sprintf(text, "Save and Exit - \"CTRL+ENTER\""); /* new addition 4/18/2023 */
  TextOut(hDC, DISPLAY_COLS + 20, 210, (LPCSTR)text, strlen(text));
  
  ReleaseDC(MainWnd,hDC);
  EndPaint(MainWnd,&Painter);
  }
}


LRESULT CALLBACK ChangeFFSpeed(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
char	  text[320];

switch(uMsg)
  {
  case WM_INITDIALOG:
	  if (changeFFSpeed)
	  {
		  sprintf(text, "%d", FFspeed);
		  SetDlgItemText(hDlg, IDC_EDIT1, (LPCSTR)text);
		  break;
	  }
	  if (changeInterpolationLength)
	  {
		  sprintf(text, "%d", nFrames);
		  SetDlgItemText(hDlg, IDC_EDIT1, (LPCSTR)text);
		  break;
	  }
  case WM_COMMAND:
	switch(LOWORD(wParam))
	{
	  case IDOK:
		  if (changeFFSpeed)
		  {
			  GetDlgItemText(hDlg, IDC_EDIT1, (LPSTR)text, 320);
			  FFspeed = atoi(text);
			  if (FFspeed < 2)
				  FFspeed = 2;
			  if (FFspeed > 150)
				  FFspeed = 150;
			  EndDialog(hDlg, IDOK);
		  }
		  if (changeInterpolationLength)
		  {
			  GetDlgItemText(hDlg, IDC_EDIT1, (LPSTR)text, 320);
			  nFrames = atoi(text);
			  nFrames = (nFrames < 15) ? 15 : nFrames;
			  EndDialog(hDlg, IDOK);
		  }
		break;
	}
	  break;
	default:
	  return(FALSE);
  }
return(TRUE);
}

void DrawPoint(int xval, int yval, int DrawMode) // draw = 0 means change back to image, draw = 1 means place point, draw mode 2 is to place a new point and to record
{
	HDC		hDC;
	double angle, x1, y1;
	int radius = 0;
	char text[320];
	//int cxmouse = xmouse;
	//int cymouse = ymouse;


	hDC = GetDC(MainWnd);
	
	if (saveIndicator == TRUE)
	{
		hDC = GetDC(MainWnd);
		sprintf(text, "                              "); /* new addition 5/18/2023 */
		TextOut(hDC, DISPLAY_COLS + 20, 600, (LPCSTR)text, strlen(text));
		ReleaseDC(MainWnd, hDC);
		saveIndicator = FALSE;
	}

	if (DrawMode == 0)
	{
		//cxmouse = x_change;
		//cymouse = y_change;
		
		while (radius < POINT_SIZE)
		{
			for (angle = 0; angle < 360; angle += 10)
			{
				x1 = radius * cos(angle * 3.14159 / 180.0);
				y1 = radius * sin(angle * 3.14159 / 180.0);
				if ((yval + (int)y1 < DISPLAY_ROWS - 1))
				{
					SetPixel(hDC, xval + (int)x1, yval + (int)y1,
						RGB((int)disp_image[((yval + (int)y1) * DISPLAY_COLS + xval + (int)x1) * 3 + 2],
							(int)disp_image[((yval + (int)y1) * DISPLAY_COLS + xval + (int)x1) * 3 + 1],
							(int)disp_image[((yval + (int)y1) * DISPLAY_COLS + xval + (int)x1) * 3 + 0]));
				}
				
			}
			radius++;
		}
		
		ReleaseDC(MainWnd, hDC);
	}
	else
	{
		yval = (yval > DISPLAY_ROWS) ? DISPLAY_ROWS : yval;
		while (radius < POINT_SIZE)
		{
			for (angle = 0; angle < 360; angle += 10)
			{
				x1 = radius * cos(angle * 3.14159 / 180.0);
				y1 = radius * sin(angle * 3.14159 / 180.0);
				if (yval + (int)y1 < DISPLAY_ROWS-1)
				{
					SetPixel(hDC, xval + (int)x1, yval + (int)y1, 0x000000FF);
				}
			}
			radius++;
		}
		if (DrawMode == 2)
		{
			pointData[FrameIndex].x[ (pointData[FrameIndex].point_count) ] = xval;
			pointData[FrameIndex].y[ (pointData[FrameIndex].point_count) ] = yval;
		}
		
	}

}

void DrawLine(int startX, int startY, int endX, int endY, int mode) // draw = 0 means change back to image, draw = 1 means draw line
{
	int dx, dy;
	int steps;
	double Xinc, Yinc, X, Y;
	HDC		hDC;

	dx = endX - startX;
	dy = endY - startY;
	steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);  // if magnitude of dx is larger than dy then steps = dx; steps = dy otherwise
	Xinc = dx / (float)steps;
	Yinc = dy / (float)steps;

	X = startX;
	Y = startY;
	hDC = GetDC(MainWnd);
	if (mode == 0)
	{

		for (int i = 0; i <= steps; i++)
		{
			if (Y > DISPLAY_ROWS)
			{
				continue;
			}
			SetPixel(hDC, round(X), round(Y),
					RGB((int)disp_image[(int)(round(Y) * DISPLAY_COLS + round(X)) * 3 + 2],
					(int)disp_image[(int)(round(Y) * DISPLAY_COLS + round(X)) * 3 + 1],
					(int)disp_image[(int)(round(Y) * DISPLAY_COLS + round(X)) * 3 + 0]));
			X += Xinc;
			Y += Yinc;
		}

	}
	else
	{
		for (int i = 0; i <= steps; i++)
		{
			if (Y > DISPLAY_ROWS)
			{
				continue;
			}
			SetPixel(hDC, round(X), round(Y), 0x000000FF);
			X += Xinc;
			Y += Yinc;
		}
	}
	ReleaseDC(MainWnd, hDC);
}



void LoadNextFrame()
{
	int X, Y;

	/* Display points and lines that are already recorded for this frame without recording anything new. */
	for (int i = 0; i < pointData[FrameIndex].point_count; i++)
	{
		X = pointData[FrameIndex].x[i];
		Y = pointData[FrameIndex].y[i];
		DrawPoint(X, Y, 1);
		if ((pointData[FrameIndex].point_count > 0) && (i > 0))
		{
			DrawLine(pointData[FrameIndex].x[i - 1], pointData[FrameIndex].y[i - 1],
				X, Y, 1);
		}
	}
}

void DeletePoint(int x_change, int y_change)
{
	// erase points
	DrawPoint(x_change, y_change, 0);

	if (pointData[FrameIndex].point_count <= 1)
	{
		pointData[FrameIndex].point_count -= 1;
		return;
	}

	// erase lines
	if ((selected_point > 0) && (selected_point < pointData[FrameIndex].point_count - 1)) // if point is between two points
	{
		DrawLine(pointData[FrameIndex].x[selected_point - 1], pointData[FrameIndex].y[selected_point - 1],
			x_change, y_change, 0);
		DrawLine(x_change, y_change,
			pointData[FrameIndex].x[selected_point + 1], pointData[FrameIndex].y[selected_point + 1], 0);
		DrawLine(pointData[FrameIndex].x[selected_point - 1], pointData[FrameIndex].y[selected_point - 1],
			pointData[FrameIndex].x[selected_point + 1], pointData[FrameIndex].y[selected_point + 1], 1);
		for (int i = selected_point; i < pointData[FrameIndex].point_count - 1; i++)
		{
			pointData[FrameIndex].x[i] = pointData[FrameIndex].x[i + 1];
			pointData[FrameIndex].y[i] = pointData[FrameIndex].y[i + 1];
		}
		pointData[FrameIndex].point_count -= 1;
	}
	else if ( selected_point == (pointData[FrameIndex].point_count - 1) ) // if point is last point
	{
		DrawLine(pointData[FrameIndex].x[selected_point - 1], pointData[FrameIndex].y[selected_point - 1],
				x_change, y_change, 0);
		pointData[FrameIndex].point_count -= 1;
	}
	else // if point is first point
	{
		DrawLine(x_change, y_change,
				pointData[FrameIndex].x[selected_point + 1], pointData[FrameIndex].y[selected_point + 1], 0);
		for (int i = selected_point; i < pointData[FrameIndex].point_count - 1; i++)
		{
			pointData[FrameIndex].x[i] = pointData[FrameIndex].x[i + 1];
			pointData[FrameIndex].y[i] = pointData[FrameIndex].y[i + 1];
		}
		pointData[FrameIndex].point_count -= 1;
	}
}

void ModifyPoint(int xmouse, int ymouse, int xchange, int ychange)
{

	DrawPoint(xchange, ychange, 0);
	DrawPoint(xmouse, ymouse, 1);

	if (pointData[FrameIndex].point_count <= 1)
	{
		return;
	}

	if ((selected_point > 0) && (selected_point < pointData[FrameIndex].point_count - 1))
	{
		DrawLine(pointData[FrameIndex].x[selected_point - 1], pointData[FrameIndex].y[selected_point - 1],
			xchange, ychange, 0);
		DrawLine(pointData[FrameIndex].x[selected_point - 1], pointData[FrameIndex].y[selected_point - 1],
			xmouse, ymouse, 1);
		DrawLine(xchange, ychange,
			pointData[FrameIndex].x[selected_point + 1], pointData[FrameIndex].y[selected_point + 1], 0);
		DrawLine(xmouse, ymouse,
			pointData[FrameIndex].x[selected_point + 1], pointData[FrameIndex].y[selected_point + 1], 1);
	}
	else if (selected_point == pointData[FrameIndex].point_count - 1)
	{
		DrawLine(pointData[FrameIndex].x[selected_point - 1], pointData[FrameIndex].y[selected_point - 1],
			xchange, ychange, 0);
		DrawLine(pointData[FrameIndex].x[selected_point - 1], pointData[FrameIndex].y[selected_point - 1],
			xmouse, ymouse, 1);
	}
	else
	{
		DrawLine(xchange, ychange,
			pointData[FrameIndex].x[selected_point + 1], pointData[FrameIndex].y[selected_point + 1], 0);
		DrawLine(xmouse, ymouse,
			pointData[FrameIndex].x[selected_point + 1], pointData[FrameIndex].y[selected_point + 1], 1);
	}
}

void LoadCSVData(char *file_name)
{
	FILE *fpt;
	int i, frame_number, frame=0;


	// code to detect size of data set
	if ((fpt = fopen(file_name, "r")) == NULL)
	{
		exit(0);
	}
	fscanf(fpt, "%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s,%*s");
	fscanf(fpt, "\n");
	while (frame < TotalData)
	{
		fscanf(fpt, "%d,%d", &frame_number, &pointData[frame].point_count);
		for (i = 0; i < 10; i++)
		{
			if (i < pointData[frame].point_count)
			{
				fscanf(fpt, ",%d,%d", &pointData[frame].x[i], &pointData[frame].y[i]);
			}
			else
			{
				fscanf(fpt, ",%*s,%*s");
			}
	
		}
		fscanf(fpt, "\n");
		frame++;
		
	}

}
