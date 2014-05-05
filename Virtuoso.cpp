#include "Demo.h"
#include "wiimote.h"
#include <mmsystem.h>	// for timeGetTime
#include <stdio.h>
#include <math.h>

#include <string>
#include <iostream>
#include <fstream>

#include <windows.h>
#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

#define PORT 1
#define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 


using namespace std;

void on_state_change (wiimote			  &remote,
					  state_change_flags  changed,
					  const wiimote_state &new_state){

	// the wiimote just connected
	if(changed & CONNECTED)
		{
		if(new_state.ExtensionType != wiimote::BALANCE_BOARD)
			{
			if(new_state.bExtension)
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT); // no IR dots
			else
				remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);		//    IR dots
			}
		}
	}
// ------------------------------------------------------------------------------------
void PrintTitle (HANDLE console){
	BRIGHT_WHITE;
	_tprintf(_T("\n")); 
	_tprintf(_T("   Team Sancho!    "));
	WHITE; _tprintf(_T("Virtuoso - Expo"));
	CYAN; _tprintf( _T("\n   May 1st, 2014\n"));
	CYAN;_tprintf(_T(" ______________________________________________________________________\n\n\n"));
	}
// ------------------------------------------------------------------------------------

void midiOn(RtMidiOut* midiout, int channel, int note){
		// Send out a series of MIDI messages.
		std::vector<unsigned char> message;
		  // Program change: 192, 5
		  message.push_back( 192 );
		  message.push_back( 5 );
		  midiout->sendMessage( &message );

		  // Note On: 145 or 146 (channel 2 or channel 3 for different instruments), the actual note note, 90 (velocity)
		  message[0] = (channel==2) ? 145 :146;
		  message[1] = note;
		  message[2] = 90; // Velocity
		  midiout->sendMessage( &message );
		 cout << "sending MIDI on note "<< note<< endl;
}

void midiOff(RtMidiOut* midiout, int note){
	// Send out a series of MIDI messages.
	std::vector<unsigned char> message;
	  // Program change: 192, 5
	  message.push_back( 192 );
	  message.push_back( 5 );
	  midiout->sendMessage( &message );
	  
	  // Note Off: 129 (sends to channel 2), the actual note, 40 (velocity)
	  message[0] = 129;
	  message[1] = note;
	  message[2] = 40;
	  midiout->sendMessage( &message );
	  
	  cout << "sending MIDI off note "<< note<< endl;
}

int getQuad( float X, float Y){
	// returns the current quandrant based on the coordinates given. not perfectly symmetrical due to comfortable user hand locations. 
	int row = 0;
	int col = 0;

	if (Y > .5){
		row = 0;
	}
	else if (Y > .3 && Y < .5){
		row = 1;
	}
	else if (Y < .3){
		row = 2;
	}
	
	if (X > .6){
		col = 0;
	}
	else if (X > .4 && X < .6){
		col = 1;
	}
	else if (X < .4){
		col = 2;
	}		

	return 3*row+col; // 
			
}

