#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;


///////////////// Color Detection //////////////////////

int main()
{
	string path = "../Resources/lambo.png"; /* gives path to find file */
	Mat img = imread(path); /* puts the image file into a openCV style array or data structure */
	imshow("Image", img); /* function to display image */
	waitKey(0); // /* keeps image displayed until a key is pressed */

}