#include "ardrone/ardrone.h"
#include "3rdparty/ObjectDetectionUsingColor-Basic/BallTracking/stdafx.h"

#include "3rdparty/opencv/include/opencv/cv.h"
#include "3rdparty/opencv/include/opencv/highgui.h"
#include <boost/thread.hpp>
using namespace boost;

// --------------------------------------------------------------------------
// main(Number of arguments, Value of arguments)
// The ball/color tracking code was originated from Shermal Fernando's tutorial "Object Detection & Tracking using Color".
// Link: http://opencv-srf.blogspot.com/2010/09/object-detection-using-color-seperation.html
// --------------------------------------------------------------------------


#define KEY_DOWN(key) (GetAsyncKeyState(key) & 0x8000)
#define KEY_PUSH(key) (GetAsyncKeyState(key) & 0x0001)

// AR.Drone class
ARDrone ardrone("192.168.1.1");

const int MIDX = 320;
const int MIDY = 220;
const int DEAD_ZONE_X = 100;
const int DEAD_ZONE_Y = 50;
const double ALT_SENSITIVITY = 1;
const double YAW_SENSITIVITY = 1;
const double PITCH_SENSITIVITY = 0.2;
const int NOISE_THRESHOLD = 100;

int deadZoneMinX;
int deadZoneMaxX;
int deadZoneMinY;
int deadZoneMaxY;

int hueMin = 0;
int satMin = 80;
int valMin = 100;
int hueMax = 22;
int satMax = 256;
int valMax = 256;
double objectThreshold = 0;

double autoX = 0;
double autoY = 0;
double autoZ = 0;
double autoR = 0;
int posX = 0;
int posY = 0;

int battery = 0;

//Thread to control the drone
boost::thread controlThread;

void controlStream()
{
	while(true)
	{
		ardrone.move3D(autoX,autoY,autoZ,autoR);
		boost::this_thread::interruption_point();
	}
}
void trackColor(IplImage* imgThresh)
{
    // Calculate the moments of 'imgThresh', then extract the area of the tracked color.
    CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
    cvMoments(imgThresh, moments, 1);
    double moment10 = cvGetSpatialMoment(moments, 1, 0);
    double moment01 = cvGetSpatialMoment(moments, 0, 1);
    double area = cvGetCentralMoment(moments, 0, 0);
	free(moments);

    // Display
     // if the area<1000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
    
	if (KEY_PUSH(VK_RETURN)) 
	{
		objectThreshold = area;
	}
    //printf("Threshold :\t = %a\n", objectThreshold);
    //printf("Area    :\t = %a\n", area);
	if (ardrone.onGround() && objectThreshold != 0 && area > NOISE_THRESHOLD)
	{
		ardrone.flatTrim();
		//ardrone.takeoff();
	}

    // calculate the position of the ball
    posX = (moment10 / area) - MIDX;
    posY = -((moment01 / area) - MIDY);        
	
	battery = ardrone.getBatteryPercentage();
        
	// Move Forward/Backward
	if (area < objectThreshold)
	{
		if (area < (objectThreshold / 2))
		{
			autoX = (2 * PITCH_SENSITIVITY);
		}
		else
		{
			autoX = PITCH_SENSITIVITY;
		}
	}
	else if (area > (2 * objectThreshold))
	{
		autoX = -PITCH_SENSITIVITY;
	}
	else
	{
		autoX = 0;
	}

	if (area < objectThreshold)
	{
		// Gain/Lose Altitude
		if (posY < deadZoneMinY)
		{
			autoZ = ALT_SENSITIVITY;
		}
		else if (posY > deadZoneMaxY)
		{
			autoZ = -ALT_SENSITIVITY;
		}
		else
		{
			autoX = 0;
		}
		// Yaw Left/Right
		if (posX < 0 - DEAD_ZONE_X)
		{
			autoR = YAW_SENSITIVITY;
		}
		else if (posX > 0 + DEAD_ZONE_X)
		{
			autoR = -YAW_SENSITIVITY;
		}
		else
		{
			autoR = 0;
		}
	}
	else
	{
		autoZ = 0;
		autoR = 0;
	}

	controlThread.interrupt();
	controlThread = thread(controlStream);
}

