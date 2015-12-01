* BUILD INSTRUCTION *

1) To build the firmware, you need Linux environment. Debian squeeze 6.0.7 and 
   Ubuntu 10.04, 10.10, 11.04 distros has been tested.
2) Build cross-toolchain for MIPS32_R2 CPU (binutils-2.21.1, gcc-447, uclibc-0.9.33.2) 
   from external package toolchain-rt3883.
3) Manual configure firmware content via .config file. Use comment mark # for 
   disable features. Change param "CONFIG_TOOLCHAIN_DIR=" to target cross-toolchain 
   directory.
4) Build firmware via "build_firmware" script. After the build is finished, 
   the firmware file (*.trx) will be placed to directory "images".


WARNING!
After building the firmware, be sure that the firmware file size (*.trx) does not 
exceed 7995392 bytes (8060928 - 65536)!


* NOTE *

To build the firmware under Debian squeeze you need the packages:
- build-essential
- gawk
- sudo
- pkg-config
- gettext
- automake
- autoconf
- libtool
- bison
- flex
- zlib1g-dev

To build cross-toolchain, you need additional packages:
- libgmp3-dev
- libmpfr-dev
- libmpc-dev




-
03/31/2013
Padavan
