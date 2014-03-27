(c) Intel 
May 23, 2013

This is an unofficial version of our hand skeleton tracking library. 
Permission and conditions for usage is granted under terms of the accompanying
license.txt file or agreement you signed with Intel if applicable.

More specifically, the included software is the result of a resarch effort and
is not an official product and is currently independent from any Intel product
roadmap. 

Support and questions should be directed to sterling.g.orsten@intel.com or 
stan.k.melax@intel.com. Information about the underlying technology can be
found in the i3D abstract and poster, and the full paper in Graphics Interface
2013.

The library uses the PerC SDK and the Creative Gesture Camera. To get these
pre-requisites, visit Intel's Perceptual Computing Website:
  http://www.intel.com/software/perceptual 
and download the PerC SDK public release, and install it. Ensure that the
camera and PerC SDK samples are already working.

To test the library, try running /hskl/bin/Win32/HandTrackViewerRelease.EXE.
Note that it defaults to tracking the right hand. Ensure nothing else is in
the field of view of the camera. Until you get a feel for what motions tend to
work and what tends to break, enter your hand from the back with fingers
spread and move fingers and hand slowly. Pull hand back to reset if tracking
seems broken.

Tracking may tend to be more accurate when hand is closer to camera since
there will be more depth pixels to use for fitting and deriving the pose. It
may help to adjust the hand model scaling to fit the user's hand size. The
default assumed hand size is about 19cm long (wrist to mid-finger-tip) and
about 9 cm across the palm (just below knuckles, not including thumb).

While work is ongoing to improve camera quality and software robustness in
general, there may be targetted improvements that could be made for specific
use cases. Your feedback is welcome.

The source code and visual studio project files for the samples is provided.
These .cpp files and the .h API to the tracking library were kept as small as
possible to hopefully provide a low learning curve for integration into your
application.

For your convenience, we have added a small C++ wrapper layer, in header file
hsklu.h, that will initialize and connect the PerC SDK and input camera depth
data to the tracking library.

While the tracking library is based on depth data, we do not offer any support
for any devices other than Creative's Gesture Camera or APIs other than
Intel's Perceptual Computing API.

Sample Overview
===============

HandTrackViewer demonstrates basic hand tracking using the depth feed from a
3D camera, while visualizing the results using a 3D model rendered via OpenGL
over top of the infrared feed from the same camera. 

The hand model can be manually resized to fit the user using W/S to modify the
length of the hand and fingers, and A/D to modify the width and breadth.

PhysicsInteraction demonstrates using powered-ragdoll technique to "drive" a
virtual hand in a physics-based interactive environment.