# recycled
A minimalistic automatic cleanup daemon &amp; recyclectl.

## installation

>chmod +x build.sh \
>chmod +x update.sh \
>chmod +x install.sh

- then just

>./install.sh

## updating

simply run

>./update.sh

- this automatically builds, stops the daemon, copies the new version and restarts it.

## configuration
The config file shall be located in /etc/recycled.conf.\
An example config may just consist of one rule:

>/home/user/Downloads 30d exclude iso zip

- this will automatically delete all files in downloads which are older than 30 days and exclude iso and zip files.

Available time units are:
- hour (h)
- day (d)
- month (m)

# note
Built and tested on **arch**, should work on anything with **systemd**.