void determineQuadrant(wiimote_state::ir::dot* dots, int* quads, float positions[][2]){
	// parses wiimote dots and finds two brightest, which represent the two hands

	int index_Xmax; // Left hand
	int index_Xmin; // Right hand

	int i; // Keep in scope of function
	float thresholdC = 0.2; // "Radius" of change
	float thresholdD = 0.1; // "Radius" of degeneracy, hands must be this far away
	
	
	for(i = 0; i < 4; i++){ // Find first visible dot
		if(dots[i].bVisible){
			index_Xmin = i;
			index_Xmax = i;			
			break;
		}
	}
	
	if(i == 4) return; // No visible dots
	
	for(i; i < 4; i++){ // Continue through dots and find most right/left visible dots
		if(dots[i].bVisible){
			if(dots[i].X < dots[index_Xmin].X)
				index_Xmin = i;
			if(dots[i].X > dots[index_Xmax].X)
				index_Xmax = i;	
		}				
	}
	
	if(index_Xmax == index_Xmin){ // Only saw one dot (or multiple dots at exact same X), only update closest hand
		if(fabs(positions[0][0] - dots[index_Xmax].X) + fabs(positions[0][1] - dots[index_Xmax].Y) < fabs(positions[1][0] - dots[index_Xmin].X) + fabs(positions[1][1] - dots[index_Xmin].Y)){ // Left hand closer			
			quads[0] = getQuad(dots[index_Xmax].X, dots[index_Xmax].Y);
			positions[0][0] = dots[index_Xmax].X;
			positions[0][1] = dots[index_Xmax].Y;
		}
		else{ // Right hand closer
			quads[1] = getQuad(dots[index_Xmin].X, dots[index_Xmin].Y); 
			positions[1][0] = dots[index_Xmin].X;
			positions[1][1] = dots[index_Xmin].Y;
		}
		return;
	}
	
	if(fabs(dots[index_Xmax].X - dots[index_Xmin].X) + fabs(dots[index_Xmax].Y - dots[index_Xmin].Y) < thresholdD){ // Hands are very close together
		if(fabs(positions[0][0] - dots[index_Xmax].X) + fabs(positions[0][1] - dots[index_Xmax].Y) < fabs(positions[1][0] - dots[index_Xmin].X) + fabs(positions[1][1] - dots[index_Xmin].Y)){ // Left hand closer to maxX LED than right hand to minX LED
			quads[0] = getQuad(dots[index_Xmax].X, dots[index_Xmax].Y);
			positions[0][0] = dots[index_Xmax].X;
			positions[0][1] = dots[index_Xmax].Y;
		}
		else{ // Right hand closer to minX LED
			quads[1] = getQuad(dots[index_Xmax].X, dots[index_Xmax].Y);
			positions[1][0] = dots[index_Xmin].X;
			positions[1][1] = dots[index_Xmin].Y;
		}
		return;
	}
	
	if(fabs(positions[0][0] - dots[index_Xmax].X) + fabs(positions[0][1] - dots[index_Xmax].Y) < thresholdC){ // 1-Norm under a value, this is the correct dot for left hand
		quads[0] = getQuad(dots[index_Xmax].X, dots[index_Xmax].Y);
		positions[0][0] = dots[index_Xmax].X;
		positions[0][1] = dots[index_Xmax].Y;
	}
	
	if(fabs(positions[1][0] - dots[index_Xmin].X) + fabs(positions[1][1] - dots[index_Xmin].Y) < thresholdC){ // 1-Norm under a value, this is the correct dot for right hand
		quads[1] = getQuad(dots[index_Xmin].X, dots[index_Xmin].Y); 
		positions[1][0] = dots[index_Xmin].X;
		positions[1][1] = dots[index_Xmin].Y;	
	}	
	
}

void sendMIDISignal(int fingerState, int quad, RtMidiOut* midiout, int handOffset, bool left){
	// given a quandrant and a change in finger state, send a midi on or off signal
	// signals are uniquely determined based on quadrant and offset 

	int gridshift = 12;
	
	if (fingerState > 0){
		fingerState = fingerState/10; // remove extra digit for positive numbers that ensured correct parsing
			if (left){
				if (fingerState == 1)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 5); 
				else if (fingerState == 2)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 4); 
				else if (fingerState == 3)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 2); 
				else if (fingerState == 4)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 0); 
				else if (fingerState == 5)
					midiOn( midiout, 3, gridshift*(quad)+ handOffset + 0);
				else if (fingerState == 6)
					midiOn( midiout, 3, gridshift*(quad)+ handOffset + 1); 						
			}
			else{
				if (fingerState == 1)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 12); 
				else if (fingerState == 2)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 11); 
				else if (fingerState == 3)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 9); 
				else if (fingerState == 4)
					midiOn( midiout, 2, gridshift*(quad)+ handOffset + 7);
				else if (fingerState == 5)
					midiOn( midiout, 3, gridshift*(quad)+ handOffset + 2);
				else if (fingerState == 6)
					midiOn( midiout, 3, gridshift*(quad)+ handOffset + 3); 		
			}
	}
	else if (fingerState < 0){
		int absFinger = abs(fingerState); // remove negative
		if (left){
			if (absFinger == 1)
				midiOff( midiout, gridshift*(quad)+ handOffset + 5); 
			else if (absFinger == 2)
				midiOff( midiout, gridshift*(quad)+ handOffset + 4); 
			else if (absFinger == 3)
				midiOff( midiout, gridshift*(quad)+ handOffset + 2); 
			else if (absFinger == 4)
				midiOff( midiout, gridshift*(quad)+ handOffset + 0); 
		}
		else{
			if (absFinger == 1)
				midiOff( midiout, gridshift*(quad)+ handOffset + 12); 
			else if (absFinger == 2)
				midiOff( midiout, gridshift*(quad)+ handOffset + 11); 
			else if (absFinger == 3)
				midiOff( midiout, gridshift*(quad)+ handOffset + 9); 
			else if (absFinger == 4)
				midiOff( midiout, gridshift*(quad)+ handOffset + 7); 
		}
	}
}


