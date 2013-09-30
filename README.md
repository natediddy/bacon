bacon
=====
Bacon is a small tool that can show information about CyanogenMod ROMs for
Android devices. As well as downloading the ROMs, bacon will show you the
latest devices supported by the CyanogenMod project.

For more information about the CyanogenMod project, visit [CyanogenMod](http://www.cyanogenmod.org/).
And for more information about Android in general, visit [Android](http://www.android.com/).

Usage
-----
    bacon [OPTION...] [DEVICE...]

Options
-------
    -d, --download             Download ROM for DEVICE
    -M N, --max-roms=N         Show a max of N ROMs per ROM type (with
                               `--show')
    -l, --list-devices         List all available devices
    -L, --latest               Show or download the latest ROM for DEVICE
                               (with the action of showing or downloading
                               depending on whether `--show' or `--download'
                               was used)
    -s, --show                 Only show ROM(s) for DEVICE (don't download
                               them)
    -H, --show-hash            Show the MD5 hash (with `--show')
    -i, --interactive          Interactive mode
    -a, --all                  Specify all ROM types
    -e, --experimental         Specify only Experimental
    -m, --snapshot             Specify only M Snapshot ROMs
    -n, --nightly              Specify only Nightly ROMs
    -r, --rc                   Specify only Release Candidate ROMs
    -S, --stable               Specify only Stable ROMs
    -u, --update-device-list   Update the local device list
    -U, --show-url             Show the URL (with `--show')
    -o FILENAME, --output=FILENAME
                               Download ROM to FILENAME
    -?, -h, --help             Show this help message and exit
    -v, --version              Show version info and exit

Building
--------
In order to build bacon, you will need the development library of libcurl
installed, as well as GNU Autotools.

On a Linux OS you can use:
    ./autogen.sh
    ./configure
    make

Installing
----------
On a Linux OS, after building you can run:
    sudo make install
