# EventFramework #
 
## Overview ##

This is a facility intended to provide an event framework for
Arduino sketches. In other words wait for or monitor when things happen
like a GPIO input change or timer elapsed and execute a function
bound to the 'event.'

## Requirements ##

Arduino IDE (to build for Arduino)
Eclipse (to build test on PC)

## Usage ##

Install in a location suitable for Arduino libraries. In the Arduino IDE go to 
`Files > Preferences` and look at the path to `Sketchbook location`. If that is
`/path/to/sketchbook` the then cd to `/path/to/sketchbook\libraries` and clone
this repo.

For coding, please see the Music project (Github URL TBD)


For testing on a PC, open the project in Eclipse and run. It would be very 
cool if a standard test framework were employed but that is not the case. Sad!

## Examples ##

Used in Music and MusicGame (not yet in Github)

## Status ##

This works with a couple of other projects. Work on these is resuming
after a couple years break so I cannot recall if there was further work
in progress. Present work is getting this into Github prior to revisiting
it and related projects.

The project originally included test code and that can be explored
at https://github.com/HankB/EventFramework-badstart.  (Look in TEV
directory.)  At some point I plan to reintegrate the test modules with
this project.

## Errata ##

This is an incomplete repo at this point.
