# .LOD format

The .LOD file format is used by Might and Magic 6, and probably other (later) games using the same engine. It is a simple container format used for all kinds of different data.

## Structure
.LOD-files contain a small header, directly followed by a global directory containing the file names and (offset, length) pairs for all files in the container. Immediately after the directory comes the file data, which, in most cases, is compressed with zlib. However, for the purpose of the .LOD file format, this compression is totally irrelevant and won't be covered here. Instead, refer to the individual file formats of the files, which are described under Might and Magic 6.

## Header
The header of a .LOD file has the following format:

    struct GHeader
    {
      char ID[4];
      char game[9];
      char unknown[256-13];
      char dir[16];
      unsigned long dirstart;
      unsigned long dirlength;
      unsigned long unknown2;
      unsigned long numFiles;
    };

where `ID` is the zero-terminated string LOD and game is either GameMMVI (in games.lod) or MMVI (all other MM6 .LOD files), padded with one or more 0-bytes.

`unknown` seems to contain a description of the container file and some other (probably random junk) data which has not yet been analyzed. `dir` seems to be the base directory of all files contained in this container. It is not yet clear if there's only one directory allowed, or if it is possible for a container to contain multiple directories. The reference unpacker (...) assumes that only one directory entry exists.

`dirstart` contains the offset of the first directory entry in the container file, while `dirlength` probably gives the length of all files in the current directory (i.e. the total size of one "directory" in the container file). In MM6, `dirstart + dirlength` always point to the end of the .LOD file. If there is any game which uses more than one directory inside a single .LOD file, `dirstart + dirlength` probably point towards the `dir` field for the next directory.

`unknown2` seems to be reserved (i.e. =0), and numFiles contains the number of files in this container (or rather in this directory for the current container)


## Directory
Each file stored in a .LOD container is indexed in a directory, the position of which is defined in the `dirstart` field of the global header. The following struct defines each file entry. Structs are packed directly one after another, one for each file:

    struct DirEntry
    {
      char name[16];
      unsigned long startoffs;
      unsigned long length;
      unsigned long u1;
      unsigned long u2;
    };

The first 16 bytes contain the ASCIIZ filename of the file (probably followed by junk after the terminating 0-byte), directly followed by an (offset, length) pair. The u1 and u2 fields seem to be reserved (=0).

