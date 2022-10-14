# .BLV

.BLV files are used in Might and Magic 6 to store the dungeon level geometry. The format used for Might and Magic 7 is probably similar.

## Compression
To save space, a .BLV file is zlib-compressed using a very simple format (i.e. the file does NOT start with the MM6 Compressed Header)

    struct CompressedBLV6header
    {
      unsigned long compressedSize;
      unsigned long uncompressedSize;
      unsigned char data[];
    };
    struct CompressedBLV7header
    {
      unsigned char id1[4];//0x41 0x67 0x01 0x00
      unsigned char id2[4];//'mvii'
      unsigned long compressedSize;
      unsigned long uncompressedSize;
      unsigned char data[];
    };

The `compressedSize` and `uncompressedSize` fields should be self-explanatory. Simply read `compressedSize` bytes from the .BLV file (normally that means the entire file, excluding the first 8 bytes(16 bytes for mm7 and mm8)), allocate a buffer of size `uncompressedSize`, and then call the zlib function `uncompress()` to decompress the whole file in one go.

## Structure
After decompression, a .BLV file is divided into different sections, each of which is explained below. All sections follow directly after each other, there's (apparently) no offset table or similar structure.

### Header
The header is 0x88 bytes long and contains a description and some other (yet unknown) information for the level.

    struct Header
    {
      uint32 unknown1;         // maybe flags?
      char description[76];    // maybe it's actually smaller and is followed by other data
      char unknown2[24];       // some kind of reference? short-name for the level?
      int32 wall_vertex_size,Rdatasize,RLdatasize;
      int32 unksize2;
      int32 unknown3[4];       
    };

### Vertex data
This section starts at offset 0x88 and contains the vertex data for the level. The first uint32 gives the number of vertices, directly followed by the vertex data as triplets of signed shorts, containing x, y and z-coordinate.

    struct VertexSection
    {
      uint32 numVertices;

      struct Vertex
      {
        int16 x, y, z;
      } Vertices[];
    };

The vertices should be numbered from 0 to `numVertices-1`, as that is how they are referenced later in the wallvertex section.

### Wall data
This section contains one entry for each wall or portal in the level, which contains a bounding box, the number of vertices for that face, and some unknown fields (probably animation related). Portals used for rendering.


    struct WallData
    {
      uint32 numWalls;

      struct Wall
      {
    #ifndef MM6
        float normal_x,normal_y,normal_z,dist;   // plane by normal vector and dist from (0,0,0) //in mm7 and mm8, dont exist in mm6
    #endif
        int32 i_normal_x,i_normal_y,i_normal_z,i_dist;   // plane by normal vector and point, all numbers multiply by 65536, so i_normal_x=int(normal_x*65536.0)
        int32 zcalc1;   // zcalc1 = -(i_normal_x << 16) / i_normal_z    (or 0 in case i_normal_z == 0)
        int32 zcalc2;   // zcalc2 = -(i_normal_y << 16) / i_normal_z    (or 0 in case i_normal_z == 0)
        int32 zcalc3;   // zcalc3 = -(i_dist << 16) / i_normal_z  (or 0 in case i_normal_z == 0)
        int32 bits;   // various attributes, see below
        int32 ingame_pointers[6]; //pointers on wall vertex data, filled by game on load(data in blv file not used)
        int16 face_param_index;       // index in FaceParams array
        int16 unknown3;
        int16 sector_index[2]; //the indices of sectors(sectors section) in which is located the wall
        int16 xmin, xmax;
        int16 ymin, ymax;
        int16 zmin, zmax;
        char orientationType;        // 5 - ceiling, 6 - in-between ceiling and wall, 1 - vertical wall, 4 - in-between floor and wall, 3 - floor
        char numVertices;
        int16 unknown3;       // always zero (alignment?)
      } walls[];
    };

The `xmin, xmax, ymin, ymax, zmin` and `zmax` fields contain a bounding box around the wall which was probably needed by the software renderer used in the game. And `numVertices` contains the number of vertices for this face, which are listed in the next section. Faces should also be counted from 0 to `numFaces`. The bits is a combination of values of which the following are known:

    0x00000001 - It's a portal.
    0x00000010 - The texture is wavy, like water.
    0x00002000 - Invisible.
    0x00040000 - For facets that can be moved (are a part of door or lever). If set, the texture is moved accordingly.
    0x00400000 - The texture moves slowly. For horizontal facets only.
    0x02000000 - Event can be triggered by clicking on the facet.
    0x04000000 - Event can be triggered by stepping on the facet.
    0x20000000 - Untouchable You can pass through it.

### Wall vertex list
This section assigns the wall data above to the correct vertices. It follows directly after the previous section, this time without any header or length info at the beginning(length of this section in blv header, field `wall_vertex_size`), just the raw data. That is possible because all relevant information is already known from the Wall data section.

