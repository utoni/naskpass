<img src=https://travis-ci.org/lnslbrty/naskpass.svg?branch=master>
<img src=https://scan.coverity.com/projects/9389/badge.svg?flat=1>

naskpass
========
Ncurses based replacement for askpass (related to cryptsetup). <br />

build instructions
========
debian: dpkg-buildpackage -b <br />
non-debian: ./compile.sh <br />
non-autotools: ./configure && make <br />

dependencies
========
post-build: coreutils, cryptsetup, libncurses5, libtinfo5 <br />
pre-build: debhelper, libncurses5-dev, libtinfo-dev, autoconf, automake <br />
Recommends: openssh-server <br />
Conflicts: dropbear <br />

note
========
Plymouth may not like naskpass (never verified). <br />
Do not use debian and dropbear during boot (broken initscript). <br />

screenshots
========

<img src=https://i.imgur.com/Vea7dQ5.jpg>
<img src=https://i.imgur.com/rU2nrBW.jpg>
