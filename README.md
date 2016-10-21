# RouterN56U
Source Tree for my ASUS N56U! 

Upstream: OLD/ARCHIVED: http://code.google.com/p/rt-n56u/ 

CURRENT: https://bitbucket.org/padavan/rt-n56u/

Before start the building, we need install the following packages in Ubuntu system:
sudo apt-get install build-essential sudo gawk pkg-config gettext automake autoconf libtool bison flex zlib1g-dev libgmp3-dev libmpfr-dev libmpc-dev texinfo mc

After you install the above packages in your ubuntu Linux building environment, you could do the following to build the final image for your Route N56U:
cd your_RouterN56U; 
./build.sh

The above script will build both toolchain and the source tree of your N56U router. After the first building, you needn't call ".build.sh" to build both your toolchain and N56U image, you could ignore the toolchian building by typing the following command:
"./build.sh img"

