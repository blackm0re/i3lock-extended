# i3lock-extended

i3lock-extended is a fork of i3lock (https://github.com/i3/i3lock).
It was started by Simeon Simeonov (sgs @ Freenode) as a small hack, 
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
   # layman -a sgs
   # emerge x11-misc/i3lock-extended
   ```

### Fedora

Set up a custom repo as described: https://pkg.pichove.org/Fedora/README.txt

Key fingerprint: A664 5797 661E 2F47 3DD3  FF06 BCE7 0555 C3BB 08F7

Install the package:

   ```bash
   $ sudo dnf install i3lock-extended
   ```


### From source

   ```bash
   wget https://simeon.simeonov.no/programs/i3lock_extended/downloads/i3lock-extended-<version>.tar.xz
   tar Jxvf i3lock-extended-<version>.tar.xz && cd i3lock-extended-<version>
   ./configure --prefix=/usr --sysconfdir=/etc --disable-sanitizers
   make
   # make install
   ```


### From git

   ```bash
   git clone https://github.com/blackm0re/i3lock-extended.git && cd i3lock-extended
   autoreconf --force --install
   ./configure --prefix=/usr --sysconfdir=/etc --disable-sanitizers
   make
   # make install
   ```


## Support and contributing

i3lock-extended is hosted on GitHub: https://github.com/blackm0re/i3lock-extended


## Author

Simeon Simeonov - sgs @ Freenode


## [License](https://github.com/blackm0re/i3lock-extended/blob/master/LICENSE)

Copyright 2020 Simeon Simeonov

Licensed under the GPLv3: http://www.gnu.org/licenses/gpl-3.0.html

This software includes software copyrighted by Michael Stapelberg and
distributed under a different license: See LICENSE for more information!
