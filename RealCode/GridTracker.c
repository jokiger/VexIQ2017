/*
* Grid Tracker Begin
*/

/*Setup*/


/*
 *This setup code must be added to robot code before
 *including GridTracker.c
 */
/*
#define GRID_WIDTH 4
#define GRID_LENGTH 8
#define GRID_THRESHHOLD_FOLLOWER 100
#define GRID_THRESHHOLD_DETECTOR 100
#define GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER  centercolor
#define GRID_PORT_COLOR_SENSOR_LINE_DETECTOR  rightcolor
#define GRID_PORT_GYRO  gyrosensor
#define GRID_GYRO_THRESHOLD 2
#define GRID_MOTOR_RIGHT rs
#define GRID_MOTOR_LEFT ls
#define GRID_MOTOR_TRAVEL_PER_TURN_IN_MM 200
#define GRID_TURN_TO_LINE_DEGREES 45
#define GRID_TIMER T2
int GRID_SET_DIRECTION_TIMEOUT = 0;
int GRID_MOVE_TIMEOUT = 0;
int GRID_MOVE_AFTER_LINE = 100;

int GRID_SPEED_PRIMARY = 75;
int GRID_SPEED_SECONDARY = 60;
int GRID_TURN_SPEED_FAST = 50;
int GRID_TURN_SPEED_SLOW = 0;
int GRID_TURN_TO_LINE = 10;
bool GRID_DEBUG = true;
*/
/*Setup End*/

/*Local variables*/
int GridX = 0;
int GridY = 0;
int TargetX = 1;
int TargetY = 1;
int TargetDir = 0;
bool gridPause=true;
bool bSetDirectionDone = false;
bool bSetDirectionTimeout = false;
bool bMoveDone = false;
bool bMoveTimeout = false;
#define GRID_DIR_NORTH 0
#define GRID_DIR_EAST  270
#define GRID_DIR_SOUTH  180
#define GRID_DIR_WEST 90

int GridDirection = GRID_DIR_NORTH;
string sGridStatus = "";
bool bLineDetected = false;
//#define GRID_NO_MOVE
task GridSetDirectionTimeoutTask()
{
	clearTimer(GRID_TIMER);
	while(!bSetDirectionDone && !bSetDirectionTimeout)
	{
		if(time1[GRID_TIMER] > GRID_SET_DIRECTION_TIMEOUT)
		{
			bSetDirectionTimeout=true;
		}
		sleep(50);
  }
}
void GridStopAllMotors()
{
	stopMotor(GRID_MOTOR_RIGHT);
	stopMotor(GRID_MOTOR_LEFT);
}
void GridSetLocation(int x,int y)
{
	GridX = x;
	GridY = y;
}
void GridMoveForward(int mm)
{
	int Rotation = (((float)mm/(float)GRID_MOTOR_TRAVEL_PER_TURN_IN_MM) * 360);
	bMoveDone=false;
	bMoveTimeout=false;
	clearTimer(GRID_TIMER);
	moveMotorTarget(GRID_MOTOR_RIGHT,Rotation,GRID_SPEED_PRIMARY);
	moveMotorTarget(GRID_MOTOR_LEFT,Rotation,GRID_SPEED_PRIMARY);
	sleep(50);
	while(!getMotorZeroVelocity(GRID_MOTOR_RIGHT) && !getMotorZeroVelocity(GRID_MOTOR_RIGHT) && !bMoveTimeout)
	{
		 if(GRID_MOVE_TIMEOUT && time1[GRID_TIMER] > GRID_MOVE_TIMEOUT)
		 {
		   stopMotor(GRID_MOTOR_RIGHT);
       stopMotor(GRID_MOTOR_LEFT);
		   bMoveTimeout=true;
	   }
	   sleep(1);
  }
	bMoveDone=true;
}

