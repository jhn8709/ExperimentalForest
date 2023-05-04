#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

///////////////////////  Warp Images ////////////////////////////////////

float w = 250, h = 350; // size of cards
Mat matrix, imgWarp;

int main()
{
	string path = "../Resources/cards.jpg";
	Mat img = imread(path);

	Point2f src[4] = { {529,142}, {771,190}, {405,395}, {674,457} }; /* Location of corners of card */
	Point2f dst[4] = { {0.0f,0.0f}, {w,0.0f}, {0.0f,h}, {w,h} }; /* Realigned coordinates */

	matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w,h));

	for (int i = 0; i < 4; i++)
	{
		circle(img, src[i], 10, Scalar(0, 0, 255), FILLED);
	}


	imshow("Image", img); /* function to display image */
	imshow("Image Warp", imgWarp); 

	waitKey(0); // /* keeps image displayed until a key is pressed */
}