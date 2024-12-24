import gymnasium as gym
import numpy as np
import tensorflow as tf


model = tf.keras.models.load_model('../model/dqn-model.keras')
test_env = gym.make("CartPole-v1", render_mode="human")
for e in range(10):
    state, _ = test_env.reset()
    done = False
    i = 0

    while not done:
        test_env.render()
        action = np.argmax(model.predict(state[np.newaxis], verbose=0))
        next_state, reward, terminated, truncated, _ = test_env.step(action)

        done = terminated or truncated

        state = next_state
        i += 1
        if done:
            print("episode: {}/{}, score: {}".format(e, 10, i))
            break
test_env.close()