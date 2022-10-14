# Might and Magic 6
Might and Magic VI (and probably later ones) use various file formats for different things.

Videos are stored in the `Anims` subdirectory, packed together into simple *.VID* archives. They are standard Smacker(TM) video files which can be played or converted with the RadGameTools for example.

In the `Sounds` directory, there is a single `AUDIO.SND` file which is an archive of all ingame sound and music. For details, see *.SND*.

The `Data` subdirectory has four [.LOD](LOD_format.md) files in it. `BITMAPS.LOD` probably stores all ingame textures and images, `SPRITES.LOD` seems to contain all enemies and other sprites, including animation. `ICONS.LOD` contains inventory items and `GAMES.LOD` contains all level data. The file format of these archives is very straightforward, for details refer to [.LOD](LOD_format.md).

The following file types are contained in the above mentioned archives:

* *.SMK* - Standard Smacker(TM) videos in `anims/*.vid`
* Compressed [.TGA](TGA_format.md) files without extension in `data/bitmaps.lod` and `data/icons.lod`. They seem to contain multiple images each, with different levels of detail.
* .BIN, .EVT, .TXT, *.STR*, .FNT files in `data/icons.lod`, each with some kind of compression (to be determined. It's probably the same as the compression used for the level data)
* [.BLV](BLV_format.md) Dungeon level data for the 3D dungeons in `data/games.lod`
* [.ODM](ODM_format.md) Outdoor level data in `data/games.lod`
* *.DDM* Outdoor level item data `data/games.lod`
* *.DLV* Dungeon level item data for the 3D dungeons in `data/games.lod`

Most of these files are zlib-compressed and have a standard-header which is described in [MM6 Compressed Header](MM6_header_format.md).
