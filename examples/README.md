# libDaisy Local Examples

These examples are meant to serve a few purposes:

1. Provide compilable examples for CI to catch library issues that can only happen when including headers that aren't compiled along with the library.
2. Provide simple, illustrative usage examples for inclusion within the documentation
3. Provide usage examples for hardware specific modules that would be hard to wrap into the unit tests

These should not be considered replacements for, unit tests, but rather as a form of integration tests.

To keep this lightweight, we won't include the standard debug files (vscode, or elsewhere), etc. 

All wiring information for required external circuits, or wiring should be included in the opening comments of the source file.

No usage of any external submodules like DaisySP, etc.

For compatibility with the generated documentation, comments can not be done with the `/** text */` style, otherwise they will be stripped out when included within the actual documentation.

## TODO

* Add exmaples to the CMake build to test for building that way as well

