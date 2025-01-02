# ESP32 Code
This is the code that will be deployed on the ESP32, the microcontroller controlling the drive base. The code in the `DrivebaseController` project was written in the Arduino IDE.

The ESP32 controls the drivebase and sends the `current_state`, `previous_state`, `reward`, `action` and `done` back to the server. If `TRAINING_MODE = true`, then it will be used to train the model. Otherwise, just the prediction will be returned based on `current_state`