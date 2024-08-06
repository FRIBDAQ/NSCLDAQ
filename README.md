# NSCLDAQ

### About releases:
   Releases will, in general, have several files.  The files labeled 'Source Files' are generated by github.  The files named something like nscldaq-M.m-eee-dist.tar.gz are source tarballs generated by us using:
   
   ```make dist```

Files named nscldaq-M.m-eee-distro.tar.gz are binary tarballs that can be rolled into the /usr/opt filesystem of containerized environment using the FRIB containers for the 'distro'  environment e.g. ```nscldaq-11.3-032-bullseye.tar.gz``` is a binary distribution for the Debian bullseye container image. 


Files named usropt-nscldaq-MM.mm-eee-distro.tar.gz are the dependencies for
NSCLDAQ compiled for usex with the binary tarballs.  They too can be rolled into the appropriate /usr/opt filesystem of a containerized environment.

### Abandon hope all ye who enter here - building from sources.

Why 'Abandon hope'?  NSCLDAQ has acreted a large number of dependencies and
we've only tracked them by putting them (and other pakcages) into our container
images.  Expect an iterative process of configuring, building cursing, adding a
dependency and trying again.

In the unlikely event your system has all the dependencies installed (well if
it's one of our container images it will but then save yourself the trouble
and get a binary install), the process is pretty typical, unroll the tarball then

```sh
./configure --prefix=/where/to/install ....
make all    # with a multicore system use an appropriate -j to speed it up
make install
VERBOSE=1 make check #  optional run unit tests.
```

Note that out of tree builds are also supported e.g.

```sh
/location/of/configure/configure ...
...
```

Now about the ...  NSCLDAQ is a framework that incliudes support for many otional
components.  These are enabled and helped to build via options to the configure
command.  Let's have a look at these options:

*  ```--enable-caen-digitizer``` - builds support for the first generation of
CAEN digitizers (e.g. 1725) support is provided for DPP-PHA and DPP_PSD
This normally requires --with-```caen-digitizer-libroot=/path-to-caen-digitizer-libraries```
*  ```--enable-caen-nextgen``` Enables building the CAEN nextgen digitizer support.
This builds support for the DPP-PHA firmware in the eg. 27xxx family of digitizers.
This support will require you to install the libraries to support that set of
digitizers.  At the time support was developed, the libraries really did not like
being installed anywhere other than their default location, so they are looked for there.
* ```--enable-docs```  Builds the documentation for the software.  This is installed in
the share subdirectoy tree and requires docbook2html.
* ```--enable-ddas-docs```  Enables building and installing the documentation for
NSCLDAQ's support for XIA Pixie16 digitizers.
directory tree for CERN-ROOT.
*  ```--enable-epics-tools`````` Enables some useful tools for interacting with EPICS process control
variables.  This may require ``````--with-epics-rootdir``` to specify where EPICS is installed.
* ```---enable-ddas```  Enable the build of the XIA Pixie16 support   Note this will require
```--with-plx-sdk``` to point to the installation path of the PLX software development kit
and ```--with-xiaapidir``` to supply the path to the XIA API installation directory tree
and ```--with-firmwardir``` and ```--with-dspdir``` to provide paths to the FPGA and DSP firmware
directories respectively.   Finally ```--with-rootsys``` is also required to specify the installtion
* ```--enable-sbs``` Enable support for the SBS, GE-FANUC, ... PCI/VME bus bridge.
* ```--enable-usb``` Enable support for the VM?CC usb controllers
* ```--with-bost-log``` Enable support of Boost's logging library.  This may require one of
```--with-bost``` or ```--with-boost-libdir```


THere are many other options and more information can be gotten from

```sh
./configure --help
```
