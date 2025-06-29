#!/bin/bash

# Install script for Algendado on macOS

echo "Installing Algendado..."

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Homebrew is required but not installed."
    echo "Please install Homebrew first: https://brew.sh/"
    exit 1
fi

# Install dependencies
echo "Installing dependencies..."
brew install sqlite libmicrohttpd raylib

# Build the project
echo "Building project..."
make clean
if ! make; then
    echo "Build failed!"
    exit 1
fi

# Install binaries
echo "Installing binaries to /usr/local/bin..."
sudo cp algen /usr/local/bin/
sudo cp algen-server /usr/local/bin/

echo ""
echo "Installation complete!"
echo ""
echo "Usage:"
echo "  algen add today 11:15:00 \"finish the project\""
echo "  algen get today"
echo ""
echo "The server will start automatically when adding items."
echo "Web interface will be available at: http://localhost:8080"
