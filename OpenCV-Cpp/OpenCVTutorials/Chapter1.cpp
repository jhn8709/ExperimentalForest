#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;


/////////////////  Images  //////////////////////

//int main()
//{
//	string path = "../Resources/test.png"; /* gives path to find file */
//	Mat img = imread(path); /* puts the image file into a openCV style array or data structure */
//	imshow("Image", img); /* function to display image */
//	waitKey(0); // /* keeps image displayed until a key is pressed */
//
//}

/////////////////  Video  //////////////////////

int main()
{
	VideoCapture cap("C:/Program Files (x86)/PedometerProject/StepView-installer/Videos/P001/Regular/Conv_0076.mp4"); /* files can also be read directly like this */
	Mat img;

	while (true)
	{
		cap.read(img); /* read in frame */
		imshow("Image", img);
		waitKey(20); /* wait for X milliseconds */
	}

}

/////////////////  Webcam  //////////////////////

//int main()
//{
//	VideoCapture cap(0); /* Use connected camera (in this case 0 denotes the 1st camera which is usually the laptop webcam */
//	Mat img;
//
//	while (true)
//	{
//		cap.read(img); /* read in frame */
//		imshow("Image", img);
//		waitKey(20); /* wait for X milliseconds */
//	}
//
//}