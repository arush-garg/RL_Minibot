from flask import Flask, jsonify, request
from dotenv import load_dotenv
import os
import numpy as np
import tensorflow as tf


app = Flask(__name__)
load_dotenv()

# Load the model
model = tf.keras.models.load_model('../model/dqn-model.keras')

if(os.getenv('TRAINING_MODE') == 'true'):
    print("Training mode")
    optimizer = tf.keras.optimizers.Adam(learning_rate=0.0001)
    model.compile(optimizer=optimizer, loss='mse')
    gamma = 0.995
else:
    print("Prediction mode")
    model.trainable = False


@app.route('/')
def home():
    return "Make a POST request with the information detailed in the README file"


@app.route('/predict', methods=['POST'])
def predict():
    data = request.get_json()
    print(data)

    current_state, previous_state = clean_data(data)

    if os.getenv("TRAINING_MODE") == 'true' and data['action'] != -1:
        train_model(previous_state, current_state, data['action'], data['reward'], data['done'])

    action = model.predict(current_state, verbose=0)
    action = np.argmax(action[0])

    return jsonify({'action':str(action)})


def train_model(previous_state, current_state, action, reward, done):
    """
    Fine-tunes the model using Q-learning principles with previous_state and current_state. The action should be the action taken in the previous state, and the reward is the reward received after taking that action. The done parameter is a boolean indicating whether the episode has ended.
    """
    
    # Predict Q-values for the previous state
    q_values = model.predict(previous_state, verbose=0)

    next_q_values = model.predict(previous_state, verbose=0)
    max_next_q_value = np.max(next_q_values)

    target_q_values = reward + gamma * max_next_q_value * (1 - done)

    q_values = model.predict(previous_state, verbose=0)
    q_values[0, action] = target_q_values

    model.fit(previous_state, q_values, epochs=1, verbose=0)
    model.save('../model/dqn-model.keras')


def clean_data(data):
    """
    Cleans the data received from the client to make it suitable for the model. The data should be a dictionary with the following keys: `current_state`, `previous_state`, `action`, `reward`, and `done`.
    """

    current_state = np.expand_dims(data['current_state'], axis=0)
    previous_state = np.expand_dims(data['previous_state'], axis=0)

    current_state[current_state == None] = 0
    previous_state[previous_state == None] = 0

    current_state = current_state.astype(np.float32)
    previous_state = previous_state.astype(np.float32)

    return current_state, previous_state


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)