#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include "RtMidi.h"

#if defined(__WINDOWS_MM__)
  #include <windows.h>
  #define PORT 1
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define PORT 0
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

#include "wiimote.h"
#include <mmsystem.h>	// for timeGetTime


using namespace std;

void on_state_change (wiimote &remote, state_change_flags  changed, const wiimote_state &new_state){

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
	// a MotionPlus was detected
	if(changed & MOTIONPLUS_DETECTED)
		{

		if(remote.ExtensionType == wiimote_state::NONE) {
			bool res = remote.EnableMotionPlus();
			_ASSERT(res);
			}
		}
	// an extension is connected to the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_CONNECTED)
		{
		// We can't read it if the MotionPlus is currently enabled, so disable it:
		if(remote.MotionPlusEnabled())
			remote.DisableMotionPlus();
		}
	// an extension disconnected from the MotionPlus
	else if(changed & MOTIONPLUS_EXTENSION_DISCONNECTED)
		{
		// enable the MotionPlus data again:
		if(remote.MotionPlusConnected())
			remote.EnableMotionPlus();
		}
	// another extension was just connected:
	else if(changed & EXTENSION_CONNECTED)
		{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(1000, 200);
#endif
		// switch to a report mode that includes the extension data (we will
		//  loose the IR dot sizes)
		// note: there is no need to set report types for a Balance Board.
		if(!remote.IsBalanceBoard())
			remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR_EXT);
		}
	// extension was just disconnected:
	else if(changed & EXTENSION_DISCONNECTED)
		{
#ifdef USE_BEEPS_AND_DELAYS
		Beep(200, 300);
#endif
		// use a non-extension report mode (this gives us back the IR dot sizes)
		remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);
		}
	}
	
void playmidi(RtMidiOut* midiout, int note){
		// Send out a series of MIDI messages.
		std::vector<unsigned char> message;
		  // Program change: 192, 5
		  message.push_back( 192 );
		  message.push_back( 5 );
		  midiout->sendMessage( &message );

		  // Note On: 144, 64, 90
		  message[0] = 144;
		  message[1] = note;
		  message[2] = 90;
		  midiout->sendMessage( &message );

		  SLEEP( 500 ); // Platform-dependent ... see example in tests directory.

		  // Note Off: 128, 64, 40
		  message[0] = 128;
		  message[1] = note;
		  message[2] = 40;
		  midiout->sendMessage( &message );
		 
}

int main(){


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


	static const TCHAR* wait_str[] = { _T(".  "), _T(".. "), _T("...") };
	unsigned count = 0;
	while(!remote.Connect(wiimote::FIRST_AVAILABLE)) {
	_tprintf(_T("\b\b\b\b%s "), wait_str[count%3]);
	count++;
	}

	remote.SetLEDs(0x0f);
	remote.SetReportType(wiimote::IN_BUTTONS_ACCEL_IR);
	while(1){
		wiimote_state::ir::dot &dot = remote.IR.Dot[0];
		cout << dot.X << endl;
		cout << dot.Y << endl;
	}


	HANDLE hSerial = CreateFile("COM8", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);
	// Must specify Windows serial port above, i.e. COM6

	if(hSerial==INVALID_HANDLE_VALUE){
		if(GetLastError()==ERROR_FILE_NOT_FOUND){
			cout << "Does not exist" << endl; //serial port does not exist. Inform user.
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

	char szBuff[1 + 1] = {0}; // First 1 for number of bytes, not sure about the second 1
	DWORD dwBytesRead = 0;

	char maxVal = '0';


	RtMidiOut *midiout = new RtMidiOut();


	// Check available ports.
	unsigned int nPorts = midiout->getPortCount();
	if ( nPorts == 0 ) {
	std::cout << "No ports available!\n";
	goto cleanup;
	}

	// Open first available port.
	midiout->openPort( PORT );
		

	while(1){
		if(!ReadFile(hSerial, szBuff, 1, &dwBytesRead, NULL)){ // 1 for number of bytes
			cout << "Can't read" << endl;
		}

		cout << szBuff << endl; // print read data
		
		if(szBuff[0] > maxVal) maxVal = szBuff[0];
		
		if(szBuff[0] == '0'){
			if(maxVal == '1'){
				playmidi(midiout, 52);
				cout << "note 1" << endl;
			}
			if(maxVal == '2'){
				playmidi(midiout, 50);
				cout << "note 2" << endl;
			}
			if(maxVal == '3'){
				playmidi(midiout, 48);
				cout << "note 3" << endl;
			}
			
			maxVal = '0';
		}

		
		
		
		if (szBuff[0] == '1'){
			
			
		}


	}
	CloseHandle(hSerial);

	// Clean up
	cleanup:
	delete midiout;

	return 0;
}