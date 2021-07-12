# libDaisy Changelog

## Unreleased

* Add unit testing for midi parser.  
* Fix midi parser bugs.
  * Sysex messages that overflow stop reading data until rx sysexstop. Previously overflowed sysex would cause junk messages.
  * NoteOns of velocity 0 cause NoteOffs.
* Update TODO comment in uart.h to reflect most recent uart update.
* Add MidiEvent and related data to daisy namespace.

## v0.1.0

Initial Release

