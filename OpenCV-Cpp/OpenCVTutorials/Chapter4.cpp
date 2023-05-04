#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

///////////////////////  Draw Shapes and Text ////////////////////////////////////

int main()
{
	// Blank Image
	Mat img(512, 512, CV_8UC3, Scalar(255, 255, 255));

	circle(img, Point(256, 256), 155, Scalar(0, 69, 255), FILLED); /* FILLED is a setting that fills the shape with color */
	rectangle(img, Point(130, 226), Point(382, 286), Scalar(255, 255, 255), FILLED);
	line(img, Point(130, 296), Point(382, 296), Scalar(255, 255, 255), 2);

	putText(img, "OpenCV Tutorial", Point(137, 262), FONT_HERSHEY_COMPLEX, 0.75, Scalar(0, 69, 255));

	imshow("Image", img); /* function to display image */

	waitKey(0); // /* keeps image displayed until a key is pressed */

}