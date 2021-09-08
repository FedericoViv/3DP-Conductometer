# 3DP-Conductometer
PSOC project for 3DP conductometer

This repository contains the PSOC project for the circuitry of a conductometer to be used with 3DP electrodes and cell. The source code is released under GPLv3. I don't own any rights on the code regarding the OLED display. All credits for the OLED code go to ADAFRUIT and Derk Steggewentz for his adaptation of ADAFRUIT code for PSOC creator.

The following text will summarize how to build and test the circuitry for the 3DP conductometer.
Material needed:
-	psoc 5lp prototyping kit (CY8CKIT-059)
-	headers
-	jumpers
-	alligators clip
-	10K and 100M resistors for the electronics
-	10 to 100Kohm resistors for calibration
-	capacitor for AC testing (A big one should be fine. We will need to test just the AC behaviour)
-	breadboard (optional, but it is convenient to see if you have assembled everything correctly before soldering)
-	soldering iron
-	computer with windows as OS

1.	Electronics setup

Figure 1 shows the circuit diagram of the conductometer.

![Immagine1](https://user-images.githubusercontent.com/39185430/132537490-3c7dca1e-6751-4a16-aa6b-8e11e57e01c1.png)
Figure 1. Conductometer circuit diagram.

Before undergoing any soldering procedure, I suggest using a breadboard in order to be sure about your connections. Each pin on the psoc 5lp prototyping kit has an assigned number, please be sure you followed the connections showed in figure 1. Figure 2 shows the connection on a breadboard.

Figure 2. Example connections on a breadboard.


2.	Firmware setup

Download and install psoc programmer:
https://www.cypress.com/products/psoc-programming-solutions
Download and install terminal emulator Teraterm or equivalent:
https://ttssh2.osdn.jp/index.html.en

Connect your psoc 5lp programming kit to the computer using the USB port.

NOTE 1. Do not use the mini usb port, the firmware has been programmed to use only the USB

Open the psoc programmer and check if the programmer firmware is updated, you can find the update button in the utilities tab (figure 3).

Figure 3. Update your programmer firmware.

After updating your programmer, you can program your psoc by loading the HEX file containing the conductometer pinout and firmware (figure 4).

Figure 4. Load and program your board. Be sure everything is green in the corner.

Open your terminal and select the serial port of your board (figure 5).

Figure 5. Teraterm serial connection.

At this point you may see random sign popping up on your terminal, this is caused by the fact that the UART communication used on the board to connect the conductometer to the computer is using a communication speed of 115200 baud. You can modify this by following SETUP -> SERIAL PORT -> and modify speed (figure 6).
NOTE 2. Be sure you have closed psoc programmer, or it will keep busy the serial port!
Figure 6. Serial setup speed must be set to 115200 baud.

If you have done everything correctly, your terminal emulator should start showing the value of the resistance of whatever is place between the alligator clips.

TROUBLESHOOTING 1. Since this firmware is designed to be used also with an OLED display, it may happen that the starting routine get stuck, and no digits appear on the terminal. If this happens, press the reset button on the programmer (NOT THE BOARD). See figure 2 for its location.

If everything went smooth so far, you should calibrate your device. Take resistors with values going from 10ohm to 100k (even 1M should be fine) and save the value popping on the terminal.
NOTE 3. The conductometer use a moving average filter, with this algorithm some cycles are needed in order to make the measurement stable. Once you have plugged the resistor, wait a few seconds before taking the values from the terminal (terminal values can be copied by selection followed by right click of the mouse).
NOTE 4. If you are using the breadboard, unstable values or with higher error may be caused the faulty connection, thing should get better as soon as you solder everything together.
As a final test, take a capacitor (the bigger the better) and connect it in series with a resistor. If the AC function generator is working fine, you should see a stable value.
NOTE 5. If the capacitor is big enough you should see the value of the resistor, otherwise the value will be a function of the impedance of the capacitor plus the resistor biased by the presence of other components inside the prototyping kit. For this reason, this device should be used just to measure the impedance of cells being sure that the contribution of capacitive components is negligible.

