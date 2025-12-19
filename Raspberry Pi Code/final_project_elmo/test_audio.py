import pygame
import time
import os

AUDIO_FILE = "/home/young-vato/Desktop/final_project_elmo/bottle_pcm.wav"   # <-- change this to any .wav you actually have

# Make sure the file exists
if not os.path.isfile(AUDIO_FILE):
    print(f"ERROR: Audio file '{AUDIO_FILE}' not found in this folder.")
    exit(1)

print("Initializing audio system...")
pygame.mixer.init()

print(f"Loading {AUDIO_FILE}...")
try:
    pygame.mixer.music.load(AUDIO_FILE)
except Exception as e:
    print(f"ERROR loading file: {e}")
    exit(1)

print("Playing audio now...")
pygame.mixer.music.play()

# Wait while audio plays
while pygame.mixer.music.get_busy():
    time.sleep(0.1)

print("Done playing audio.")
