#define _CRT_SECURE_NO_WARNINGS	  /* disable compiler warnings for standard C functions like strcpy() */
//#define _NO_CRT_STDIO_INLINE

#include <cstdio>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <math.h>
#include <filesystem>

namespace fs = std::filesystem;

#include "globals.h"
#include "resource.h"

/* global variables */
HINSTANCE	  hInst;				/* pointer to program instance (need for dialog boxes) */
HWND		  MainWnd;				/* main window */
bool Playing{ FALSE };				/* 0=>no; 1=>yes (video is playing, using timer to repeatedly move TimeIndex) */
bool PlaceDot{ FALSE };				/* 0=>no; 1=>yes (user can place points in window) */
bool ModifyDot{ FALSE };				/* 0=>no; 1=>yes (user can move placed points in window) */
bool DeleteDot{ FALSE };				/* 0=>no; 1=>yes (user can delete placed points in window) */
bool SelectDot{ FALSE };				/* 0=>no; 1=>yes (program will detect closest point to click) */
bool changeFFSpeed{ FALSE };
bool changeInterpolationLength{ FALSE };
bool saveIndicator{ FALSE };

char		  DataFilename[320];	/* file that contains the accelerometer data (synchronized and collated at 15 Hz) */
char		  CurrentPath[320];		/* path where files were last read/saved */
int			  FrameIndex;			/* index of data currently being displayed (units are samples, 1/30 Hz) */
int			  PlayJump;				/* amount of samples to jump during each timer */
int			  FFspeed;				/* user-definable amount of frames to skip per iteration during fast forward */
int			  PlayCountdown;		/* used to play brief video sequences, halting when it counts down to zero */

//struct UndoData
//{
//	int actionType[5];
//	int x[5];
//	int y[5];
//	int actions_made{0};
//};
//UndoData undo;

void		DataToCSV();

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow)

{
MSG			msg;
WNDCLASS	wc;
LRESULT		CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
//FILE		*fpt;
//int			i;

wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=hInstance;
hInst=hInstance;
wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(ID_ICON));
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName=MAKEINTRESOURCE(ID_MAIN_MENU);
wc.lpszClassName=(LPCSTR)"STEPVIEW";

if (!RegisterClass(&wc))
  return(FALSE);

MainWnd=CreateWindow((LPCSTR)"STEPVIEW",(LPCSTR)"Forest Labeler",
			WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL | WS_MAXIMIZE,
			0,0,600+650,800,NULL,NULL,hInstance,NULL);
if (!MainWnd)
  return(FALSE);

ShowScrollBar(MainWnd,SB_BOTH,FALSE);
ShowWindow(MainWnd,SW_MAXIMIZE);
InvalidateRect(MainWnd,NULL,TRUE);
UpdateWindow(MainWnd);

FFspeed = 5;

InitializeDataVariables();

while (GetMessage(&msg,NULL,0,0))
  {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  }
return(msg.wParam);
}



LRESULT CALLBACK WndProc (HWND hWnd,
						  UINT uMsg,
						  WPARAM wParam,
						  LPARAM lParam)

