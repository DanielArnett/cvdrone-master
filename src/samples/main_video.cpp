#include "ardrone/ardrone.h"

#define KEY_DOWN(key) (GetAsyncKeyState(key) & 0x8000)
#define KEY_PUSH(key) (GetAsyncKeyState(key) & 0x0001)

// --------------------------------------------------------------------------
// main(Number of arguments, Value of arguments)
// This is the main function.
// Return value Success:0 Error:-1
// --------------------------------------------------------------------------
int main(int argc, char **argv)
{
    // AR.Drone class
    ARDrone ardrone;

    // Initialize
    if (!ardrone.open()) {
        printf("Failed to initialize.\n");
        return -1;
    }

    // Image of AR.Drone's camera
    IplImage *image = ardrone.getImage();

    // Create a video writer
    char filename[256];
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf(filename, "cam%d%02d%02d%02d%02d%02d.avi", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    CvVideoWriter *video = cvCreateVideoWriter(filename, CV_FOURCC('D','I','B',' '), 30, cvGetSize(image));

    // Main loop
    while (!GetAsyncKeyState(VK_ESCAPE)) {
        // Update your AR.Drone
        if (!ardrone.update()) break;

        // Getting an image
        image = ardrone.getImage();

        // Write a frame
        cvWriteFrame(video, image);

        // Display the image
        cvShowImage("camera", image);
        cvWaitKey(33);
    }

    // Save video
    cvReleaseVideoWriter(&video);

    // See you
    ardrone.close();

    return 0;
}