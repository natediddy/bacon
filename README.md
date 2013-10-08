bacon
=====
Bacon is a small tool that can show information about CyanogenMod ROMs for
Android devices. As well as downloading the ROMs, bacon will show you the
latest devices supported by the CyanogenMod project.

For more information about the CyanogenMod project, visit [CyanogenMod](http://www.cyanogenmod.org/).
And for more information about Android in general, visit [Android](http://www.android.com/).

Usage
-----
    Usage: bacon [OPTION...] [DEVICE...]

Options
-------
    General Options:
        -d, --download             Download the latest ROM for DEVICE
                                   Requires specific ROM type option
                                   (See 'ROM Type Options' below)
        -i, --interactive          Interactive mode
        -l, --list-devices         List all available DEVICEs
        -N, --no-progress          Do not show any progress when retrieving
                                   data from the internet (this includes the
                                   progress bar during ROM downloads)
        -s, --show                 Show ROMs for DEVICE (no downloading)
        -u, --update-device-list   Update the local DEVICE list
        -?, -h, --help             Display this help text and exit
        -v, --version              Display version information and exit
    ROM Type Options:
        -a, --all                  Specify all ROM types [default when no
                                   ROM types are given]
        -e, --experimental         Specify only Experimental
        -m, --snapshot             Specify only M Snapshot ROMs
        -n, --nightly              Specify only Nightly ROMs
        -r, --rc                   Specify only Release Candidate ROMs
        -S, --stable               Specify only Stable ROMs
    Download Options:
        -o PATH, --output=PATH     Save downloaded ROM to PATH
    Show Options:
        -H, --hash                 Show remote MD5 hash for each ROM
                                   displayed
        -L, --latest               Show only the latest ROM for each ROM type
                                   specified
        -M N, --max=N              Show a maximum of N ROMs for each ROM type
                                   specified
        -U, --url                  Show download URL for each ROM displayed

    Providing no arguments will launch the GTK+ graphical user interface.

Building
--------
In order to build bacon, you will need the development library of libcurl
installed, as well as GNU Autotools.

On a Linux OS you can use:

    ./buildconf
    ./configure [--with-gtk]
    make

Installing
----------
On a Linux OS, after building you can run:

    sudo make install