void GridMoveBackward(int mm)
{
	int Rotation = (((float)mm/(float)GRID_MOTOR_TRAVEL_PER_TURN_IN_MM) * 360);
	bMoveDone=false;
	bMoveTimeout=false;
	clearTimer(GRID_TIMER);
	moveMotorTarget(GRID_MOTOR_RIGHT,Rotation*-1,GRID_SPEED_PRIMARY);
	moveMotorTarget(GRID_MOTOR_LEFT,Rotation*-1,GRID_SPEED_PRIMARY);
	sleep(50);
	while(!getMotorZeroVelocity(GRID_MOTOR_RIGHT) && !getMotorZeroVelocity(GRID_MOTOR_RIGHT) && !bMoveTimeout)
	{
		 if(GRID_MOVE_TIMEOUT && time1[GRID_TIMER] > GRID_MOVE_TIMEOUT)
		 {
		   stopMotor(GRID_MOTOR_RIGHT);
       stopMotor(GRID_MOTOR_LEFT);
		   bMoveTimeout=true;
	   }
	   sleep(1);
  }
	bMoveDone=true;
}
void GridStatus(const char * s)
{
	sGridStatus = s;
}
int GridGetGyroDegrees()
{
	int Degrees = getGyroDegrees(GRID_PORT_GYRO);
	if(Degrees < 0)
	{
		while(Degrees < 0)
			Degrees += 360;
	}
	if(Degrees > 360)
	{
		while(Degrees > 360)
			Degrees -= 360;
	}
	return Degrees;
}
int GridGetDistance(int currentposition,int targetposition)
{
	int Distance;
	if(currentposition-targetposition < -180)
	{
		Distance=(currentposition-targetposition+180) * -1;
	}
	else
	{
		Distance=currentposition-targetposition;
	}
	return Distance;
}
void GridUpdateStatus()
{
	if(GRID_DEBUG)
	{
		displayTextLine(line1,"%d:%d %d:%d Dir:%d:%d:%d",GridX,GridY,TargetX,TargetY,GridGetGyroDegrees(),TargetDir,GridDirection);
	//	displayTextLine(line2,"%d:%d  %d:%d",getDistanceValue(distLeft),getDistanceValue(distRight),getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER),getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR));
		displayTextLine(line2,"%d:%d",getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER),getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR));
		displayText(line3,sGridStatus);
	}
}
task Display()
{
	repeat (forever) {
		GridUpdateStatus();
		sleep(100);
	}

}
void GridInit()
{
	if(GRID_DEBUG)
	{
	   startTask(Display);
  }
	GridStatus("GridInit");
	short count = 200;
	startGyroCalibration( GRID_PORT_GYRO, gyroCalibrateSamples64 );

	// delay so calibrate flag can be set internally to the gyro
	wait1Msec(100);

	/* Initialize Color Sensors while gyro is calibrating*/
	if(getColorMode(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER) != colorTypeGrayscale_Reflected)
	{
		setColorMode(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER, colorTypeGrayscale_Reflected);
		while(!getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER))
		{
			sleep(25);
		}
	}
	return;
	if(getColorMode(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR) != colorTypeGrayscale_Reflected)
	{
		setColorMode(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR, colorTypeGrayscale_Reflected);
		while(!getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR))
		{
			sleep(25);
		}
	}
	// wait for calibration to finish or 2 seconds, whichever is longer
	while( getGyroCalibrationFlag(GRID_PORT_GYRO) && (count-- > 0) ) {
		char Status[64];
		sprintf(Status,"GridInit Testing Gyro %d",count);
		GridStatus(Status);
		wait1Msec(100);
	}
	// reset so this is 0 heading
	resetGyro(GRID_PORT_GYRO);
	GridStatus("GridInit Complete");
	sleep(1000);
}


void GridPause()
{
	GridStatus("Pause");
	gridPause = true;
}
void GridResume()
{
	GridStatus("GridResume");
	gridPause = false;
}
void GridGoto(int x,int y)
{
	GridStatus("Goto");
	TargetX=x;
	TargetY=y;
	GridResume();
}