{
int				i;
static int		xmouse, ymouse, xchange, ychange;
OPENFILENAME	ofn;
//HMENU			hAppMenu;
char			text[320];
/* Newer additions 4/2/2023 */
double			dist, local_min = 10000;
fs::path filePath;
HDC				hDC;


switch (uMsg)
  {
  case WM_COMMAND:
	switch (LOWORD(wParam))
	  {
	  case ID_FILE_LOAD:
		memset(&(ofn),0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.lpstrFile=(LPSTR)DataFilename;
		DataFilename[0]=0;
		ofn.nMaxFile=320;
		ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
		// ofn.lpstrFilter="Data files\0P???_*.txt\0All files\0*.*\0\0";
		ofn.lpstrFilter = (LPCSTR)"All files\0 * .*\0\0";
		
		if (!( GetOpenFileName(&ofn))  ||  DataFilename[0] == '\0')
		  break;
		filePath = DataFilename;
		
						// set currentpath to where file loaded from 
		strcpy(CurrentPath,DataFilename);
		i=strlen(CurrentPath)-1;
		while (i > 0  &&  CurrentPath[i] != '\\')
		  i--;
		CurrentPath[i]='\0';
		SetCurrentDirectory((LPCTSTR)CurrentPath);
		
		/*if (VideoLoaded)
		  {
		  if (disp_image != NULL)
			{
			free(disp_image);
			disp_image=NULL;
			}
		  CloseVideoFile();
		  }*/
		//InitializeDataVariables();
		if (filePath.extension() == ".mp4") // Heed the dot.
		{
			ReadVideo(DataFilename);
		}
		else if (filePath.extension() == ".csv")
		{
			LoadCSVData(DataFilename);
		}


		//ReadVideo(DataFilename);
		sprintf(text, "Data Labeling   %s", DataFilename);
		SetWindowText(hWnd,(LPCSTR)text);
		PaintImage();
		break;

	 
	  case ID_EDIT_FF_SPEED:
		changeFFSpeed = TRUE;
		DialogBox(hInst,MAKEINTRESOURCE(IDD_FF_DIALOG),hWnd,(DLGPROC)ChangeFFSpeed);
		changeFFSpeed = FALSE;
		PaintImage();
		break;

	  case ID_SETTINGS_INTERPOLATION:
		  changeInterpolationLength = TRUE;
		  DialogBox(hInst, MAKEINTRESOURCE(IDD_INTP_DIALOG), hWnd, (DLGPROC)ChangeFFSpeed);
		  changeInterpolationLength = FALSE;
		  break;


	  case ID_HELP_KEYS:
		sprintf(text,"Key(s)\tAction(s)\n-------------------------------------------------\n");
		strcat(text,"asdfg\tplayback controls   << < [] > >>\n");
		strcat(text,"[]\tprevious/next step\n");
		strcat(text,"p\tplayback step (-1 sec ... +1 sec)\n");
		strcat(text,"-+\tload prev/next file\n");
		MessageBox(hWnd,(LPCSTR)text,(LPCSTR)"Keyboard controls",MB_OK | MB_APPLMODAL);
		break;
	 
	  case ID_LABELING_PLACEPOINTS:
		  PlaceDot = !PlaceDot;
		  SelectDot = FALSE;
		  DeleteDot = FALSE;
		  UpdateMode(ID_LABELING_PLACEPOINTS, PlaceDot);
		  break;

	  case ID_LABELING_MODIFYPOINTS:
		  SelectDot = !SelectDot;
		  PlaceDot = FALSE;
		  DeleteDot = FALSE; 
		  UpdateMode(ID_LABELING_MODIFYPOINTS, SelectDot);
		  break;
	  
	  case ID_LABELING_DELETEPOINTS:
		  PlaceDot = FALSE;
		  SelectDot = FALSE;
		  DeleteDot = !DeleteDot;
		  UpdateMode(ID_LABELING_DELETEPOINTS, DeleteDot);
		  break;

	  case ID_QUIT:
		if (VideoLoaded)
		  {
		  /*if (disp_image != NULL)
			{
			free(disp_image);
			disp_image=NULL;
			}*/
		  //CloseVideoFile();
		  VideoLoaded=0;
		  }

		Sleep(100);
		DestroyWindow(hWnd);
		break;
		
	  }
	break;

  case WM_LBUTTONDOWN:
	  xmouse = LOWORD(lParam);
	  ymouse = HIWORD(lParam);

	  // select location where the fixed point is placed
	  if (ModifyDot == TRUE)
	  {
		  //RecordAction(pointData[FrameIndex].x[selected_point], pointData[FrameIndex].y[selected_point], 1);
		  pointData[FrameIndex].x[selected_point] = xmouse;
		  pointData[FrameIndex].y[selected_point] = ymouse;
		  DrawPoint(xmouse, ymouse, 1);
		  ModifyDot = 0;  // stop the point moving process
	  }
	  // initiate point fixing
	  else if ((SelectDot == TRUE) || (DeleteDot == TRUE))
	  {
		  if (SelectDot == TRUE)
		  {
			  ModifyDot = TRUE;
		  }
		  //for (i = 0; i < PointTotals[FrameIndex]; i++)
		  for (i = 0; i < pointData[FrameIndex].point_count; i++)
		  {
			  dist = sqrt((double)SQR(pointData[FrameIndex].x[i] - xmouse)
				   + (double)SQR(pointData[FrameIndex].y[i] - ymouse));
			  if (dist < local_min)
			  {
				  local_min = dist;
				  //xchange = px[FrameIndex][i];
				  //ychange = py[FrameIndex][i];
				  xchange = pointData[FrameIndex].x[i];
				  ychange = pointData[FrameIndex].y[i];
				  selected_point = i;
			  }
		  }
	  }
	  if ((PlaceDot == TRUE) && (ModifyDot == FALSE))
	  {
		  //RecordAction(xmouse, ymouse, 2);
		  DrawPoint(xmouse, ymouse, 2);
		  //if (PointTotals[TimeIndex] > 0)
		  if (pointData[FrameIndex].point_count > 0)
		  {
			  DrawLine(pointData[FrameIndex].x[pointData[FrameIndex].point_count-1], pointData[FrameIndex].y[pointData[FrameIndex].point_count - 1],
				  pointData[FrameIndex].x[pointData[FrameIndex].point_count], pointData[FrameIndex].y[pointData[FrameIndex].point_count], 1);
		  } 
		  pointData[FrameIndex].point_count += 1;
	  }
	  if (DeleteDot == TRUE)
	  {
		  //RecordAction(xchange, ychange, 0);
		  DeletePoint(xchange, ychange);
	  }
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_LBUTTONUP:	  
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_MOUSEMOVE:

	  xmouse = LOWORD(lParam);
	  ymouse = HIWORD(lParam);
	  // have the contour be visible while moving it
	  if (ModifyDot == 1)
	  {
		  ModifyPoint(xmouse, ymouse, xchange, ychange);
		  xchange = xmouse;
		  ychange = ymouse;
	  }
	 
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_HSCROLL:
	break;

  case WM_CHAR:
									/* all these controls set the current PlayJump */
	if (((TCHAR)wParam == 'a') || ((TCHAR)wParam == 'A'))
 	  {
	  PlayJump=((TCHAR)wParam == 'a' ? -FFspeed : -FFspeed*5);
	  UpdateDisplay();
	  LoadNextFrame();
	  }
	if (((TCHAR)wParam == 's') || ((TCHAR)wParam == 'S'))
 	  {
	  PlayJump=-1;
	  UpdateDisplay();
	  LoadNextFrame();
	  }
	if (((TCHAR)wParam == 'f') || ((TCHAR)wParam == 'F'))
 	  {
	  PlayJump=+1;
	  UpdateDisplay();
	  LoadNextFrame();
	  }
	if (((TCHAR)wParam == 'g') || ((TCHAR)wParam == 'G'))
 	  {
	  PlayJump=((TCHAR)wParam == 'g' ? FFspeed : FFspeed*5);
	  UpdateDisplay();
	  LoadNextFrame();
	  }
	if (((TCHAR)wParam == 'd') || ((TCHAR)wParam == 'D'))
	  {	
	  Playing=(Playing+1)%2;
	  if (Playing == 1)
		SetTimer(hWnd,TIMER_SECOND,10,NULL);
	  else
		KillTimer(hWnd,TIMER_SECOND);
	  UpdateDisplay();
	  }
	/* added 4/18/2023 */
	if (((TCHAR)wParam == 'm') || ((TCHAR)wParam == 'M')) /* toggle modify points */
	{
		PostMessage(MainWnd, WM_COMMAND, ID_LABELING_MODIFYPOINTS, 0);
	}
	if (((TCHAR)wParam == 'n') || ((TCHAR)wParam == 'N')) /* toggle add points */
	{
		PostMessage(MainWnd, WM_COMMAND, ID_LABELING_PLACEPOINTS, 0);
	}
	if (((TCHAR)wParam == 'i') || ((TCHAR)wParam == 'I')) /* interpolate the current frames to the next set of frames */
	{
		InterpolateFrames();
		if (InterruptError == TRUE)
		{
			sprintf(text, "Error!\n-------------------------------------------------\n");
			strcat(text, "More than 2 points have lost tracking.\n");
			strcat(text, "This occurs when 2 points reach the edge of the screen.\n");
			strcat(text, "Modify or delete points and then interpolate again.\n");
			MessageBox(hWnd, (LPCSTR)text, (LPCSTR)"Interpolation Interrupted!", MB_OK | MB_APPLMODAL);
			InterruptError = FALSE;
		}
	}
	if (((TCHAR)wParam == 'p') || ((TCHAR)wParam == 'P'))	  /* play a step -- from 1 seconds before, to 1 second after current time */
	  {
	  if (PlayCountdown == 0)
		{		/* start playback of step 1 second prior and end 1 second after */
		FrameIndex-=15;
		if (FrameIndex < 0)
		  FrameIndex=0;
		PlayJump=+1;
		PlayCountdown=30;
		SendMessage(hWnd,WM_CHAR,(TCHAR)'d',0);
		}
	  else
		{		/* end playback currently in operation */
		PlayCountdown=0;
		SendMessage(hWnd,WM_CHAR,(TCHAR)'d',0);
		}
	  }
	break;

  case WM_KEYDOWN:
	  switch (wParam)
	  {

		case (VK_DELETE): /* Enable deleting points */
			PostMessage(MainWnd, WM_COMMAND, ID_LABELING_DELETEPOINTS, 0);
			break;
		case(VK_RETURN): /* create the line after all points are on the image */
			DataToCSV();
			hDC = GetDC(MainWnd);
			sprintf(text, "Progress Saved!"); /* new addition 5/18/2023 */
			TextOut(hDC, DISPLAY_COLS + 20, 600, (LPCSTR)text, strlen(text));
			ReleaseDC(MainWnd, hDC);
			saveIndicator = TRUE;
			if (GetAsyncKeyState(VK_CONTROL) < 0)
			{
				PostMessage(MainWnd, WM_COMMAND, ID_QUIT, 0);
			}
			
			break;
			
	  }
	  break;

  case WM_SIZE:
	if (hWnd != hWnd  ||  GetUpdateRect(hWnd,NULL,FALSE) == 0)
	  return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;

  case WM_PAINT:
	PaintImage();
	return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;

  case WM_TIMER:
	if (wParam == TIMER_SECOND)
	  {
	  UpdateDisplay();
	  LoadNextFrame();
	  if (PlayCountdown > 0)	  /* playing a brief sequence; stop when counter reaches zero */
		{
		PlayCountdown--;
		if (PlayCountdown == 0)
		  SendMessage(hWnd,WM_CHAR,(TCHAR)'d',0);
		}
	  }
	break;
	
  case WM_DESTROY:
	Sleep(100);
	DestroyWindow(hWnd);
	PostQuitMessage(0);
	break;
	
  default:
	return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  }

return(0L);
}




		/* moves the TimeIndex according to PlayJump, then calls PaintImage() */

void UpdateDisplay()

{
if (FrameIndex+PlayJump < 0  ||  FrameIndex+PlayJump >= TotalData)
  {
  if (FrameIndex+PlayJump < 0)
	FrameIndex=0;
  else
	FrameIndex=TotalData-1;
  if (Playing == 1)		  /* halt play if reached either start or end */
	{
	KillTimer(MainWnd,TIMER_SECOND);
	Playing=0;
	}
  }
else
  {
  FrameIndex+=PlayJump;
  //undo.actions_made = 0;
  }
PaintImage();

}


void InitializeDataVariables()

{
Playing=0;
PlayJump=1;
PlayCountdown=0;
}

void DataToCSV()
{
	FILE* fpt;
	int i=0 , j;

	fpt = fopen("GroundTruth.csv", "w+");
	if (fpt == NULL) 
	{
		// Handle the case when fopen fails
		return;
	}
	fprintf(fpt, "Frame,#ofPoints,X1,Y1,X2,Y2,X3,Y3,X4,Y4,X5,Y5,X6,Y6,X7,Y7,X8,Y8,X9,Y9,X10,Y10\n");
	//fprintf(fpt, "\n");
	for (i = 0; i <= TotalData; i++)
	{
		fprintf(fpt, "%d", i);
		fprintf(fpt, ",%d", pointData[i].point_count);
		for (j = 0; j < 10; j++)
		{
			if (j < pointData[i].point_count)
			{
				fprintf(fpt, ",%d,%d", pointData[i].x[j], pointData[i].y[j]);
			}
			else
			{
				fprintf(fpt, ",%s,%s", "NaN", "NaN");
			}
		}
		fprintf(fpt, "\n");
	}
	fclose(fpt);
}

/* action=0 means point was deleted, action=1 means point was moved, action=2 means point was placed */
//void RecordAction(int x_record, int y_record, int action) 
//{
//	undo.x[undo.actions_made] = x_record;
//	undo.y[undo.actions_made] = y_record;
//	undo.actionType[undo.actions_made] = action;
//	undo.actions_made++;
//}

