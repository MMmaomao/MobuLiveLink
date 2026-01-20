# MobuLiveLink Python API Example
# This script demonstrates how to use the livelink module in MotionBuilder

"""
The livelink module is now available immediately when MotionBuilder starts,
you don't need to manually create a device first.

Available functions:
    - livelink.add_stream_object(name)    : Add a model to the LiveLink stream
    - livelink.remove_stream_object(name) : Remove a model from the LiveLink stream
    - livelink.get_stream_objects()       : Get list of all models in the stream
"""

import livelink

# Example 1: Add a model to the stream
try:
    # Replace 'MyCharacter' with the actual name of your model
    livelink.add_stream_object('MyCharacter')
    print("Model added to LiveLink stream")
except Exception as e:
    print("Error adding model: " + str(e))

# Example 2: Add multiple models
model_names = ['Camera001', 'Light001', 'Character_Root']
for model_name in model_names:
    try:
        livelink.add_stream_object(model_name)
        print("Added: " + model_name)
    except:
        print("Failed to add: " + model_name)

# Example 3: Get all stream objects
try:
    stream_objects = livelink.get_stream_objects()
    print("\nCurrently streaming " + str(len(stream_objects)) + " objects:")
    for obj in stream_objects:
        print("  - " + obj)
except Exception as e:
    print("Error getting stream objects: " + str(e))

# Example 4: Remove a model from the stream
try:
    livelink.remove_stream_object('MyCharacter')
    print("\nModel removed from LiveLink stream")
except Exception as e:
    print("Error removing model: " + str(e))

# Example 5: Add and remove models dynamically
models_to_stream = ['Camera001', 'Light001']

# Add all models
for model in models_to_stream:
    try:
        livelink.add_stream_object(model)
        print("Streaming: " + model)
    except:
        pass

# Check what's streaming
print("\nCurrently streaming:")
for obj in livelink.get_stream_objects():
    print("  - " + obj)

# Later, remove some models
for model in models_to_stream:
    try:
        livelink.remove_stream_object(model)
        print("Stopped streaming: " + model)
    except:
        pass

