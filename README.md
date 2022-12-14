
# Raspberry-Pi-Controls-Robot
Running the Makefile on a Raspberry Pi running the Raspberry Pi OS will generate a binary. Running this binary will launch the OpenCV GUI, where the user can select an object to track.  Once the object has been selected, the algorithm will track the object as it moves around.   
This project, in particular, demonstrates that a Raspberry Pi can be used to control a robot with wheels using a web camara. 
The OpenCV object-tracking demo found here: https://docs.opencv.org/3.4/d6/d7f/samples_2cpp_2camshiftdemo_8cpp-example.html was modfied to send a "Turn Left" or "Turn Right" signals to a robot. 

# Arduino Mega
Under the raspi_arduino directory you will find a file raspi_arduino.ino written using the Arduino language that recevies "Turn Left" or "Turn Right" signals from the Raspberry Pi. 

For more details, please view the PDF file found here: https://github.com/mathurpratik/Raspberry-Pi-Controls-Robot/blob/main/ENEE699-Section-0301-Mathur-Pratik.pdf


![Screenshot](raspi_arduino.png)  ![Screenshot](robot.png)
