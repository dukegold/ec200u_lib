#!/bin/bash

# QuectelEC200U Documentation Build Script

echo "======================================"
echo "QuectelEC200U Documentation Builder"
echo "======================================"
echo ""

# Check if we're in the docs directory
if [ ! -f "conf.py" ]; then
    echo "Error: Please run this script from the docs directory"
    exit 1
fi

# Check for Python
echo "Checking Python installation..."
if command -v python3 &> /dev/null; then
    PYTHON=python3
    PIP=pip3
elif command -v python &> /dev/null; then
    PYTHON=python
    PIP=pip
else
    echo "Error: Python is not installed"
    exit 1
fi

echo "Using $PYTHON"
$PYTHON --version
echo ""

# Install dependencies
echo "Installing dependencies..."
$PIP install -r requirements.txt
echo ""

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf _build
echo ""

# Build HTML documentation
echo "Building HTML documentation..."
sphinx-build -b html . _build/html

if [ $? -eq 0 ]; then
    echo ""
    echo "======================================"
    echo "✅ Documentation built successfully!"
    echo "======================================"
    echo ""
    echo "📁 Location: $(pwd)/_build/html/index.html"
    echo ""
    echo "To view the documentation:"
    echo "  open _build/html/index.html"
    echo ""
    echo "To start a live server:"
    echo "  $PIP install sphinx-autobuild"
    echo "  sphinx-autobuild . _build/html"
    echo ""
else
    echo ""
    echo "❌ Build failed. Please check the errors above."
    exit 1
fi