void GridSetDirection(int Direction)
{
	GridStatus("GridSetDirection");
	bSetDirectionTimeout=false;
	bSetDirectionDone=false;
	updateMotorDriveTrain();
	TargetDir=Direction;
	if(GRID_SET_DIRECTION_TIMEOUT)
	{
  	startTask(GridSetDirectionTimeoutTask);
  }

	while(abs(GridGetGyroDegrees()-Direction) > GRID_GYRO_THRESHOLD && !bSetDirectionTimeout)
	{
		int Distance = GridGetDistance(GridGetGyroDegrees(),Direction);
		if(Distance > 0 && Distance <= 180)
		{
			//Right
#ifndef GRID_NO_MOVE
			setMotorSpeeds(GRID_TURN_SPEED_FAST,GRID_TURN_SPEED_SLOW);
#endif
		}
		else
		{
			//Left
#ifndef GRID_NO_MOVE
			setMotorSpeeds(GRID_TURN_SPEED_SLOW,GRID_TURN_SPEED_FAST);
#endif
		}
	}
#ifndef GRID_NO_MOVE
	GridStopAllMotors();
#endif
	GridDirection = Direction;
	bSetDirectionDone = true;
}

void GridTurnToLine()
{
	bool bFound = false;
	bool bOnLine = false;
	bool bReverse=false;
	updateMotorDriveTrain();
#ifndef	GRID_NO_MOVE
	setMotorSpeeds(GRID_TURN_TO_LINE, GRID_TURN_TO_LINE*-1);
#endif
	GridStatus("GridToLine Right");
	/*Find the right edge, when turning right wait until we cross the whole line*/
	int leftLimit = GridDirection+45;
	int rightLimit;
	if(GridDirection-GRID_TURN_TO_LINE_DEGREES+360 >  360)
		rightLimit=GridDirection-GRID_TURN_TO_LINE_DEGREES;
	else
		rightLimit=GridDirection-GRID_TURN_TO_LINE_DEGREES+360;

	/*Get past zero if going to the right */
	if(rightLimit == 360-GRID_TURN_TO_LINE_DEGREES )
	{
		while(GridGetGyroDegrees() < 340)
		{
			sleep(10);
		}
	}
	while(!bFound && !bReverse)
	{
		setTouchLEDColor(ledfront,colorRed);
		if(getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER) < GRID_THRESHHOLD_DETECTOR)
		{
			bOnLine = true;
		}
		else if(bOnLine)
		{
			bFound = true;
#ifndef GRID_NO_MOVE
			GridStopAllMotors();
#endif
		}
		if(!bOnLine)
		{
			if(GridGetGyroDegrees() < rightLimit)
			{
#ifndef GRID_NO_MOVE
				GridStopAllMotors();
#endif
				bReverse=true;
			}
		}
	}

	if(!bFound)
	{
		GridStatus("GridToLine Left");
		bReverse=false;
#ifndef GRID_NO_MOVE
		setMotorSpeeds(GRID_TURN_TO_LINE * -1, GRID_TURN_TO_LINE);
#endif
		GridStatus("GridTurnToLine Left Restting to straight");
		while(abs(GridGetGyroDegrees()-GridDirection) > 3)
		{
			sleep(25);
		}

		/*Get past zero if going to the left */
		if(rightLimit == GRID_TURN_TO_LINE_DEGREES)
		{
			while(GridGetGyroDegrees() > 340)
			{
				sleep(10);
			}
		}
		GridStatus("GridTurnToLine Left");
		while(!bFound && !bReverse)
		{
		setTouchLEDColor(ledfront,colorGreen);
			if(getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER) < GRID_THRESHHOLD_DETECTOR)
			{
				bFound = true;
			}
			if(GridGetGyroDegrees() > leftLimit)
			{
				bReverse=true;
			}
		}
	}
