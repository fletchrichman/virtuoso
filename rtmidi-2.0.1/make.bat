g++ -Wall -D__WINDOWS_MM__ -c -o pleasework.o Demo_BT_Integration.cpp 
g++ -o pleasework.exe pleasework.o wiimote.cpp Rtmidi.cpp -s -L. -lwinmm -lhid -lsetupapi