void controls()
{
	// Take off / Landing Control
    if (KEY_PUSH(VK_SPACE)) 
	{
        if (ardrone.onGround())
		{
			ardrone.takeoff();
		}
        else
		{
			ardrone.landing();
		}
    }

    // Emergency stop
    if (KEY_PUSH(VK_BACK)) ardrone.emergency();
	// Flat Trim
	if (KEY_DOWN('F'))		ardrone.flatTrim();
	
	// Video Processing controls
	if (KEY_DOWN(VK_NUMPAD1))    hueMin--;    
	if (KEY_DOWN(VK_NUMPAD7))    hueMin++;    
	if (KEY_DOWN(VK_NUMPAD2))    satMin--;
	if (KEY_DOWN(VK_NUMPAD8))    satMin++;
	if (KEY_DOWN(VK_NUMPAD3))    valMin--;
	if (KEY_DOWN(VK_NUMPAD9))    valMin++;
	   
	if (KEY_DOWN(VK_INSERT))   hueMax++;
	if (KEY_DOWN(VK_DELETE))   hueMax--;     
	if (KEY_DOWN(VK_HOME))     satMax++;    
	if (KEY_DOWN(VK_END))      satMax--;    
	if (KEY_DOWN(VK_PRIOR))    valMax++;    
	if (KEY_DOWN(VK_NEXT))     valMax--;

	// Movement Controls
    double x = 0.0, y = 0.0, z = 0.0, r = 0.0;
	if (KEY_DOWN(VK_UP))    x =  1;
    if (KEY_DOWN(VK_DOWN))  x = -1;
    if (KEY_DOWN(VK_LEFT))  y =  1;
    if (KEY_DOWN(VK_RIGHT)) y = -1; 
    if (KEY_DOWN('W'))      z =  1;
    if (KEY_DOWN('S'))      z = -1;
    if (KEY_DOWN('A'))      r =  1;
    if (KEY_DOWN('D'))      r = -1;
    ardrone.move3D(x, y, z, r);
}

// Clear the user interface
void clearScreen()
{
	DWORD n;                         /* Number of characters written */
	DWORD size;                      /* number of visible characters */
	COORD coord = {0};               /* Top left screen position */
	CONSOLE_SCREEN_BUFFER_INFO csbi;
 
	/* Get a handle to the console */
	HANDLE h = GetStdHandle ( STD_OUTPUT_HANDLE );

	GetConsoleScreenBufferInfo ( h, &csbi );
 
	/* Find the number of characters to overwrite */
	size = csbi.dwSize.X * csbi.dwSize.Y;

	/* Overwrite the screen buffer with whitespace */
	FillConsoleOutputCharacter ( h, TEXT ( ' ' ), size, coord, &n );
	GetConsoleScreenBufferInfo ( h, &csbi );
	FillConsoleOutputAttribute ( h, csbi.wAttributes, size, coord, &n );

	/* Reset the cursor to the top left position */
	SetConsoleCursorPosition ( h, coord );
}

// Display the user interface
void display()
{
	clearScreen();
	printf("Pitch:\t = %f\n", autoX);
	printf("Roll:\t = %f\n", autoY);
	printf("Gaz:\t = %f\n", autoZ);
	printf("Yaw:\t = %f\n\n", autoR);
	
    printf("Battery\t = %d \n\n", battery);
    printf("Hue Min\t = %d \n", hueMin);
    printf("Hue Max\t = %d \n", hueMax);
    printf("Sat Min\t = %d \n", satMin);
    printf("Sat Max\t = %d \n", satMax);
    printf("Val Min\t = %d \n", valMin);
    printf("Val Max\t = %d \n\n", valMax);
	
	printf("PosX\t = %d \n", posX);
	printf("PosY\t = %d \n", posY);
	
}

// Threshold the HSV image and create a binary image
IplImage* GetThresholdedImage(IplImage* imgHSV)
{        
       IplImage* imgThresh=cvCreateImage(cvGetSize(imgHSV),IPL_DEPTH_8U, 1);
       cvInRangeS(imgHSV, cvScalar(hueMin,satMin,valMin), cvScalar(hueMax,satMax,valMax), imgThresh); 
       return imgThresh;
} 

// Main video processing loop
void videoProcessing()
{
	// Main loop
    while (!GetAsyncKeyState(VK_ESCAPE)) 
	{
        // Update your AR.Drone
        if (!ardrone.update()) break;

		controls();
		display();

        // Getting an image
        IplImage *image = ardrone.getImage();
        image = cvCloneImage(image); 
        cvSmooth(image, image, CV_GAUSSIAN,3,3); //smooth the original image using Gaussian kernel

		IplImage* imgHSV = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3); 
        cvCvtColor(image, imgHSV, CV_BGR2HSV); //Change the color format from BGR to HSV
        IplImage* imgThresh = GetThresholdedImage(imgHSV);

        // Display the image
        // cvShowImage("RGB Image", image);
        cvShowImage("Binary Image", imgThresh);
		
        //track the position of the ball
		trackColor(imgThresh);

        //Clean up used images
        cvReleaseImage(&imgHSV);
        cvReleaseImage(&imgThresh);
        cvReleaseImage(&image);

        // Press Esc to exit
        if (cvWaitKey(1) == 0x1b)
		{
			//controlThread.join();
			break;
		}
    }
}

int main(int argc, char **argv)
{
	deadZoneMinX = MIDX - DEAD_ZONE_X;
	deadZoneMaxX = MIDX + DEAD_ZONE_X;
	deadZoneMinY = MIDY - DEAD_ZONE_Y;
	deadZoneMaxY = MIDY + DEAD_ZONE_Y;

	videoProcessing();

    return 0;
}

