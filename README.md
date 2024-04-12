# i3lock-extended

i3lock-extended is a fork of i3lock (https://github.com/i3/i3lock).
It was started by Simeon Simeonov (sgs @ LiberaChat) as a small hack, 
implementing a LED-clock on top of an existing and robust screensaver.

i3lock-extended adds some additional functionality and is a free-software
licenced under the GPL3+. Most of the code is copyrighted by Michael Stapelberg
under a different license. See LICENSE for more information\!

The original README.md file from i3lock, containing installation instructions
can be found as INSTALL.md.


## Additional features

- LED-clock
- Digital clock
- Elapsed time since activation
- Display-text
- Count broken-down time until a specific calendar time (absolute time)

i3lock-extended includes all of the original code and features of i3lock.

Please refer to the manual page for more details and information about the different parameters\!


## Versioning

i3lock-extended will merge the changes and fixes coming to i3lock in the
future.

In order to create more clarity the version format for i3lock-extended will be:
\<extended version starting from 1>.\<i3lock version that is merged in\>[a-z
extended revision if used].


## Installation

### Gentoo

   ```bash
   # add sgs' custom repository using app-eselect/eselect-repository
   eselect repository add sgs

   emerge x11-misc/i3lock-extended
   ```


### Fedora ( >= 31)

Set up a custom repo as described: https://pkg.pichove.org/Fedora/README.txt

Key fingerprint: A664 5797 661E 2F47 3DD3  FF06 BCE7 0555 C3BB 08F7

Install the package:

   ```bash
   sudo dnf install i3lock-extended
   ```


### From source (meson > 0.55.0)

   ```bash
   wget https://github.com/blackm0re/i3lock_extended/releases/download/<version>/i3lock-extended-<version>.tar.xz
   tar Jxvf i3lock-extended-<version>.tar.xz
   cd i3lock-extended-<version>
   meson setup build --prefix=/usr
   meson compile -C build
   sudo meson install -C build
   ```


### From git (meson > 0.55.0)

   ```bash
   git clone https://github.com/blackm0re/i3lock-extended.git
   cd i3lock-extended
   meson setup build --prefix=/usr
   meson compile -C build
   sudo meson install -C build
   ```


## Examples

The following invocation

   ```bash
   i3lock-extended -f -L -n -S 32 -c 000000 -F 000000 -O 00FF00 -B 7f7f7f -R 7f7f7f -G 7f7f7f -D -E
   ```

produces the following result:

![Screenshot](https://simeon.simeonov.no/programs/i3lock_extended/screenshot.png)

while

   ```bash
   date +%s -d"Jan 1, 2021 00:00:00"
   # returns seconds since the Epoch (f.i. 1609455600)
   i3lock-extended -f -L -n -S 32 -c 000000 -F 000000 -O 00FF00 -B 7f7f7f -R 7f7f7f -G 7f7f7f \ 
   -D -C 1609455600 -W left -Z top -J "%d days, %h hours, %m minutes and %s seconds until 2021"
   ```

also displays the broken-down time until 2021 using the specified template (-J)
in the top left corner (instead of elapsed time).


## Support and contributing

i3lock-extended is hosted on GitHub: https://github.com/blackm0re/i3lock-extended


## Author

Simeon Simeonov - sgs @ LiberaChat


## [License](https://github.com/blackm0re/i3lock-extended/blob/master/LICENSE)

Copyright 2020-2024 Simeon Simeonov

Licensed under the GPLv3: http://www.gnu.org/licenses/gpl-3.0.html

This software includes software copyrighted by Michael Stapelberg and
distributed under a different license: See LICENSE for more information!
