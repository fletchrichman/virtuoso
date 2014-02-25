g++ -Wall -g0 -O2 -c WiimoteMidi.cpp -o WiimoteMidi.o
g++ -D__WINDOWS_MM__ -o WiimoteMidi.exe WiimoteMidi.o wiimote.cpp RtMidi.cpp -s -L. -lwinmm -lhid -lsetupapi

@rem g++ -o WiimoteMidi.exe WiimoteMidi.o wiimote.dll -lwinmm ; to create Demo.exe that uses wiimote.dll