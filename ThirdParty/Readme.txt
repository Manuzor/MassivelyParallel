Third Party Libraries
=====================

This directory is where all third party dependencies should be referenced. The build system expects folders to the projects here, such as SFML.

The recommended way to set up the third party libs is to create a link to the actual directory (e.g. using `mklink -J`). You could also create real directories here and copy the files over.

The following is a list of required libs.

SFML
----

Used by these projects:
  - Texture
Required directories:
  - "SFML_x64" for the 64 bit build of SFML
  - "SFML_Win32" for the 32 bit build of SFML
Notes:
  - The expected directory structure is the one produced by the CMake generated "INSTALL" project of SFML.
  - If you are building a 'Release' version of any of our projects, you will need the 'Release' version of SFML. The same goes for the 'Debug' configuration.
