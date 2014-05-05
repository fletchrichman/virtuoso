g++ -Wall -c -o Virtuoso.o Virtuoso.cpp
g++ -D__WINDOWS_MM__ -o Virtuoso.exe Virtuoso.o wiimote.cpp Rtmidi.cpp -s -lwinmm -lhid -lsetupapi