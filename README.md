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
        -c, --color                Enable colored output
        -d, --download             Download the latest ROM for DEVICE
                                   Requires specific ROM type option
                                   (See 'ROM Type Options' below)
        -f PATTERN, --find-device=PATTERN
                                   Search all supported devices for PATTERN,
                                   and print the results.
                                   This can be useful for identifying the
                                   codename of a particular device.
        --gtk                      Launch the GTK+ user interface
                                   (ONLY IF COMPILED WITH GTK+)
                                   NOTE: when this option is used, it MUST be
                                   provided as the FIRST option (so that any
                                   GTK+ specific options can be provided
                                   after this one on the command line)
        -i, --interactive          Interactive mode
        -l, --list-devices         List all available DEVICEs
        -p, --no-progress          Do not show any progress when retrieving
                                   data from the internet (this includes the
                                   progress bar during ROM downloads)
        -s, --show                 Show ROMs for DEVICE (no downloading)
        -u, --update-device-list   Update the local DEVICE list
        -?, -h, --help             Display this help text and exit
        -v, --version              Display version information and exit
    ROM Type Options:
        -a, --all                  Specify all ROM types
                                   NOTE: this is the default if no other ROM
                                   types were specified
        -e, --experimental         Specify Experimental
        -m, --snapshot             Specify M Snapshot ROMs
        -n, --nightly              Specify Nightly ROMs
        -r, --rc                   Specify Release Candidate ROMs
        -S, --stable               Specify Stable ROMs
    Download Options:
        -o PATH, --output=PATH     Save the downloaded ROM to PATH
                                   A few things to keep in mind:
                                   - If PATH is an existing directory, then
                                     PATH will be appended with the ROM
                                     filename.
                                   - If PATH does not exist, then an attempt
                                     to create PATH's parent will be made,
                                     and PATH's basename will become the ROM
                                     filename.
                                   - If PATH is an existing file, then it
                                     will be overwritten (UNLESS it is a
                                     previously downloaded ROM by this
                                     program, in which case an attempt to
                                     resume the download will be made).
                                   - If PATH exists and its MD5 hash matches
                                     the remote ROM MD5 hash, then nothing
                                     will be done.
    Show Options:
        -H, --hash                 Show remote MD5 hash for each ROM
                                   displayed
        -L, --latest               Show only the latest ROM for each ROM type
                                   specified
        -M N, --max=N              Show a maximum of N ROMs for each ROM type
                                   specified
        -U, --url                  Show download URL for each ROM displayed

Building
--------
In order to build bacon, you will need the development library of libcurl
installed, as well as GNU Autotools.

On a Linux OS you can use:

    ./buildconf all [--with-gtk]

Installing
----------
On a Linux OS, after building you can run:

    sudo make install
Usage: bacon [OPTION...] [DEVICE...]
General Options:
  -d, --download             Download the latest ROM for DEVICE
                             Requires specific ROM type option
                             (See 'ROM Type Options' below)
  -G, --gtk                  Launch the GTK+ user interface
                             NOTE: when this option is used, it MUST be
                             provided as the FIRST option (so that any
                             GTK+ specific options can be provided
                             after this one on the command line)
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
  -a, --all                  Specify all ROM types
                             NOTE: this is the default if no other ROM
                             types were specified
  -e, --experimental         Specify Experimental
  -m, --snapshot             Specify M Snapshot ROMs
  -n, --nightly              Specify Nightly ROMs
  -r, --rc                   Specify Release Candidate ROMs
  -S, --stable               Specify Stable ROMs
Download Options:
  -o PATH, --output=PATH     Save the downloaded ROM to PATH
                             A few things to keep in mind:
                             - If PATH is an existing directory, then
                               PATH will be appended with the ROM
                               filename.
                             - If PATH does not exist, then an attempt
                               to create PATH's parent will be made,
                               and PATH's basename will become the ROM
                               filename.
                             - If PATH is an existing file, then it
                               will be overwritten (UNLESS it is a
                               previously downloaded ROM by this
                               program, in which case an attempt to
                               resume the download will be made).
                             - If PATH exists and its MD5 hash matches
                               the remote ROM MD5 hash, then nothing
                               will be done.
Show Options:
  -H, --hash                 Show remote MD5 hash for each ROM
                             displayed
  -L, --latest               Show only the latest ROM for each ROM type
                             specified
  -M N, --max=N              Show a maximum of N ROMs for each ROM type
                             specified
  -U, --url                  Show download URL for each ROM displayed
