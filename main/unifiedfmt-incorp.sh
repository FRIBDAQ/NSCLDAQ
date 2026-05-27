#!/bin/bash

##  Incorporate the unfied formatting library:

# Most recently used version 2.2-004 minimum needed
# for the new dumper additions.
# And moving FragmentIndex et al. into ufmt.

REPOSITORY="https://github.com/FRIBDAQ/UnifiedFormat.git"
TAG=$1
TARGET="unifiedformat"

if [[ ! $TAG ]]
then
  TAG="2.4-000"
fi

rm -rf $TARGET
git clone $REPOSITORY $TARGET
(cd $TARGET; git checkout $TAG)