HANDLE* setupBluetooth(const char* port){
	// create serial port that is used for communication with bluetooth devices. 
	
	HANDLE* hSerial = new HANDLE;
	*hSerial = CreateFile(port, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);
	// Must specify Windows serial port above, i.e. COM6

	if(*hSerial==INVALID_HANDLE_VALUE){
		if(GetLastError()==ERROR_FILE_NOT_FOUND){
			cout << "Does not exist!" << endl; //serial port does not exist. Inform user.
		}
		cout << "Strange error" << endl;
	}

	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if (!GetCommState(*hSerial, &dcbSerialParams)) {
		cout << "Can't get state" << endl;
	}

	dcbSerialParams.BaudRate=CBR_9600; // 9600 baud rate
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	if(!SetCommState(*hSerial, &dcbSerialParams)){
	 cout << "Can't set state" << endl;
	}

	// set timeouts correctly to make sure no data is missed or stalls

	COMMTIMEOUTS btStatus;
	GetCommTimeouts(*hSerial, &btStatus);
	btStatus.ReadIntervalTimeout= 10;
	btStatus.ReadTotalTimeoutMultiplier= 0;
	btStatus.ReadTotalTimeoutConstant= 10;
	SetCommTimeouts(*hSerial, &btStatus);
	
	return hSerial;
}

void readBluetooth(HANDLE &hSerial, char szBuff[], int quad, RtMidiOut* midiout, int offset, int* errorHandle, bool left){

		// continuosly read the buffer of signals

		DWORD dwBytesRead = 0;
		if(!ReadFile(hSerial, szBuff, 2, &dwBytesRead, NULL)){ 
			cout << "Can't read" << endl;
		}
		if (szBuff[0] != '0') {
		
		
			if(offset == 40 && errorHandle[0] == 1){
				sendMIDISignal(-atoi(szBuff)/10, quad, midiout, offset, left);
				errorHandle[0] = 0;
			}
			else if(offset != 40 && errorHandle[1] == 1){
				sendMIDISignal(-atoi(szBuff)/10, quad, midiout, offset, left);
				errorHandle[1] = 0;
			}
			else{
				sendMIDISignal(atoi(szBuff), quad, midiout, offset, left);
			}
			
			
			if(szBuff[0] == '-' && szBuff[1] == '0'){
				if(offset == 40)
					errorHandle[0] = 1;
				else
					errorHandle[1] = 1;
			
			}
		}
		
		szBuff[0] = '0'; // Reset input buffer
		szBuff[1] = '0';
}

