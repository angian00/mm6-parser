# .ODM format
.ODM files are used in Might and Magic 6 to store the outdoor map level geometry. The format used for Might and Magic 7 and Might and Magic 8 is a slightly modified version with more fields.


## Compression
TODO: I don't remember if the .ODM file is compressed the same way as the .BLV file. We'll need to double-check this. To save space, a .ODM file is zlib-compressed using a very simple format (i.e. the file does NOT start with the MM6 Compressed Header)

    struct CompressedODM6header
    {
      unsigned long compressedSize;
      unsigned long uncompressedSize;
      unsigned char data[];
    };
    struct CompressedODM7header
    {
      unsigned char id1[4];//0x41 0x67 0x01 0x00
      unsigned char id2[4];//'mvii'
      unsigned long compressedSize;
      unsigned long uncompressedSize;
      unsigned char data[];
    };

The `compressedSize` and `uncompressedSize` fields should be self-explanatory. Simply read `compressedSize` bytes from the .ODM file (normally that means the entire file, excluding the first 8 bytes(16 bytes for mm7 and mm8)), allocate a buffer of size `uncompressedSize`, and then call the zlib function uncompress() to decompress the whole file in one go.

## Structure
After decompression, a .ODM file is divided into different sections, each of which is explained below. All sections follow directly after each other, there's (apparently) no offset table or similar structure.

### Header
The header is x bytes long and contains a description and some other (yet unknown) information for the level.

    struct ODM
    {
     // header
     unsigned char  blank[32];  // map name -- normally left blank /// Probably not used by Engine
     unsigned char  defaultOdm[32];        // byte[32] @ 000020 // filename of map -- normally "default.odm" /// Probably not used by Engine
     unsigned char  editor[32]; // byte[32] @ 000040 // editor version string /// Probably not used by Engine  // in mm8, 31 bytes, master tile is last byte
     unsigned char  sky_texture[32];   /// Probably not used by Engine
     unsigned char  ground_texture[32];  /// Probably not used by Engine
     TilesetSelector tileset_selector[3]; 
     TilesetSelector road_tileset; '''''TODO: section on tileset selector.   short group, id. See BDJ tutorial.'''''
     int attributes; /// Only exists in MM8
      
     // coordinate maps
     char heightMap[128*128];
     char tileSetMap[128*128];
     char attributeMap[128*128];
     Shading shadingMap[128*128]; // two chars each /// Only exists in MM7 and MM8
     
      short width;            // width /// Only exists in MM7 and MM8
      short height;           // height /// Only exists in MM7 and MM8
      short width2;           // width /// Only exists in MM7 and MM8
      short height2;          // height /// Only exists in MM7 and MM8
      int unknown; 				/// Only exists in MM7 and MM8
      int unknown;     			/// Only exists in MM7 and MM8
              
     int bModelCount; // number of 3d model data sets
     BModel *bmodels;
             
     int SpriteCount; // number of billboard objects, 2d images in 3d space
     Sprite *sprites;
     
     // Sprite location id list and location by tile map
     int idDataCount; // number of idDataEntries in the list
     short idDataList[idDataCount];
     int idListAtCoordinateMap[128*128];

     int SpawnPointCount; // number of spawn points (monsters)
     SpawnPoint *spawnPoints;
     
    }; 

### Tile Set Selector
TODO: This section hasn't been finished yet

    struct TilesetSelector
    {
      short group;
      short dtileIdNumber;
    };

TODO: See BDJ tutorial.

### Height Map
TODO: This section hasn't been finished yet

    // 0 is low point.  Multiply by 32 to get game coordinate height.
    char heightMap[128*128];

### Tile Set Map
TODO: This section hasn't been finished yet

    char tileSetMap[128*128];

### Attribute Map
TODO: This section hasn't been finished yet

    char attributeMap[128*128];

### Shading Map
TODO: This section hasn't been finished yet

    struct Shading
    {
    char unknown;
    char unknown;
    }

    Shading shadingMap[128*128]

### BModel
TODO: This section hasn't been finished yet

    struct BModelOtherStuff
    {
    vertex data
    Face data
    BspNode data /// Only exists in MM7 and MM8?
    }

    BModel bModelData[bModelCount]
    BModelOtherStuff[bModelCount];

### Sprite
...

### Sprite Location
...

### Spawn Point
...


...
...
