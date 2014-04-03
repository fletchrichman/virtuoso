g++ -Wall -c -o pleasework.o Demo_BT_Integration.cpp
g++ -D__WINDOWS_MM__ -o pleasework.exe pleasework.o wiimote.cpp Rtmidi.cpp -s -lwinmm -lhid -lsetupapi