#ifndef GRID_NO_MOVE
	GridStopAllMotors();
#endif
	GridStatus("GridTurnToLine Done");
	if(!bFound)
	{
		playSound(soundCarAlarm4);
		sleep(2000);
	}
}

void GridProcess()
{
	updateMotorDriveTrain();

	if((GridX != TargetX || GridY != TargetY) && !gridPause)
	{
		if(GridY != TargetY)
		{
			if(GridY < TargetY)
			{
				if(GridDirection != GRID_DIR_NORTH)
				{
					GridStatus("Turning North");
					GridSetDirection(GRID_DIR_NORTH);
					GridTurnToLine();
				}
			}
			else
			{
				GridStatus("Turning South");
				if(GridDirection != GRID_DIR_SOUTH)
				{
					GridSetDirection(GRID_DIR_SOUTH);
					GridTurnToLine();
				}
			}
		}
		else if(GridX != TargetX)
		{
			if(GridX < TargetX)
			{
				GridStatus("Turning East");
				if(GridDirection != GRID_DIR_EAST)
				{
					GridSetDirection(GRID_DIR_EAST);
					GridTurnToLine();
				}
			}
			else
			{
				GridStatus("Turning West");
				if(GridDirection != GRID_DIR_WEST)
				{
					GridSetDirection(GRID_DIR_WEST);
					GridTurnToLine();
				}
			}
		}
		if(getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_FOLLOWER) > GRID_THRESHHOLD_FOLLOWER)
		{
			GridStatus("Off the line");
			setTouchLEDColor(ledback,colorRed);
#ifndef GRID_NO_MOVE
			setMotorSpeeds(GRID_SPEED_SECONDARY, GRID_SPEED_PRIMARY);
#endif
		}
		else
		{
			GridStatus("On the line");
			setTouchLEDColor(ledback,colorGreen);
#ifndef GRID_NO_MOVE
			setMotorSpeeds(GRID_SPEED_PRIMARY, GRID_SPEED_SECONDARY);
#endif
		}

		if(getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR) < GRID_THRESHHOLD_DETECTOR && bLineDetected == false)
		{
			GridStatus("Line Detected");
			//playSound(soundTollBooth);
			if(GridDirection == GRID_DIR_NORTH)
				GridY++;
			else if(GridDirection == GRID_DIR_SOUTH)
				GridY--;
			else if(GridDirection == GRID_DIR_EAST)
				GridX++;
			else if(GridDirection == GRID_DIR_WEST)
				GridX--;

			bLineDetected = true;
		}
		if(getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR) > GRID_THRESHHOLD_DETECTOR)
		{
			bLineDetected = false;
		}

	}
	else
	{
		/*Reached the target*/
		if(!gridPause)
		{
			GridMoveForward(GRID_MOVE_AFTER_LINE);
#ifndef GRID_NO_MOVE
			GridStopAllMotors();
#endif
			GridPause();
		}

	}
}

void GridFindLine(bool bFarSide)
{
	bool bFound = false;
	GridStatus("Finding Line");
	updateMotorDriveTrain();
#ifndef GRID_NO_MOVE
	setMotorSpeeds(GRID_SPEED_PRIMARY, GRID_SPEED_PRIMARY);
#endif
	while(!bFound)
	{
		if(getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR) < GRID_THRESHHOLD_DETECTOR)
		{
			bFound = true;
		}
	}
	if(bFarSide)
	{
		bFound=false;
		while(!bFound)
		{
			if(getColorGrayscale(GRID_PORT_COLOR_SENSOR_LINE_DETECTOR) < GRID_THRESHHOLD_DETECTOR)
			{
				bFound = true;
			}
		}
	}
#ifndef GRID_NO_MOVE
	GridStopAllMotors();
#endif
}



/*
* Grid Tracker End
*/
