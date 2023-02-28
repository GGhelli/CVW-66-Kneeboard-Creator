# CVW-66-Kneeboard-builder
A kneeboard builder loosely based on [KneeboardBuilder](https:https://dcskneeboardbuilder.com/).
This project relies on the mutool.exe utility from [MuPDF](https://mupdf.com) for importing PDF files and [Qt 5.x.x](https://www.qt.io) for all the development framework.

## Building

Just download the source, execute:

- qmake kneeboard.pro
- nmake

The executable will be built in the bin folder

## Deploying

- copy the mutool.exe from [MuPDF](https://mupdf.com) in the bin directory
- execute 'windeployqt kneeboard.exe'
- Copy the 'styles.css' in the bin directory
- Then it's only a copy&paste of the bin directory wherever you want


