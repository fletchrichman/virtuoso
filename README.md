# Virtuoso Readme

## Arduino and glove voltages setup  

While a user is wearing the gloves, the voltage on each finger needs to be tested at the extended and bent states. Measure the different between the input pin on the Lilypad Arduino (A0-A3) and ground, and flex the corresponding finger.  Utilize this range of voltages to set the configuration values in the Virtuoso.c Arduino code, which has  comments to specify which voltage initializes the pointer, middle, ring, and pinky fingers. Measure the voltage from power to ground on the Arduino as well and set that value as the power value in the code. Connect the computer to the Lilypad Arduino and upload the code to the glove. Repeat this process for each glove. 

## Bluetooth Arduino and Wiimote Setup 

Plug in Bluetooth dongles into the Lilypad Arduinos, and turn on the gloves. On the computer, open the �Devices and Printers� window. Right click and select �Add a Device�. You should see two devices whose name begins with RNBT. Select one of the devices, and add the device with pairing code 1234. Repeat for the other device. You should see the computer installing driver software successfully. Next, get the Wiimote out, turn it on, hold down the 1 and 2 buttons, and click add device. Select the Nintendo device, and choose to pair without a code. Computer will connect, configure, and install the driver software for the device.  


## Setting COM Ports and Compiling 

Right click each Bluetooth module in the devices and printers window, select properties, and then select the hardware tab. Find the COM port number in the name column, and take note of the COM port number of each module. Go into the Virtuoso.cpp file and change the COM ports on line _____ to correspond to each Bluetooth module. You need to have the MinGW compiler installed, the Windows SDK, and the Windows Driver Kit. Once you�ve installed these, open the terminal and go to the project directory. Run make_Virtuoso.bat. This should create a Virtuoso.exe file. Last, open the Virtuoso_Main.m Matlab file. Click play on the file. This will open a terminal which will connect to both gloves and the wiimote, a GUI to show the location of the hands, and you can now start sending MIDI signals. 

## Setting up the MIDI Port 

Install the LoopBe1 free virtual midi driver to create a midi port that the program can utilize. Please see the User Manual for DAW specific instructions for interpreting MIDI signals and creating music. 

## Troubleshooting 

If Wiimote won�t connect, just keep adding and removing it. 

If the COM ports in the properties are higher than 9, then you�ve got some hidden ports that are unused. To fix it, follow these steps, and then remove and add the Bluetooth devices again. 

1. Right-click Command Prompt in Accessories and choose Run as Administrator
2. Enter 'set devmgr_show_nonpresent_devices=1?' without the quotes obviously
3. Enter 'start devmgmt.msc'
4. In the box that opens, select 'Show hidden devices' in the view menu.
Now if you expand the section on COM ports, all the COM ports that have ever
been created will be displayed, the non present ones being in grey. You can
uninstall away anything that you don�t want (right click, select uninstall).

