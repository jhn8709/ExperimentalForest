
#define _CRT_SECURE_NO_WARNINGS	  /* disable compiler warnings for standard C functions like strcpy() */

#define SQR(x)	((x)*(x))

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <winuser.h>
#include "globals.h"
#include "resource.h"
#include <iostream>
#include <fstream>
#include <sstream>


//#include <opencv2/video.hpp>
using namespace std;

int          selected_point;		/* indices of the point closest to where the user has clicked */
GroundTruth *pointData;  // Groud Truth storing the labels
GroundTruth* pointData_2;
//int			*deletedPointsIndexes;
vector<int>	deletedPointsIndexes;
int			deletedPointsCount;
int 		compareReady{0};

void AllocateStruct(int frame_count)
{
	//pointData = (GroundTruth*)calloc(frame_count, sizeof(GroundTruth));
	pointData = new GroundTruth[frame_count]();
	if (pointData == nullptr) {
		cout << "Failure to allocate!" << "\n";
	}
}

void saveDeletedPointsInit()
{
	//deletedPointsIndexes = (int*)calloc(10, sizeof(int));
	deletedPointsIndexes.clear();
	//deletedPointsCount = 0;
}

void saveDeletedPoint(int index)
{
	//deletedPointsIndexes[deletedPointsCount] = index;
	//deletedPointsCount++;
	deletedPointsIndexes.push_back(index);
}

void removeDeletedPoint()
{
	// Remove the last element from the vector
	if (!deletedPointsIndexes.empty()) {
		deletedPointsIndexes.pop_back();
	}
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

	  /*if (compareReady == 2) {
		applyMask();
	  }
	  else {
		ReadVideoFrame();
	  }*/
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
	  break;

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
	if (compareReady != 2) {
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

void LoadCSVData(char* filename) {
	ifstream file (filename);
	int frame = 0;
	int p_count;

	if (!file) {
		std::cerr << "Failed to opent the file \n";
		return;
	}

	string line;
	getline(file, line);
	while (getline(file, line)) {
		stringstream iss(line);
		string token;
		
		std::getline(iss, token, ',');
		std::getline(iss, token, ',');
		p_count = stoi(token);

		if (compareReady == 2)
		{
			pointData_2 = new GroundTruth[TotalData]();
			if (pointData_2 == nullptr) {
				cout << "Failure to allocate!" << "\n";
			}

			pointData_2[frame].point_count = p_count;
			for (int i = 0; i < 10; i++) {
				getline(iss, token, ',');
				pointData_2[frame].x[i] = (token == "NaN") ? 0 : stoi(token);
				getline(iss, token, ',');
				pointData_2[frame].y[i] = (token == "NaN") ? 0 : stoi(token);
			}
		}
		else {
			pointData[frame].point_count = p_count;
			for (int i = 0; i < 10; i++) {
				getline(iss, token, ',');
				pointData[frame].x[i] = (token == "NaN") ? 0 : stoi(token);
				getline(iss, token, ',');
				pointData[frame].y[i] = (token == "NaN") ? 0 : stoi(token);
			}
		}
		frame++;
	}
}

void UpdateMode(int mode, int flag) /* new addition 5/26/2023 */
{
	char text[320];
	HDC hDC;

	hDC = GetDC(MainWnd);
	switch (mode)
	{
	case ID_LABELING_PLACEPOINTS:
		sprintf(text, "Click to Place Points----------------------");
		break;
	case ID_LABELING_MODIFYPOINTS:
		sprintf(text, "Click to Modify Points---------------------");
		break;
	case ID_LABELING_DELETEPOINTS:
		sprintf(text, "Click to Delete Points---------------------");
		break;
	}
	if (flag == FALSE)
	{
		sprintf(text, "Press \"n\",\"m\",or \"DELETE\" to Edit");
	}
	TextOut(hDC, DISPLAY_COLS + 20, 250, (LPCSTR)text, strlen(text));

	ReleaseDC(MainWnd, hDC);
}

void storeGTData(vector<int>& xVector)
{
	int point_count{ pointData[FrameIndex].point_count };
	int i{ 1 };

	if (point_count < 2)
	{
		return;
	}

	while (i < point_count)
	{
		FillGT(pointData[FrameIndex].x[i - 1], pointData[FrameIndex].y[i - 1],
			pointData[FrameIndex].x[i - 0], pointData[FrameIndex].y[i - 0], xVector);
		i++;
	}


}

void storeGTInterpolationData(vector<int>& xVector, int *xvals, int *yvals, int frameIndex)
{
	int i{ 1 };
	if (pointData[frameIndex].point_count < 2)
	{
		return;
	}

	while (i < pointData[frameIndex].point_count)
	{
		FillGT(xvals[i - 1], yvals[i - 1],
			xvals[i], yvals[i], xVector);
		i++;
	}
}

void FillGT(int startX, int startY, int endX, int endY, vector<int>& xVector)// int mode)
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

	for (int i = 0; i <= steps; i++)
	{

		if ( (Y > DISPLAY_ROWS) || (Y < HORIZON) )
		{
			continue;
		}
		xVector[round(Y) - HORIZON] = X;

		X += Xinc;
		Y += Yinc;
	}
}