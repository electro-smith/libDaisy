# libDaisy Changelog

## Unreleased

Add unit testing for midi parser.  
Fix midi parser bugs.
 - Sysex messages that overflow stop reading data until rx sysexstop. Previously overflowed sysex would cause junk messages.
 - NoteOns of velocity 0 cause NoteOffs.
## v0.1.0

Initial Release

