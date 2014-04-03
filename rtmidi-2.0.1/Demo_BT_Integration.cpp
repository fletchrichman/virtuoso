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

struct PositionHistory { // Basically a circular buffer
	int size; // Size of circular buffer
	int oldest_point; // Oldest element index of position circular buffer
	int velo_point; // Element index to compare with newest to get velocity
	
	int dis; // How far away newest point to compare point is (for abs velo measurement, i.e. delta t
	
	float *positionsX; // Circular buffer of positions (x)
	float *positionsY; // Circular buffer of positions (y)
};

PositionHistory* setupPosHistory(int size, int velo_pt){
	PositionHistory* ret = new PositionHistory;
	
	ret->size = size;
	ret->oldest_point = velo_pt;
	ret->velo_point = 0;	
	ret->dis = velo_pt;
	ret->positionsX = new float[size];
	ret->positionsY = new float[size];

	return ret;
}
	


// ------------------------------------------------------------------------------------
//  state-change callback example (we use polling for everything else):
// ------------------------------------------------------------------------------------
void on_state_change (wiimote			  &remote,
					  state_change_flags  changed,
					  const wiimote_state &new_state)
	{
	// we use this callback to set report types etc. to respond to key events
	//  (like the wiimote connecting or extensions (dis)connecting).
	
	// NOTE: don't access the public state from the 'remote' object here, as it will
	//		  be out-of-date (it's only updated via RefreshState() calls, and these
	//		  are reserved for the main application so it can be sure the values
	//		  stay consistent between calls).  Instead query 'new_state' only.

	// the wiimote just connected
	if(changed & CONNECTED)
		{
		// ask the wiimote to report everything (using the 'non-continous updates'
		//  default mode - updates will be frequent anyway due to the acceleration/IR
		//  values changing):

		// note1: you don't need to set a report type for Balance Boards - the
		//		   library does it automatically.
		
		// note2: for wiimotes, the report mode that includes the extension data
		//		   unfortunately only reports the 'BASIC' IR info (ie. no dot sizes),
		//		   so let's choose the best mode based on the extension status:
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
	_tprintf(_T("   Sancho!!!! "));
	WHITE; _tprintf(		   _T("Super Awesome Demo   "));
	CYAN; _tprintf(										    _T(" December 13th, 2013\n")
			 _T("                    v") WIIYOURSELF_VERSION_STR
										 _T("\n"));
	CYAN;_tprintf(_T(" ______________________________________________________________________\n\n\n"));
	}
// ------------------------------------------------------------------------------------

void midiOn(RtMidiOut* midiout, int note){
		// Send out a series of MIDI messages.
		std::vector<unsigned char> message;
		  // Program change: 192, 5
		  message.push_back( 192 );
		  message.push_back( 5 );
		  midiout->sendMessage( &message );

		  // Note On: 144, note, 90
		  message[0] = 144;
		  message[1] = note;
		  message[2] = 127;
		  midiout->sendMessage( &message );
		 
}

void midiOff(RtMidiOut* midiout, int note){

	// Send out a series of MIDI messages.
	std::vector<unsigned char> message;
	  // Program change: 192, 5
	  message.push_back( 192 );
	  message.push_back( 5 );
	  midiout->sendMessage( &message );

	  // Note Off: 128, 64, 40
	  message[0] = 128;
	  message[1] = note;
	  message[2] = 40;
	  midiout->sendMessage( &message );
		 
}

void determineQuadrant(wiimote_state::ir::dot* dots, int* quads, float positions[][2]){
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

	int row = 0;
	int col = 0;
	
	if(index_Xmax == index_Xmin){ // Only saw one dot (or multiple dots at exact same X), only update closest hand
		if(fabs(positions[0][0] - dots[index_Xmax].X) + fabs(positions[0][1] - dots[index_Xmax].Y) < fabs(positions[1][0] - dots[index_Xmin].X) + fabs(positions[1][1] - dots[index_Xmin].Y)){ // Left hand closer
			row = dots[index_Xmax].Y > 0.5 ? 0 : 1; // Only update left hand
			col = dots[index_Xmax].X < 0.5 ? 0 : 1;
			quads[0] = 2*row+col; 
			positions[0][0] = dots[index_Xmax].X;
			positions[0][1] = dots[index_Xmax].Y;
		}
		else{ // Right hand closer
			row = dots[index_Xmin].Y > 0.5 ? 0 : 1; // Only update right hand
			col = dots[index_Xmin].X < 0.5 ? 0 : 1;
			quads[1] = 2*row+col; 
			positions[1][0] = dots[index_Xmin].X;
			positions[1][1] = dots[index_Xmin].Y;
		}
		return;
	}
	
	if(fabs(dots[index_Xmax].X - dots[index_Xmin].X) + fabs(dots[index_Xmax].Y - dots[index_Xmin].Y) < thresholdD){ // Hands are very close together
		if(fabs(positions[0][0] - dots[index_Xmax].X) + fabs(positions[0][1] - dots[index_Xmax].Y) < fabs(positions[1][0] - dots[index_Xmin].X) + fabs(positions[1][1] - dots[index_Xmin].Y)){ // Left hand closer to maxX LED than right hand to minX LED
			row = dots[index_Xmax].Y > 0.5 ? 0 : 1; // Only update left hand
			col = dots[index_Xmax].X < 0.5 ? 0 : 1;
			quads[0] = 2*row+col; 
			positions[0][0] = dots[index_Xmax].X;
			positions[0][1] = dots[index_Xmax].Y;
		}
		else{ // Right hand closer to minX LED
			row = dots[index_Xmax].Y > 0.5 ? 0 : 1; // Only update right hand
			col = dots[index_Xmax].X < 0.5 ? 0 : 1;
			quads[1] = 2*row+col; 
			positions[1][0] = dots[index_Xmin].X;
			positions[1][1] = dots[index_Xmin].Y;
		}
		return;
	}
	
	
	if(fabs(positions[0][0] - dots[index_Xmax].X) + fabs(positions[0][1] - dots[index_Xmax].Y) < thresholdC){ // 1-Norm under a value, this is the correct dot for left hand
		row = dots[index_Xmax].Y > 0.5 ? 0 : 1;
		col = dots[index_Xmax].X < 0.5 ? 0 : 1;
		quads[0] = 2*row+col; 
		positions[0][0] = dots[index_Xmax].X;
		positions[0][1] = dots[index_Xmax].Y;
	}
	
	if(fabs(positions[1][0] - dots[index_Xmin].X) + fabs(positions[1][1] - dots[index_Xmin].Y) < thresholdC){ // 1-Norm under a value, this is the correct dot for right hand
		row = dots[index_Xmin].Y > 0.5 ? 0 : 1;
		col = dots[index_Xmin].X < 0.5 ? 0 : 1;
		quads[1] = 2*row+col; 
		positions[1][0] = dots[index_Xmin].X;
		positions[1][1] = dots[index_Xmin].Y;	
	}	
}

void sendMIDISignal(int fingerState, int quad, RtMidiOut* midiout, int offset){
	//system("cls");
	int gridsize = 2;
	//cout << fingerState << endl;
	if (fingerState > 0){
		fingerState = fingerState/10;
		//cout << "POS  " << fingerState << endl;
		//cout << (fingerState-1)*gridsize+(quad-1) << endl;
		midiOn( midiout, (fingerState-1)*gridsize+(quad-1) + offset); 
	}
	else if (fingerState < 0){
		//cout << "NEG                    " << fingerState << endl;
		int absFinger = abs(fingerState);
		for (int i = 1; i <= gridsize; i++){

			//cout << "\t" << (absFinger-1)*gridsize+(i-1);
			midiOff( midiout, (absFinger-1)*gridsize+(i-1) + offset) ;
		}
		//cout << endl;
	}
}

int _tmain (int argc, char** argv)
	{
	
	FreeConsole();
	AllocConsole();
	freopen( "CONOUT$", "wb", stdout);


	///// SETUP FILE WRITE
	
	ofstream outfile;
	outfile.open ("data12.txt");
	
	
	
	//////////////////SETUP BLUETOOTH//////////
	
	HANDLE hSerial = CreateFile("COM6", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);
	// Must specify Windows serial port above, i.e. COM6

	if(hSerial==INVALID_HANDLE_VALUE){
		if(GetLastError()==ERROR_FILE_NOT_FOUND){
			cout << "Does not exist!" << endl; //serial port does not exist. Inform user.
		}
		cout << "Strange error" << endl;
	}

	DCB dcbSerialParams = {0};
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		cout << "Can't get state" << endl;
	}

	dcbSerialParams.BaudRate=CBR_9600; // I didn't expect this number but it works
	dcbSerialParams.ByteSize=8;
	dcbSerialParams.StopBits=ONESTOPBIT;
	dcbSerialParams.Parity=NOPARITY;
	if(!SetCommState(hSerial, &dcbSerialParams)){
	 cout << "Can't set state" << endl;
	}

	
	COMMTIMEOUTS btStatus;
	GetCommTimeouts(hSerial, &btStatus);
	btStatus.ReadIntervalTimeout= 10;
	btStatus.ReadTotalTimeoutMultiplier= 0;
	btStatus.ReadTotalTimeoutConstant= 10;
	SetCommTimeouts(hSerial, &btStatus);
	
	char szBuff[2 + 1] = {0}; // Not sure about the second 1
	DWORD dwBytesRead = 0;

	//////////////////////////////////////////
	
	/////////////// SETUP MIDI ////////////////
	RtMidiOut *midiout = new RtMidiOut();


	// Check available ports.
	unsigned int nPorts = midiout->getPortCount();
	if ( nPorts == 0 ) {
		std::cout << "No ports available!\n";
		//goto cleanup;
	}

	// Open first available port.
	midiout->openPort( PORT );
	
	/////////////////////////////////////
	
	
	
	SetConsoleTitle(_T("- Sancho!!!! - Demo: "));
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	// write the title
	PrintTitle(console);

	// create a wiimote object
	wiimote remote;
	
	// in this demo we use a state-change callback to get notified of
	//  extension-related events, and polling for everything else
	// (note you don't have to use both, use whatever suits your app):
	remote.ChangedCallback		= on_state_change;
	//  notify us only when the wiimote connected sucessfully, or something
	//   related to extensions changes
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
	
	
	/////////////// MIDI Variables ///////////////
	int fingerState; // What we receive from the Arduino

	/////////////// Wiimote Variables ///////////////
	float positions [2][2];   // First array is left hand, second array is right. First element of array is x, second is y
	int quads[2]; // First element is left hand, second element is right
	
	PositionHistory* posHistoryL = setupPosHistory(1000, 30); // Left hand
	PositionHistory* posHistoryR = setupPosHistory(1000, 30); // Left hand
	
	positions[0][0] = 0.8; // These give our starting points!!!!
	positions[0][1] = 0.61; // Very important
	positions[1][0] = 0.53; // Must calibrate these
	positions[1][1] = 0.63; // Don't forget!!
	
	//////////////////////////////////////////////////
	
	
	// display the wiimote state data until 'Home' is pressed:
	while(!remote.Button.Home()){ // && !GetAsyncKeyState(VK_ESCAPE))
		// IMPORTANT: the wiimote state needs to be refreshed each pass
		while(remote.RefreshState() == NO_CHANGE)
			Sleep(1); // // don't hog the CPU if nothing changed

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
		// (the green/yellow colour ranges are rough guesses - my wiimote
		//  with rechargeable battery pack fails at around 15%)
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
		// IR dot sizes are only reported in EXTENDED IR mode (FULL isn't supported yet)
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

			//_tprintf(_T("  X %.3f Y %.3f\n"), 133.9*(dot.X-0.470),  88.7*(dot.Y-0.575)); // Changed stuff here!
			_tprintf(_T("  X %.3f Y %.3f\n"), dot.X, dot.Y); 
			
			if(index < 3)
				_tprintf(_T("                        "));
		}
		BRIGHT_WHITE;
			
		determineQuadrant(remote.IR.Dot, quads, positions);
		
		//cout << quads[0] << endl;
		//cout << quads[1] << endl;
		cout << "Left Hand - X = " << positions[0][0] << "   Y = " << positions[0][1] << endl;
		cout << "Right Hand - X = " << positions[1][0] << "   Y = " << positions[1][1] << endl;
		
		outfile << positions[0][0] << " " << positions[0][1] << " " << "0" << endl; // write to file
		outfile << positions[1][0] << " " << positions[1][1] << " " << "1" << endl;
		
		
		posHistoryL->oldest_point = (posHistoryL->oldest_point + 1) % posHistoryL->size; // Move circular buffer forward
		posHistoryL->velo_point = (posHistoryL->velo_point + 1) % posHistoryL->size; // Move circular buffer forward
		
		posHistoryL->positionsX[posHistoryL->oldest_point] = positions[0][0]; // Update left hand X position
		posHistoryL->positionsY[posHistoryL->oldest_point] = positions[0][1]; // Update left hand Y position
		
		cout << (posHistoryL->positionsX[posHistoryL->oldest_point] - posHistoryL->positionsX[posHistoryL->velo_point])/posHistoryL->dis << endl; // Print left hand x velocity

		cout << posHistoryL->oldest_point << endl; // For debugging
		
		
			
			
		if(!ReadFile(hSerial, szBuff, 2, &dwBytesRead, NULL)){ 
			cout << "Can't read" << endl;
		}
		if (szBuff[0] != '0') 
			cout << szBuff << endl; // print read data

		fingerState = atoi(szBuff);
		sendMIDISignal(fingerState, quads[0], midiout, 40);
		
		szBuff[0] = '0'; // Reset input buffer
		szBuff[1] = '0';
  	}

	outfile.close();
	// disconnect (auto-happens on wiimote destruction anyway, but let's play nice)
	remote.Disconnect();
	Beep(1000, 200);

	BRIGHT_WHITE; // for automatic 'press any key to continue' msg
	CloseHandle(console);
	
	
	
	
	/////////////////////////////
	CloseHandle(hSerial);

	// Clean up
	//cleanup:
	//delete midiout;
	
	return 0;
	}
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
