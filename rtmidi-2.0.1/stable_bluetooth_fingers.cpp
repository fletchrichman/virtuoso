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


using namespace std;

void midiOn(RtMidiOut* midiout, int note){
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

int main(){

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

	if(szBuff[0] == '0'){
		midiOff(midiout, 52);
		midiOff(midiout, 50);
		midiOff(midiout, 48);
	}
		
	if(szBuff[0] == '4'){
		midiOff(midiout, 52);
		midiOff(midiout, 50);
		midiOn(midiout, 48);
	}
	
	if(szBuff[0] == '2'){
		midiOff(midiout, 48);
		midiOff(midiout, 52);
		midiOn(midiout, 50);
	}
	if(szBuff[0] == '6'){
		midiOff(midiout, 48);
		midiOff(midiout, 50);
		midiOn(midiout, 52);
	}
	

}
CloseHandle(hSerial);

// Clean up
cleanup:
delete midiout;

return 0;
}