#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

///////////////////////  Resize and Crop ////////////////////////////////////

int main()
{
	string path = "../Resources/test.png"; /* gives path to find file */
	Mat img = imread(path); /* puts the image file into a openCV style array or data structure */
	Mat imgResize, imgCrop;

	//cout << img.size() << endl;
	resize(img, imgResize, Size(640, 480));

	Rect roi(100, 100, 300, 250); /* Creating rectangle data type that is 100x100 pixels large at x=300 and y=250 */
	imgCrop = img(roi); /* method to crop images */

	imshow("Image", img); /* function to display image */
	imshow("Image Resized", imgResize);
	imshow("Image Crop", imgCrop);

	waitKey(0); // /* keeps image displayed until a key is pressed */

}