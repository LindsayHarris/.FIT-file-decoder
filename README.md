# .FIT-file-decoder
A framework to decode .FIT files.   Currently only decodes GPS information, and tested only on a Garmin Virb Ultra 30 action camera.

It should be fairly straightforward to write functions to decode other records within the file.  There are several decode functions included in the code here.    You will want/need to download the SDK from Dynastream Innovations for the format/encoding of other records.

Dynastream offers the SDK here:- https://www.thisisant.com/resources/fit
That pages requires you to accept their conditions for accessing the SDK.   They are not onerous conditions.

The -d flag accepts parameters to print various parts of the file, undecoded!

Building the code involves running the 'mkit' script - the files are small and compile quickly.  Hence I did not bother with Makefile or other building tools.
