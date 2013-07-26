# Defiance Tools #
==============

## Overview ##
These are a series of unofficial command-line tools to extract various assets from the PC version of Defiance. 

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
Used to extract and convert texture assets to viewable PNG files. Only the largest mipmap is extracted. Run with -h to see usage. Cubemaps are also supported and will have a -N appended the end of the filename. Some textures are actually terrain data and dom't make a lot of sense when viewed. Some textures seem to be almost transparent because of how the alpha is used to separate the emissive parts. You may need to strip the alpha channel from the texture to see it properly.   

### mes2obj ###
Used to extract and convert static meshes and related texture assets to OBJ, MTL & PNG files. Only diffuse textures are supported. Run with -h to see usage. Some meshes reference textures that either don't exist in the WAD files or are dynamic/generated/animated textures. These meshes will be missing textures. 

### ski2obj ###
Used to extract and convert skinned meshes and related texture assets to OBJ, MTL & PNG files. Only diffuse textures are supported. Run with -h to see usage. All meshes are extracted including all the LoD meshes so you will end up with several meses of various detail stacked on top of each other. I may add an option to extract a specific LoD in the future. When viewing the meshes in Blender (2.66.1) the OBJ importer turns on alpha blending by default when it detects a texture with alpha. If you have a mesh with a texture assigned but you only see a mesh try disablng alpha blending for that texture. 
  
### wadlib ###
General library for reading and dumping asset records from WAD files. Useful if you want to write your own tools. The API is still unstable so excpect it to change. 


## Releases ##
###0.1.0###
Initial release.

###0.2.0###
Added support for skinned mesh extraction with ski2obj
Moved rmidlib code to wadlib
Cleaned up and standardized the usage for all the tools. Use -h to see usage.
Initial internal DDS export support. Cubemaps are not supported yet. 
