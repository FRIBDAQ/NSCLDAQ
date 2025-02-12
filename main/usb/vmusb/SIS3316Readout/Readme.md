## Background

The SIS3316 is a flash ADC with firmware that supports PHA and
PSD computations.   I attempted to add support for it to the 
VMUSBReadout but I think the module is fundamentally incompatible
with the stack-driven capabilities of the VMUSB.

The module digitizes data into RAM that is invisible to the VME bus.
The host system must then, for each ADC channel, initiate a transfer from
the RAM into a FIFO.  Initiating a transfer is destructive, so once the
data from each ADC channel are transferred, the FIFO must be read.

The problem is that there's no published bound on how long the
transfer from RAM to FIFO will take.    There is a bit that
indicates transfer is in progress and that must be polled until
clear prior to performing the FIFO readout.

This is not within the capability of the VMUSB autonomous mode 
command set.

### What's here

What we're doing in this directory is an unholy fusion of the 
SBS readout framework (which can respond in software to individual triggers)
and the VMUSB interface support in ../vmusb.   

We'll try to retain Tcl script based configuration of the set of SIS3316
modules available.  However, each module will be an event segment and
each module event segment will have its own configuration script.

### How to use it.

This directory will install an enhanced Readout skeleton.  The skeleton will work
just like ano other SBS readout skeleton, however, there are several key classes:

* CSIS316EventSegment which should not be instantiated.
* CV977Eventsegment which should not be defined.
* CSIS3820Event segment which can read an SIS scaler in timestamp mode.
* CVMUSBScalersEventSegment which can configure inputs and read the VMUSB scalers.
* CSIS3820ScalerBank which can read an SIS scaler as a scaler
* CConfigurableCompoundEventSegment which defines a configuration script for the module
that are defined in it.  The configuration is exactly the same as a typical VMUSBReadout
daqconfig script commands defined are:
    *  sis3316
    *  v977
* CConfigurableScalerBank - which is for scaler banks what CConfigurableEventSegment is for events.
    

On initialize will empty out the sub segments and run the configuration
script:
*   create creaes a new module object and adds it to the contained sub-segments.
*   config - locates the module object and configures it.
*   cget - locates the module and configure it.


### The trigger and busy...