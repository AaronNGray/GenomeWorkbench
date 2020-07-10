# NCBI Genome Workbench 

Genome Workbench is also provided in source form. The source package is placed in the public domain, and is freely available for anyone to modify and redistribute, in either binary or source form. Details of the NCBI Public Domain License can be found here.

Genome Workbench makes use of the NCBI C++ Toolkit. A subset of the public toolkit source code is included in the Genome Workbench source package. While the latest versions of Genome Workbench can be found here, Genome Workbench is also available as part of the NCBI C++ Toolkit's public releases.

The essentials of the build process are documented as part of the NCBI C++ Toolkit. There are a couple of specific details that should be remembered when building Genome Workbench:

For Unix/Linux, a build script (build-linux-distro.sh) is provided in the root directory of the source tarball. This script is the script we use for building distributions in-house, and will automatically ensure that the wxWidgets libraries are built and installed side-by-side with the application. This dual installation ensures that the application will maintain binary compatibility through system upgrades. Please, check README.LINUX for more information for the Linux/Unix builds.
The build for MacOS X supports two versions of the build system: the command-line 'configure'-based solution, as described for Unix builds; and a build system for xCode (in compilers/xCode).
Genome Workbench requires a multithreaded environment.
In Unix / Linux, you should supply the --with-mt flag to 'configure'. This is implied by --with-gbench.

# Download

https://ftp.ncbi.nlm.nih.gov/toolbox/gbench/ver-3.4.1/gbench-src-3.4.1.tar.gz

# Missing file

The file src\algo\blast\proteinkmer\unit_test\data\nr_test.pki is missing due to GitHub's 100MB maximum file size.