Because walls are polygons, they must be drawn as a closed loop through some number of (coplanar) vertices. The key is the word closed here, which means that for a wall which has n vertices, there are actually n+1 vertex indices in this section, where the last index is always the same as the first one.

Example: Suppose we have a wall with 12 vertices (as described in it's entry in the wall data section). This means that for this wall, we would have 13 entries here, for example 0x0f, 0x15, 0x01, 0x1a, 0x1a, 0x1c, 0x1c, 0x02, 0x18, 0x11, 0x0d, 0x0b, 0x0f. This is an actual example, taken from the level zddb02.blv (the first wall). I don't know why certain points appear two times in a row, I'm guessing it has something to do with normals or textures (see below for more guesses in that direction ;-). Important thing is that the last vertex is the same as the first one (0x0f).

So we have to read `n+1` vertex indices for each wall. Directly after that, there is more data, which also seems to be repeated n+1 times. Here's the structure which describes this, in pseudo-code:

    struct WallVertices
    {
      uint16 vertex[n+1];  // indexes in Vertices[] from VertexSection,which describe the wall boundary
      int16 x[n+1]; // not sure if this is actually vector (normals?) data.
      int16 y[n+1]; // 
      int16 z[n+1]; //
      int16 x_off[n+1]; //Texture x-offset
      int16 y_off[n+1]; //Texture y-offset
    }

### Texture section
This section describes all textures used in the level. Each entry in the wall data section (see above) has one corresponding entry in this list. For each wall data entry, there's a 10-byte ASCIIZ string here describing the texture used by this particular wall. The string gives the filename of a .TGA file in the BITMAPS.LOD container file.

Portals have emtry string name.

### FaceParams section
This is another section which contains a counter:

    struct FaceParams
    {
      uint32 count; // one item size 0x24
      struct FaceParamData
      {
        int16  unk1[6];
        int16  someIndex;  // maybe index into texture list?
        int16  unk2;       // always -1 ?
        int16  unk3[2];
        int16 dx,dy;//texture deltas (off 0x14)
        int16 unk4;
        uint16 event_n;// # of event in map evt file
        int16 unk5[4];   // always 0 ?
      } FaceParams[];//count items
      struct FaceParam2
      {
        char unknown[10]; //may be name for second texture(usualy filled by zero)
      } FaceParams2[];//count items

    };

Texture coordinates calculated as:

    u= (wall_vertex.x_off+faceParam.dx)/texture.Width;
    v= (wall_vertex.y_off+faceParam.dy)/texture.Height;

### Sectors section
BLV map consists of the sectors of those connected by portals. This section consists of 3 parts, Sectors[] array, sectorsRdata, sectorsRLdata, witch follow directly after each other.

    struct SectorsSection
    {
      uint32 count; 
      struct Sector
      {
    #ifdef MM8
        char unk[0x78]; 
    #else
        char unk[0x74]; 
    #endif
      } Sectors[];
      char sectorsRdata[Header.Rdatasize];
      char sectorsRLdata[Header.RLdatasize];
    };

### Object section
This section describes the objects found in the dungeon. It starts directly after the Sectors section and has the following format:

    struct Object
    {
      uint32 numUnknown;
      uint32 numObjects;
      struct Unk
      {
        int16 unk1;
        int16 unk2;
        int16 unk3[12];
    #ifdef MM7
        int16 unk4[2];
    #endif
    #ifdef MM8
        int16 unk4[2];
    #endif
      } objectData[numObjects];
      struct Name
      {
        char name[16];
        int16 unk1[4];
        int16 x;
        int16 y;
        int16 z;
        int16 unk2;
      } names[numObjects];
    };

### LightSources section
This section has the following format:

    struct LightSources
    {
      uint32 count;
      struct light
      {
        int16 x,y,z;
        int16 unk1;//usualy 0
        int16 unk2;//usualy 31
        int16 level;//light brightness
    #ifdef MM7
        int16 unk4[2];//color i think
    #endif
    #ifdef MM8
        int16 unk4[4];//color and something else
    #endif
      } lights[n];
    };

### Unknown D section
    struct UnknownD
    {
      uint32 count;
      struct Unknown
      {
        uint16 unknown[4];
      } unknown[];
    };

### Spawn section
Information about monsters and may be NPC.

    struct Spawn
    {
      uint32 count;
      struct SpawnSection
      {
    #ifdef MM6
    	char unknown[0x14];
    #else
    	char unknown[0x18];
    #endif
        
      } spawns[];
    };

### Map outlines section
Information about lines for drawing 2d map in right upper corner.

    struct MapOutlines
    {
      uint32 count;
      struct outline
      {
        int16 x1,y1,x2,y2,z,unk;
      } outlines[];
    };
