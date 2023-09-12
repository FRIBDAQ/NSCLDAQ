# NSCLDAQ

### About releases:
   Releases will, in general, have several files.  The files labeled 'Source Files' are generated by github.  The files named something like nscldaq-M.m-eee.tar.gz are source tarballs generated by us using:
   
   ```make dist```

Files named nscldaq-M.m-eee-distro.tar.gz are binary tarballs that can be rolled into the /usr/opt filesystem of containerized environment using the FRIB containers for the 'distro'  environment e.g. ```nscldaq-11.3-032-bullseye.tar.gz``` is a binary distribution for the Debian bullseye container image. 
