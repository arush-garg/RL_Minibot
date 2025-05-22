from collections import deque
from flask import Flask, jsonify, request
import numpy as np
import tensorflow as tf
import random
import threading


TRAINING_MODE = True

app = Flask(__name__)

# Load the model
model = tf.keras.models.load_model('../model/dqn-model.keras')
target_model = tf.keras.models.load_model('../model/dqn-model.keras')

steps = 0

if(TRAINING_MODE):
    print("Training mode")

    BATCH_SIZE = 8
    TARGET_UPDATE_INTERVAL = 32
    EPOCHS = 3

    optimizer = tf.keras.optimizers.Adam(learning_rate=0.0005)
    model.compile(optimizer=optimizer, loss=tf.keras.losses.Huber())
    gamma = 0.995
    replay_memory = deque(maxlen=100)

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

    if TRAINING_MODE and data['action'] != -1:
        # Run training thread
        threading.Thread(target=train_model, args=(previous_state, current_state, data['action'], data['reward'], data['done'])).start()

    action = model.predict(current_state, verbose=0)
    action = np.argmax(action[0])

    return jsonify({'action':str(action)})


def train_model(previous_state, current_state, action, reward, done):
    """
    Fine-tunes the model using Q-learning principles with previous_state and current_state. The action should be the action taken in the previous state, and the reward is the reward received after taking that action. The done parameter is a boolean indicating whether the episode has ended.
    """
    
    global steps
    steps += 1

    if len(replay_memory) > BATCH_SIZE and steps%BATCH_SIZE == 0:
        batch = random.sample(replay_memory, BATCH_SIZE)

        states, actions, rewards, next_states, dones = map(np.array, zip(*batch))

        # Compute target Q-values
        next_q_values = target_model.predict(next_states, verbose=0)
        max_next_q_values = np.max(next_q_values, axis=1)
        target_q_values = rewards + gamma * max_next_q_values * (1 - dones)

        q_values = model.predict(states, verbose=0)
        for i, action in enumerate(actions):
            q_values[i, action] = target_q_values[i]
        
        model.fit(states, q_values, epochs=EPOCHS, verbose=1)
        model.save("../model/dqn-model.keras")
    else:
        replay_memory.append((np.squeeze(previous_state), action, reward, np.squeeze(current_state), done))

    if steps%TARGET_UPDATE_INTERVAL == 0:
        target_model.set_weights(model.get_weights())


def clean_data(data):
    """
    Cleans the data received from the client to make it suitable for the model. The data should be a dictionary with the following keys: `current_state`, `previous_state`, `action`, `reward`, and `done`.
    """

    current_state = np.expand_dims(data['current_state'], axis=0)
    previous_state = np.expand_dims(data['previous_state'], axis=0)
    
    if None in current_state or None in previous_state:
        print('None values in the data')
    
    current_state[current_state == None] = 0
    previous_state[previous_state == None] = 0

    current_state = current_state.astype(np.float32)
    previous_state = previous_state.astype(np.float32)

    return current_state, previous_state


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)