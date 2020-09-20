PrintPDF
========

Easy PDF creation with GhostScript.


Introduction
------------

PrintPDF provides a front-end to the `*ps2pdf` command in GhostScript, allowing PDF documents to be produced more easily. Normally, the process requires a document to be 'printed' to a postscript file, before that file is passed through GhostScript and converted into a PDF. This two stage process is complicated by the fact that the RISC OS printer drivers can only 'print' to a file in a single location, so care has to be taken not to overwrite an existing postscript file before it has been converted.

By setting up a dedicated postscript driver in Printers, which prints to a specific location, PrintPDF can watch out for new documents being printed and add them to a queue. These are processed in turn, with the resulting PDF files being saved using drag-and-drop from a dialogue box. To the user, printing from an application while this printer driver is selected will result in a Create PDF dialogue box opening; the PDF is saved directly, and no postscript files need be seen.

As the process behind the scenes still involves creating a postscript file and then converting that to a PDF, PrintPDF also provides a quick way to convert any of these files which already exist by dragging them to its icon.


Building
--------

PrintPDF consists of a collection of C and un-tokenised BASIC, which must be assembled using the [SFTools build environment](https://github.com/steve-fryatt). It will be necessary to have suitable Linux system with a working installation of the [GCCSDK](http://www.riscos.info/index.php/GCCSDK) to be able to make use of this.

With a suitable build environment set up, making PrintPDF is a matter of running

	make

from the root folder of the project. This will build everything from source, and assemble a working !PrintPDF application and its associated files within the build folder. If you have access to this folder from RISC OS (either via HostFS, LanManFS, NFS, Sunfish or similar), it will be possible to run it directly once built.

To clean out all of the build files, use

	make clean

To make a release version and package it into Zip files for distribution, use

	make release

This will clean the project and re-build it all, then create a distribution archive (no source), source archive and RiscPkg package in the folder within which the project folder is PrintPDFd. By default the output of `git describe` is used to version the build, but a specific version can be applied by setting the `VERSION` variable -- for example

	make release VERSION=1.23


Licence
-------

PrintPDF is licensed under the EUPL, Version 1.2 only (the "Licence"); you may not use this work except in compliance with the Licence.

You may obtain a copy of the Licence at <http://joinup.ec.europa.eu/software/page/eupl>.

Unless required by applicable law or agreed to in writing, software distributed under the Licence is distributed on an "**as is**"; basis, **without warranties or conditions of any kind**, either express or implied.

See the Licence for the specific language governing permissions and limitations under the Licence.