#!/bin/bash
./build.sh
sudo cp recycled /usr/local/bin
sudo cp recyclectl /usr/local/bin
sudo cp recycled.service /etc/systemd/system

sudo systemctl enable recycled
sudo systemctl start recycled
