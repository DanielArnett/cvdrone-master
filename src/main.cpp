#include "ardrone/ardrone.h"
#include "samples/Xbox Controller/XboxController.h"
#define KEY_DOWN(key) (GetAsyncKeyState(key) & 0x8000)
#define KEY_PUSH(key) (GetAsyncKeyState(key) & 0x0001)



// AR.Drone class
ARDrone ardrone("192.168.1.112");
XboxController* player1;

const double STICK_RANGE = 32767;
const double TRIGGER_RANGE = 255;
const double STICK_DEAD_ZONE = 0.05;

// --------------------------------------------------------------------------
// clearScreen()
// This function clears the console display.
// --------------------------------------------------------------------------
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

// --------------------------------------------------------------------------
// controls()
// This function displays sensory data from the drone to the console.
// --------------------------------------------------------------------------
void display()
{
		clearScreen();
        // Orientation
        double roll  = ardrone.getRoll();
        double pitch = ardrone.getPitch();
        double yaw   = ardrone.getYaw();
        printf("Roll \t\t = %3.2f [deg]\n", roll  * RAD_TO_DEG);
        printf("Pitch\t\t = %3.2f [deg]\n", pitch * RAD_TO_DEG);
        printf("Yaw  \t\t = %3.2f [deg]\n", yaw   * RAD_TO_DEG);

        // Altitude
        double altitude = ardrone.getAltitude();
        printf("Altitude\t = %3.2f [m]\n", altitude);

        // Velocity
        double vx, vy, vz;
        double velocity = ardrone.getVelocity(&vx, &vy, &vz);
        printf("X Velocity\t = %3.2f [m/s]\n", vx);
        printf("Y Velocity\t = %3.2f [m/s]\n", vy);
        printf("Z Velocity\t = %3.2f [m/s]\n", vz);

        // Battery
        int battery = ardrone.getBatteryPercentage();
        printf("Battery\t\t = %d [%%]\n", battery);
}

// --------------------------------------------------------------------------
// controls()
// This function sends commands to the drone using an Xbox Controller.
// --------------------------------------------------------------------------
void controls()
{
	// Check for Xbox Controller
	if(player1->IsConnected())
	{
		// 'A' button
		if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_A)
		{
			// Change camera
			static int cameraToggle = 0;
			ardrone.setCamera(++cameraToggle%4);
		}
		// 'B' button
		if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_B)
		{
			ardrone.emergency();
		}
		// 'X' button
		if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_X)
		{
			ardrone.flatTrim();	
		}
		// 'Y' button
		/*if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_Y)
		{

		}*/
		// 'Start' button
		if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_START)
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
		// 'Back' button
		if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
		{
			// Exit the program.
			if (ardrone.onGround())
			{
				exit(0);
			}
		}
		// Left Thumb-Stick Press
		/*if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
		{

		}*/
		// Right Thumb-Stick Press
		/*if(player1->GetState().Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		{

		}*/

		// AR.Drone flight controls
		//if (!ardrone.onGround()) 
		{
			// Check left thumbStick
			double leftThumbY = player1->GetState().Gamepad.sThumbLY / STICK_RANGE;
			double leftThumbX = player1->GetState().Gamepad.sThumbLX / STICK_RANGE;
			// Check the dead zone
			if (leftThumbY < STICK_DEAD_ZONE && leftThumbY > -STICK_DEAD_ZONE)
			{
				leftThumbY = 0;
			}
			if (leftThumbX < STICK_DEAD_ZONE && leftThumbX > -STICK_DEAD_ZONE)
			{
				leftThumbX = 0;
			}

			// Check left thumbStick
			double rightThumbY = player1->GetState().Gamepad.sThumbRY / TRIGGER_RANGE;
			double rightThumbX = player1->GetState().Gamepad.sThumbRX / TRIGGER_RANGE;
			// Check the dead zone
			if (rightThumbY < STICK_DEAD_ZONE && rightThumbY > -STICK_DEAD_ZONE)
			{
				rightThumbY = 0;
			}
			if (rightThumbX < STICK_DEAD_ZONE && rightThumbX> -STICK_DEAD_ZONE)
			{
				rightThumbX = 0;
			}
			
			double rightTrigger = player1->GetState().Gamepad.bRightTrigger / TRIGGER_RANGE;
			double leftTrigger = player1->GetState().Gamepad.bLeftTrigger / TRIGGER_RANGE;
			double gaz = rightTrigger - leftTrigger;
			
			// Move
			ardrone.move3D(rightThumbY*10, -rightThumbX*10, gaz*10, -leftThumbX*10);
				
		}
	}
}
// --------------------------------------------------------------------------
// main(Number of arguments, Value of arguments)
// This is the main function.
// Return value Success:0 Error:-1
// --------------------------------------------------------------------------
int main(int argc, char **argv)
{
	player1 = new XboxController(1);

    // Initialize
    /*if (!ardrone.open()) 
	{
        printf("Failed to find the AR.Drone. Please ensure that the drone is connected via WiFi.\n");
		system("Pause");
        return -1;
    }*/
	
    // Update your AR.Drone
    /*if (!ardrone.update())
	{
		printf("This AR.Drone is not up to date, please update the AR.Drone before use.\n");
		system("Pause");
        return -1;
	}*/
    // Main loop
    while (!GetAsyncKeyState(VK_ESCAPE)) 
	{
        // Get an image
        IplImage *image = ardrone.getImage();

		controls();
		display();

        // Display the image
        cvShowImage("camera", image);
        cvWaitKey(1);
    }

    ardrone.close();
	
    return 0;
}