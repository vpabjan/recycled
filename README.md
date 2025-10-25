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

## configuration & usage
The config file shall be located in /etc/recycled.conf.\
An example config may just consist of one rule:

>/home/user/Downloads 30d exclude iso zip

- this will automatically delete all files in downloads which are older than 30 days and exclude iso and zip files.

  or of course more

>/home/user/Downloads 30d exclude iso zip\
>/home/user/.cache 12h

- this will do the previous and also clean the .cache folder ever 12 hours.

Available time units are:
- hour (h)
- day (d)
- month (m)

### usage
Just update /etc/recycled.conf as shown above, then call recyclectl reload.

# note
Built and tested on **arch**, should work on anything with **systemd**.

---

Contributions are welcome, just send a pull request. Unlike my other daemon [hwmond](https://github.com/vpabjan/hwmond) this project uses the AGPL license so note the difference if forking.
