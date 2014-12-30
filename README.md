path-tracer
===========

Little global illumination Monte Carlo path tracer

Dependencies
------------
If you're on OS X, I suggest using [Homebrew](http://brew.sh/) to get the right
dependencies. If you're on Linux, you can use your package manager.

* [Eigen](http://eigen.tuxfamily.org/)
  (template library)
* [Intel Threading Building Blocks](https://www.threadingbuildingblocks.org/)
  (links with libtbb)
* [OpenEXR libraries](http://openexr.com/)
  (links with libIlmImf and libHalf)
* [Open Asset Import Library](http://assimp.sourceforge.net/)
  (links with libassimp) 
* [Boost.MultiArray](http://www.boost.org/)
  (part of the Boost template libraries)

Reference
---------
Some of the comments make reference to Pharr and Humphreys' text
[*Physically Based Rendering*](pbrt.org), 2nd edition (Morgan Kaufmann, 2010).
This excellent resource describes the `pbrt` renderer in a literate
programming style.

My renderer is also influenced by Kevin Beason's
[smallpt](http://www.kevinbeason.com/smallpt/) path tracer.
