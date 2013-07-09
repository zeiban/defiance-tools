# Defiance Tools #
==============

## Overview ##

These are a series of tools to extract various assets from the PC version of Defiance. If you are just curious about the WAD archive and it's internal RMID asset formats take a look in the wadlib/wadf.h and rmidlib/rmid.h files. The tools have all be developed with Visual Studio 2010 and the solution/projects are included. A lot of the code is very Windows specific so porting would take some effort.   


## Tools ##
### waddiff ###
Compares 2 directories containing WAD files and generates a CSV file that exaplains what was added, deleted and changed. Very useful if you want to know what changed between client patches.  

### waddump ###
Used to dump individual assets from WAD files. It will decompress any compressed assets as well. Useful for extracting assets for research but doesn't convert.  

### snd2wav ###
Used to extract and convert audio (Type 9) assets to playable WAV files.  

### tex2png ###
Used to extract and convert textures (Type 3) assets to playable PNG files. 

### mes2obj ###
Used to extract and convert static meshes (Type 4) and related texture assets to OBJ, MTL & PNG files. 

### ski2obj ###
Used to extract and convert skelatal meshes (Type 7) and related texture assets to OBJ, MTL & PNG files. 
  
## Libraries ##
### wadlib ###
Library for reading and dumping asset records in WAD files of the client. Mostly used in conjunction with rmidlib. 

### rmidlib ###
Library for reading the internal RMID files inside the WAD files. Also supports decompression of compressed containers. 

