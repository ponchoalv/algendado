#!/bin/bash

# Test script for Algendado

echo "Testing Algendado..."

# Build the project
echo "Building project..."
if ! make; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Build successful!"
echo ""

# Test adding items
echo "Testing add functionality..."
./algen add today 14:30:00 "Test meeting"
./algen add tomorrow 09:00:00 "Morning standup"
./algen add 01/07/2025 16:00:00 "Project deadline"

echo ""
echo "Testing get functionality..."
./algen get today

echo ""
echo "Testing server startup..."
echo "Starting server in background..."
./algen-server &
SERVER_PID=$!

sleep 3

echo "Testing web interface..."
if curl -s http://localhost:8080 > /dev/null; then
    echo "✅ Web interface is accessible"
else
    echo "❌ Web interface is not accessible"
fi

echo "Testing ICS calendar..."
if curl -s http://localhost:8080/calendar.ics > /dev/null; then
    echo "✅ ICS calendar is accessible"
else
    echo "❌ ICS calendar is not accessible"
fi

echo ""
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo "Tests completed!"
