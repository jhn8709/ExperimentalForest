# ExperimentalForest
C/C++ Code for experimental forest data labeler.

ForestLabelOpenCV Folder has most of the files needed to successfully run the program. To run the program, OpenCV .dll files need to be placed in the same folder
as the executable. These files can be downloaded at: https://opencv.org/releases/ under OpenCV 4.6.0 for Windows. The files needed are:
1) opencv_world460.dll
2) opencv_videoio_msmf460_64.dll
3) opencv_videoio_ffmpeg460_64.dll

The path to these files is: opencv\build\x64\vc15\bin

There are a couple of things you should know before using the program:
1) You will likely want to use videos that are 1280x720 so you can see the UI. A feature that automatically resizes and allows the user
to resize the image will be added later.
2) You should expect to get a csv file when clicking enter (save feature)
3) You will likely want to save often as the program does crash without saving progress.
4) The interpolation will automatically stop when 2 points cannot be tracked. Points will usually lose tracking when they reach
the bottom of the image area. Try to modify the points before this happens.
5) The amount of frames interpolated and the amount of frames skipped when using 'fast forward' buttons can be changed
in the settings tab at the top right.

The standard procedure is:
1) Place the points by pressing n.
2) Then, press i to interpolate the points forward.
3) You can press m to modify point locations or del to delete points. You can always press i to interpolate again.
4) Use s and d to move backwards and forwards through the frames.
- Use a to move quickly backwards and g to move quickly forwards.
6) Export all the points to a .csv file when pressing enter.
