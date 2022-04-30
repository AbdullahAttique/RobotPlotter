
# Robot CNC plotter

A robot that can draw a picture by moving around on a page with a pen. It uses Gcode to draw the image (the sort of file that a 3D printer, laser cutter, or CNC mill would use). The Gcode file could easily be produced using a vector drawing. Programmed using arduino.
## Features

- slot for pen, with a replaceable bit that can be customised for different pens
- SD card reader for reading drawing files.
- integrated OLED display for selecting drawings from the SD card.
- rechargeable battery 


## Images
The Robot
![A picture of the robot](https://github.com/AbdullahAttique/RobotPlotter/blob/main/robotPlotterPicture.jpg?raw=true)

CAD Model
![The CAD Model of the robot](https://github.com/AbdullahAttique/RobotPlotter/blob/main/robot%20cad%20angle.png?raw=true)



## Technical Details
- driven by an arduino nano board.
- moves using 2 inexpensive 28BYJ-48 stepper motors paired with ULN2003 stepper drivers.
- pen placed down/lifted using SG90 servo.
- 1.3 inch single colour OLED. driven using a text-only OLED library due to memory constraints of arduino nano.
- knob for scrolling the interface, and buttons for selecting.
- SD card reader module.
## Libraries Used
- Low memory OLED library - SSD1306Ascii
