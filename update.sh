#!/bin/bash
./build.sh

sudo systemctl stop recycled
sudo cp recycled /usr/local/bin
sudo cp recyclectl /usr/local/bin
sudo systemctl restart recycled
