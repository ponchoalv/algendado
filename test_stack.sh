#!/bin/bash

echo "Testing stacked notifications..."

# Add multiple notifications to the stack
./algen-stack "First Notification" "This is the first notification" "20:35" &
sleep 1
./algen-stack "Second Notification" "This is the second notification" "20:36" &
sleep 1  
./algen-stack "Third Notification" "This is the third notification" "20:37" &

echo "Three notifications added to stack!"
