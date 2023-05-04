#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>


#define INTERPOLATION_MAX   300

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    int frame_count;

    VideoCapture capture("C:/Program Files (x86)/PedometerProject/StepView-installer/Videos/P001/Regular/Conv_0076.mp4");
    //VideoCapture capture("../Resources/slow_traffic_small.mp4");
    if (!capture.isOpened()) {
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
        return 0;
    }

    frame_count = (int)(capture.get(CAP_PROP_FRAME_COUNT));
    cout << frame_count << endl;

    /* Code to get specific frame in OpenCV */
    Mat setFrame;
    capture.set(CAP_PROP_POS_FRAMES, 710 - 1);
    capture.read(setFrame);
    //imshow("Set Frame", setFrame);
    //waitKey(0);

    // Create some random colors
    vector<Scalar> colors;
    RNG rng;
    for (int i = 0; i < 100; i++)
    {
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        colors.push_back(Scalar(r, g, b));
    }

    //Mat old_frame, old_gray;
    //vector<Point2f> p0, p1;
    ///* Take first frame and find corners in it */
    //capture >> old_frame;
    //cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
    //goodFeaturesToTrack(old_gray, p0, 100, 0.3, 7, Mat(), 7, false, 0.04);

    /* How to address vector<Point2f> data structure */
    //double py = p0[0].y;
    //double px = p0[0].x;
    //cout << "1st point is at location: (" << px << ", " << py << ")" << endl;

    /* Custom tracking points */
    Mat old_frame, old_gray;
    capture >> old_frame;
    cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
    vector<Point2f> p0(10), p1(10);
    p0[0] = Point2f(617, 638);
    p0[1] = Point2f(605, 336);
    p0[2] = Point2f(389, 233);
    p0[3] = Point2f(443, 195);


    // Create a mask image for drawing purposes
    Mat mask = Mat::zeros(old_frame.size(), old_frame.type());

    int iter = 0;
    while (iter < INTERPOLATION_MAX) {
        Mat frame, frame_gray;

        capture >> frame;
        if (frame.empty())
            break;
        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

        // calculate optical flow
        vector<uchar> status;
        vector<float> err;
        TermCriteria criteria = TermCriteria((TermCriteria::COUNT)+(TermCriteria::EPS), 10, 0.03);
        calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err, Size(15, 15), 2, criteria);

        vector<Point2f> good_new;
        for (uint i = 0; i < p0.size(); i++)
        {
            // Select good points
            if (status[i] == 1) {
                good_new.push_back(p1[i]);
                // draw the tracks
                line(mask, p1[i], p0[i], colors[i], 2);
                circle(frame, p1[i], 5, colors[i], -1);
            }
        }
        Mat img;
        add(frame, mask, img);

        imshow("Frame", img);

        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;

        // Now update the previous frame and previous points
        old_gray = frame_gray.clone();
        p0 = good_new;

        iter++;
    }
}