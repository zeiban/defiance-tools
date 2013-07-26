# Defiance Tools #
==============

## Overview ##
These are a series of tools to extract various assets from the PC version of Defiance. If you are just curious about the WAD archive and it's internal RMID asset formats take a look in the wadlib/wadf.h and wadlib/rmid.h files. The code is quite a mess while I figure out the formats dispite rewriting everything several times. 

## Compiling ##
All the tools were created using Visual C++ Exress 2010. If you have that you should be good to go. All the depenancies like zlib and libpng are included in the solution. 

## Tools ##
### waddiff ###
Compares 2 directories containing WAD files and generates a CSV file that exaplains what was added, deleted and changed. Very useful if you want to know what changed between client patches.  

### waddump ###
Used to dump individual assets from WAD files. It will decompress any compressed assets as well. Useful for extracting assets for research but doesn't convert. Run with -h to see usage. 

### snd2wav ###
Used to extract and convert audio assets to playable WAV files. Run with -h to see usage. 

### tex2png ###
Used to extract and convert textures assets to viewable PNG files. Only the largest mipmap is extracted. Run with -h to see usage.

### mes2obj ###
Used to extract and convert static meshes and related texture assets to OBJ, MTL & PNG files. Run with -h to see usage.

### ski2obj ###
Used to extract and convert skinned meshes and related texture assets to OBJ, MTL & PNG files. Run with -h to see usage.
  
### wadlib ###
General library for reading and dumping asset records from WAD files. 


## Releases ##
###0.1.0###
Initial release

###0.2.0###
Added support for skinned mesh extraction with ski2obj