int _tmain (int argc, char** argv)
	{
	
	///// SETUP FILE WRITE BETWEEN MATLAB /// 
	
	const char* datafile = "data.txt";
	ofstream outfile;
	outfile.open (datafile);
	
	//// Setup bluetooth
	
	const char* portLeft = "COM3";
	const char* portRight = "COM5";
	
	HANDLE* hSerialLeft = setupBluetooth(portLeft);
	HANDLE* hSerialRight = setupBluetooth(portRight);


	char szBuff[2 + 1] = {0};
	
	
	/////////////// SETUP MIDI ////////////////
	RtMidiOut *midiout = new RtMidiOut();


	// Check available ports.
	unsigned int nPorts = midiout->getPortCount();
	if ( nPorts == 0 ) {
		std::cout << "No ports available!\n";
	}

	// Open first available port.
	midiout->openPort( PORT );
	
	/////////////////////////////////////
	
	SetConsoleTitle(_T("Virtuoso - Expo"));
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	// write the title
	PrintTitle(console);

	// create a wiimote object
	wiimote remote;
	
	remote.ChangedCallback		= on_state_change;
	remote.CallbackTriggerFlags = (state_change_flags)(CONNECTED |
													   EXTENSION_CHANGED |
													   MOTIONPLUS_CHANGED);
reconnect:
	COORD pos = { 0, 6 };
	SetConsoleCursorPosition(console, pos);

	// try to connect the first available wiimote in the system
	//  (available means 'installed, and currently Bluetooth-connected'):
	WHITE; _tprintf(_T("  Looking for a Wiimote     "));
	   
	static const TCHAR* wait_str[] = { _T(".  "), _T(".. "), _T("...") };
	unsigned count = 0;
	while(!remote.Connect(wiimote::FIRST_AVAILABLE)) {
		_tprintf(_T("\b\b\b\b%s "), wait_str[count%3]);
		count++;
		}

	// connected - light all LEDs
	remote.SetLEDs(0x0f);
	BRIGHT_CYAN; _tprintf(_T("\b\b\b\b... connected!"));

	COORD cursor_pos = { 0, 6 };
	

	/////////////// Wiimote Variables ///////////////
	float positions [2][2];   // First array is left hand, second array is right. First element of array is x, second is y
	int quads[2]; // First element is left hand, second element is right
	quads[0] = 0;
	quads[1] = 0;
	
	int errorHandle[2];
	errorHandle[0] = 0;
	errorHandle[1] = 0;
	
	positions[0][0] = 0.67; // These give our starting points!!!!
	positions[0][1] = 0.5; 
	positions[1][0] = 0.33; 
	positions[1][1] = 0.5; 
	
	int writeTimerCount = 0;
	//////////////////////////////////////////////////
	
	
	// display the wiimote state data until 'Home' is pressed:
	while(!remote.Button.Home()){ // && !GetAsyncKeyState(VK_ESCAPE))

		while(remote.RefreshState() == NO_CHANGE)
			Sleep(1);

		cursor_pos.Y = 8;
		SetConsoleCursorPosition(console, cursor_pos);

		// did we loose the connection?
		if(remote.ConnectionLost()){
			BRIGHT_RED; _tprintf(
				_T("   *** connection lost! ***                                          \n")
				BLANK_LINE BLANK_LINE BLANK_LINE BLANK_LINE BLANK_LINE BLANK_LINE
				BLANK_LINE BLANK_LINE BLANK_LINE BLANK_LINE BLANK_LINE BLANK_LINE
				BLANK_LINE BLANK_LINE BLANK_LINE);
			Beep(100, 1000);
			Sleep(2000);
			COORD pos = { 0, 6 };
			SetConsoleCursorPosition(console, pos);
			_tprintf(BLANK_LINE BLANK_LINE BLANK_LINE);
			goto reconnect;
		}

		// Battery level:
		CYAN; _tprintf(_T("  Battery: "));
		(remote.bBatteryDrained	    )? BRIGHT_RED   :
		(remote.BatteryPercent >= 30)? BRIGHT_GREEN : BRIGHT_YELLOW;
		_tprintf(_T("%3u%%   "), remote.BatteryPercent);
			
		// IR:
		CYAN ; _tprintf(_T("\n       IR:"));
		_tprintf(_T("  Mode %s  "),
			((remote.IR.Mode == wiimote_state::ir::OFF     )? _T("OFF  ") :
			 (remote.IR.Mode == wiimote_state::ir::BASIC   )? _T("BASIC") :
			 (remote.IR.Mode == wiimote_state::ir::EXTENDED)? _T("EXT. ") :
															  _T("FULL ")));

		bool dot_sizes = (remote.IR.Mode == wiimote_state::ir::EXTENDED);

		for(unsigned index=0; index<4; index++){
			wiimote_state::ir::dot &dot = remote.IR.Dot[index];
			
			if(!remote.IsBalanceBoard()) WHITE;
			_tprintf(_T("%u: "), index);

			if(dot.bVisible) {
				WHITE; _tprintf(_T("Seen       "));
				}
			else{
				RED; _tprintf(_T("Not seen   "));
				}

			_tprintf(_T("Size"));
			if(dot_sizes)
				 _tprintf(_T("%3d "), dot.Size);
			else{
				RED; _tprintf(_T(" n/a"));
				if(dot.bVisible) WHITE;
			}

			_tprintf(_T("  X %.3f Y %.3f\n"), dot.X, dot.Y); 
			
			if(index < 3)
				_tprintf(_T("                        "));
		}
		BRIGHT_WHITE;
			
		// find the left and right hands 

		determineQuadrant(remote.IR.Dot, quads, positions);
	
		cout << "Right Hand Quad = " << quads[0] << endl;
		cout << "Left Hand Quad = " << quads[1] << endl;
		cout << "Right Hand - X = " << positions[0][0] << "   Y = " << positions[0][1] << endl;
		cout << "Left Hand - X = " << positions[1][0] << "   Y = " << positions[1][1] << endl;
		if (writeTimerCount == 3){
			outfile << positions[0][0] << " " << positions[0][1] << " " << "1" << endl; // write to file
			outfile << positions[1][0] << " " << positions[1][1] << " " << "0" << endl;
			writeTimerCount = 0;
		}
		else {
			writeTimerCount++;
		}

		
		/// Read Bluetooth and send MIDI signals /// 	
		readBluetooth(*hSerialRight, szBuff, quads[0], midiout, 36, errorHandle, false);
		readBluetooth(*hSerialLeft, szBuff, quads[1], midiout, 12, errorHandle, true);
		
  	}

  // when home button is pressed, discconect, delete shared data file, and close serial ports. 
	outfile.close();
	remove (datafile);
	// disconnect (auto-happens on wiimote destruction anyway, but let's play nice)
	remote.Disconnect();
	Beep(1000, 200);

	BRIGHT_WHITE; // for automatic 'press any key to continue' msg


	/////////////////////////////
	CloseHandle(*hSerialLeft);
	CloseHandle(*hSerialRight);

	CloseHandle(console);
	
	return 0;
}
