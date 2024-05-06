#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file blockimage.tcl
# @brief Create images that are solid colors
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide blockimage 1.0

##
# This package provides a proc that creates images that are solid colors.
# This provides  the images needed for e.g. tabs in tabbed notebooks that
# may need to be multi-colored.  The method is to create the image as a photo,
# then get its image with the required background and set it back into the
# original image.

##
# makeBlock
#    Make the image rectangle
#
# @param name - image name.
# @param color- image color - this can be a named color or #rrggbb
# @param width - width of the resulting image in pixels.
# @param height - height of the resulting image in pixels.
# @return image name (first parameter):
#
proc makeBlock {name color width height} {
    image create photo $name -width $width -height $height
    $name put [$name data -background $color -from 0 0 $width $height] -to 0 0 $width $height
    
    return $name
}

##
# getColorImage
#   Gets a large, cached color image (uses makeBlock above) 
#   suitable for using as background for widgets (which will clip it)
#   The images are cached so that they are not continuously remade.
#
#  @param color - color of the widget to be made.
#  @return image handle.
#  

namespace eval ColorImageCache {
    array set cache [list] ;    # The image cache indexed by color.
}

proc getColorImage {color} {
    # See if the request can be fulfilled from the cache

    if {[array names ColorImageCache::cache $color] eq $color} {
        return $ColorImageCache::cache($color)
    }

    # Have to make a new one

    set name getColorImage.$color
    set result [makeBlock $name $color 71 25] 

    set ColorImageCache::cache($color) $result
    return $result
}