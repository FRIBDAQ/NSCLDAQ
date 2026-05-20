#!/bin/bash

##  Incorporate the DDAS format library:


# See TAG definition for the default to incorp.

REPOSITORY="https://github.com/FRIBDAQ/DDASFormat.git"
TAG=$1
TARGET="ddasformat"

if [[ ! $TAG ]]
then
  TAG="2.0-001"
fi

rm -rf $TARGET
git clone $REPOSITORY $TARGET
(cd $TARGET; git checkout $TAG)
