#!/usr/bin/env fish

echo "Setting up Vortex Engine..."

cmake -B build

cd build

cmake ..
make

echo "Vortex Engine setup complete!"
