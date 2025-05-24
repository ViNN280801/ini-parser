#!/bin/bash

rm -rfv build/*
if [ $? -eq 0 ]; then
    echo "Build directory successfully cleaned"
else
    echo "Failed to clean build directory"
fi
