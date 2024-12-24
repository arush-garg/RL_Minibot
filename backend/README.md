# Flask Backend

START THE SERVER USING THE FOLLOWING COMMAND:
`python backend/server.py`

- Ensure that `v_env` has been activated
- Ensure that the current directory is `RL_Minibot`


This backend loads the model and returns the predictions to the ESP32.

To get a prediction, make a POST request to `http://10.0.0.163:8000/predict` with:<br>
1. Cart Position (m)
2. Cart Velocity (m/s) - Forward is positive, backward is negative
3. Pole Angle (rad)
4. Pole Angular Velocity (rad/s) - Clockwise is positive, anticlockwise is negative