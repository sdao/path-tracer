path-tracer
===========

Little global illumination Monte Carlo path tracer

Ready to build on either Windows or Linux

**This repository includes submodules.** In order to get everything when you
clone, add the `--recursive` flag.

[![Build Status](https://semaphoreapp.com/api/v1/projects/f17e953b-83a4-4364-a2e3-04e1f126be21/317820/shields_badge.svg)](https://semaphoreapp.com/sdao/path-tracer)

Dependencies
------------
These are the dependencies on both Windows and Linux:
* [Eigen](http://eigen.tuxfamily.org/)
  (template library)
* [Open Asset Import Library](http://assimp.sourceforge.net/)
  (links with libassimp)
* [Boost](http://www.boost.org/)
  (template libraries; also links with libboost_program_options)
* [TinyExr](https://github.com/syoyo/tinyexr)
  (header-only library)
* [Intel Embree](http://embree.github.io/)
  (links with libembree)

On Linux, Intel TBB is required:
* [Intel Threading Building Blocks](https://www.threadingbuildingblocks.org/)
  (links with libtbb)

Windows Build (Visual Studio)
-----------------------------
You will need VS 2015 because the project uses C++11 language features only
available in VS 2015+.

_Eigen_, _Boost_, and the _Open Asset Import Library_ can be obtained from
NuGet; the project should automatically update the packages on first load.
_TinyExr_ is included in the project as a Git submodule and you will get the
master version when you clone this repository.

You will need to install Embree separately by running the Windows x64 installer
from [their downloads page](https://embree.github.io/downloads.html). The
solution will look for Embree at `$(ProgramW6432)\Intel\Embree v2.7.1 x64`,
the default installation location for Embree 2.7.1. (Note that `$(ProgramW6432)`
is the VS macro for the 64-bit Program Files directory.)

If you install a different version of Embree or you install it at a different
location, edit the `PathTracerDependencies.props` file at the root of the
repository to point to your Embree installation.

Linux Build (Makefile)
----------------------
You should build using `g++`. Using `clang++` is not guaranteed to work.

You will need to get the development versions of `eigen3`, `assimp`, `boost`,
and `tbb` from your package manager (e.g. from `apt-get` or from `dnf`).
_TinyExr_ is included in the project as a Git submodule and you will get the
master version when you clone this repository.

You will need to install Embree separately from
[their downloads page](https://embree.github.io/downloads.html). Red Hat-based
users can use the RPM installer; Debian-based users should get the tarball.

Reference
---------
Some of the comments make reference to Pharr and Humphreys' text
[*Physically Based Rendering*](pbrt.org), 2nd edition (Morgan Kaufmann, 2010).
This excellent resource describes the `pbrt` renderer in a literate
programming style.

My renderer is also influenced by Kevin Beason's
[smallpt](http://www.kevinbeason.com/smallpt/) path tracer.

Renderings
----------
![Cornell box](https://raw.githubusercontent.com/wiki/sdao/path-tracer/pathtracer_dragon_800iters_30min.png)

![Spheres with depth of field](https://raw.githubusercontent.com/wiki/sdao/path-tracer/pathtracer_spheres_800iters_20min.png)
