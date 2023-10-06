# ExperimentalForest
C/C++ Code for experimental forest data labeler.

This program is a data labeling tool that is meant to label outdoor video. When starting the program an .mp4 file should be loaded in for the program to work.
The program allows the user to create line labels which signifies which areas are the most traversable or the most optimal path someone should take. It has an
interpolation algorithm that utilizes KLT tracking so that the user will not need to modify the points placed very often. The KLT algorithm should allow the 
labeler to create datasets very quickly compared to other data labeling programs/software.

The files in this repository were developed in Visual Studio 2022. If you want to work on this code, make sure to link OpenCV libraries to your project in your IDE.
The files used for OpenCV were downloaded at: https://opencv.org/releases/ under OpenCV 4.6.0 for Windows.

Use this link to help import OpenCV libraries.
https://learn.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp?view=msvc-170

From the "To add the DLL header to your include path" and the "To add the DLL import library to your project" sections should help.

Make sure to place the OpenCV extracted files in C:\opencv\opencv
