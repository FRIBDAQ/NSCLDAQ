#!/bin/bash

##  Incorporate the DDAS format library:


# See TAG definition for the default to incorp.

REPOSITORY="https://github.com/FRIBDAQ/DDASFormat.git"
TAG=$1
TARGET="ddasformat"

if [[ ! $TAG ]]
then
  TAG="1.1-002"
fi

rm -rf $TARGET
git clone $REPOSITORY $TARGET
(cd $TARGET; git checkout $TAG)
