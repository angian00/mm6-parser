#include "blv.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compression.h"
#include "geometry.h"


//--------------------------------------------------------------
//--- Data structures
//--------------------------------------------------------------

//----   Gamefile data format  ----------

struct blv_v6_file_header
{
    uint32_t compressed_size;
    uint32_t uncompressed_size;
//  unsigned char data[];
};


struct blv_internal_header
{
    uint32_t unknown1;       // maybe flags?
    char description[76];    // maybe it's actually smaller and is followed by other data
    char unknown2[24];       // some kind of reference? short-name for the level?
    int32_t wall_vertex_size;
    int32_t r_datasize;
    int32_t rl_datasize;
    int32_t unksize2;
    int32_t unknown3[4];
};


struct vertex_section
{
    uint32_t count;
    //struct vertex vertices[];
};

struct vertex
{
  int16_t x, y, z;
};


struct wall_section
{
    uint32_t count;
    //struct wall walls[];
};

struct wall
{
    int32_t i_normal_x,i_normal_y,i_normal_z,i_dist;   // plane by normal vector and point, all numbers multiply by 65536, so i_normal_x=int(normal_x*65536.0)
    int32_t zcalc1;   // zcalc1 = -(i_normal_x << 16) / i_normal_z    (or 0 in case i_normal_z == 0)
    int32_t zcalc2;   // zcalc2 = -(i_normal_y << 16) / i_normal_z    (or 0 in case i_normal_z == 0)
    int32_t zcalc3;   // zcalc3 = -(i_dist << 16) / i_normal_z  (or 0 in case i_normal_z == 0)
    int32_t bits;   // various attributes, see below
    int32_t ingame_pointers[6]; //pointers on wall vertex data, filled by game on load(data in blv file not used)
    int16_t face_param_index;       // index in FaceParams array
    int16_t unknown3;
    int16_t sector_index[2]; //the indices of sectors(sectors section) in which is located the wall
    int16_t xmin, xmax;
    int16_t ymin, ymax;
    int16_t zmin, zmax;
    char orientation_type;        // 5 - ceiling, 6 - in-between ceiling and wall, 1 - vertical wall, 4 - in-between floor and wall, 3 - floor
    unsigned char num_vertices;
    int16_t unknown4;       // always zero (alignment?)
};


/*
struct wall_vertices
{
        uint16_t vertex[n+1];  // indexes in vertices[] from vertex_section, which describe the wall boundary
        int16_t x[n+1]; // not sure if this is actually vector (normals?) data.
        int16_t y[n+1]; // 
        int16_t z[n+1]; //
        int16_t x_off[n+1]; //Texture x-offset
        int16_t y_off[n+1]; //Texture y-offset
};
*/

/*
struct texture textures[num_walls];
*/

struct texture {
    char name[10];
};


struct face_section
{
    uint32_t count; // one item size 0x24

    // struct face_param_data face_params[];
    // struct face_param_data_2 face_params2[];

};

struct face_param_data
{
    int16_t  unk1[6];
    int16_t  someIndex;  // maybe index into texture list?
    int16_t  unk2;       // always -1 ?
    int16_t  unk3[2];
    int16_t  dx, dy; //texture deltas (off 0x14)
    int16_t  unk4;
    uint16_t event_n; // # of event in map evt file
    int16_t  unk5[4];   // always 0 ?
};

struct face_param_data_2
{
    char unknown[10]; //may be name for second texture(usualy filled by zero)
};



struct room_section
{
    uint32_t count; 
    //struct room sectors[];

    //char sectors_rdata[Header.Rdatasize];
    //char sectors_rldata[Header.RLdatasize];
};

struct room
{
   char unk[0x74];
};

//TODO: door.count


struct object_section
{
    uint32_t num_unknown;
    uint32_t count;
    //struct obj_unk object_data[num_objects];
    //struct object names[num_objects];
};

struct obj_unk
{
    int16_t unk1;
    int16_t unk2;
    int16_t unk3[12];
};

struct object
{
    char name[16]; //ex. Torch1?
    int16_t unk1[4];
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t unk2;
};

struct light_section
{
    uint32_t count;
    //struct light lights[n];
};

struct light
{
    int16_t x, y, z;
    int16_t unk1; //usualy 0
    int16_t unk2; //usualy 31
    int16_t level; //light brightness
};

struct bsp_node_section
{
    uint32_t count;
    //struct bsp_node bsp_node[];
};

struct bsp_node
{
    uint16_t unknown[4];
};


struct spawn_section
{
    uint32_t count;
    //struct spawn spawns[];
};

struct spawn
{
   char unknown[0x14];    
};


struct outline_section
{
    uint32_t count;
    //struct outline outlines[];
};

    struct outline
    {
       //int16_t x1,y1,x2,y2,z,unk;
        uint16_t i_v1, i_v2, i_w1, i_w2, z, bits;
    };


//----   Parsed format  ----------

struct blv_data
{
    uint32_t size;
    uint8_t *buffer;

    struct parsed_info
    {
		struct blv_internal_header *p_blv_header;
		struct vertex_section *p_vertex_section;
		struct vertex *vertices;
		struct wall_section *p_wall_section;
		struct wall *walls;
		struct texture *textures;
		struct face_section *p_face_section;
		struct face *faces;
		struct room_section *p_room_section;
		struct room *rooms;
		struct object_section *p_object_section;
		struct object *objects;
		struct light_section *p_light_section;
		struct light *lights;
		struct bsp_node_section *p_bsp_node_section;
		struct bsp_node *bsp_nodes;
		struct spawn_section *p_spawn_section;
		struct spawn *spawns;
		struct outline_section *p_outline_section;
		struct outline *outlines;
	} parsed_info;
};


//----------------------------------------------------------------------
//--- Internal function declarations
//----------------------------------------------------------------------

void blv_parse_fields(struct blv_data* p_blv);

//----------------------------------------------------------------------
//--- Implementation
//----------------------------------------------------------------------

struct blv_data *blv_parse(char *raw_data)
{
	char *p_data = raw_data;
	struct blv_v6_file_header blv_header;
	
	memcpy(&blv_header, p_data, sizeof(struct blv_v6_file_header));
	p_data += sizeof(struct blv_v6_file_header);


	unsigned char compr_data[blv_header.compressed_size];
	memcpy(compr_data, p_data, blv_header.compressed_size);
	p_data += blv_header.compressed_size;

	struct blv_data *p_blv = malloc(sizeof(struct blv_data));
	p_blv->size = blv_header.uncompressed_size;
	
 	p_blv->buffer = z_uncompress(compr_data, blv_header.compressed_size, blv_header.uncompressed_size);

	blv_parse_fields(p_blv);

 	return p_blv;
}




void blv_parse_fields(struct blv_data* p_blv)
{
 	void *curr_pos = p_blv->buffer;
 	struct parsed_info *pi = &p_blv->parsed_info;

 	pi->p_blv_header = (struct blv_internal_header *)curr_pos;
 	curr_pos += sizeof(struct blv_internal_header);

 	pi->p_vertex_section = (struct vertex_section *)curr_pos;
 	curr_pos += sizeof(struct vertex_section);
 	pi->vertices = (struct vertex *)curr_pos;
 	curr_pos += pi->p_vertex_section->count * sizeof(struct vertex);

 	pi->p_wall_section = (struct wall_section *)curr_pos;
 	curr_pos += sizeof(struct wall_section);
 	pi->walls = (struct wall *)curr_pos;

 	curr_pos += pi->p_wall_section->count * sizeof(struct wall);
 	curr_pos += pi->p_blv_header->wall_vertex_size;

  	pi->textures = (struct texture *)curr_pos;
	curr_pos += (sizeof(struct texture) * pi->p_wall_section->count);

	//curr_pos += (sizeof(struct wall_vertex) * pi->p_wall_section->count);

 	pi->p_face_section = (struct face_section *)curr_pos;
 	curr_pos += sizeof(struct face_section);
 	pi->faces = (struct face *)curr_pos;
 	curr_pos += pi->p_face_section->count * (sizeof(struct face_param_data) + sizeof(struct face_param_data_2));

 	pi->p_room_section = (struct room_section *)curr_pos;
 	curr_pos += sizeof(struct room_section);
 	pi->rooms = (struct room *)curr_pos;
 	curr_pos += pi->p_room_section->count * sizeof(struct room);
 	curr_pos += pi->p_blv_header->r_datasize;
 	curr_pos += pi->p_blv_header->rl_datasize;

 	pi->p_object_section = (struct object_section *)curr_pos;
 	curr_pos += sizeof(struct object_section);
	curr_pos += pi->p_object_section->count * sizeof(struct obj_unk);
  	pi->objects = (struct object *)curr_pos;
	curr_pos += pi->p_object_section->count * sizeof(struct object);

 	pi->p_light_section = (struct light_section *)curr_pos;
 	curr_pos += sizeof(struct light_section);
 	curr_pos += pi->p_light_section->count * (sizeof(struct light));

 	pi->p_bsp_node_section = (struct bsp_node_section *)curr_pos;
 	curr_pos += sizeof(struct bsp_node_section);
 	pi->bsp_nodes = (struct bsp_node *)curr_pos;
 	curr_pos += pi->p_bsp_node_section->count * (sizeof(struct bsp_node));

 	pi->p_spawn_section = (struct spawn_section *)curr_pos;
 	curr_pos += sizeof(struct spawn_section);
 	pi->spawns = (struct spawn *)curr_pos;
 	curr_pos += pi->p_spawn_section->count * (sizeof(struct spawn));

 	pi->p_outline_section = (struct outline_section *)curr_pos;
 	curr_pos += sizeof(struct outline_section);

 	pi->outlines = (struct outline *)curr_pos;
 	//curr_pos += (sizeof(struct outline) * pi->p_outline_section->count);
}


void blv_extract_outlines(struct blv_data *p_blv, uint32_t *p_n_lines, struct point **p_lines)
{
	struct parsed_info *pi = &p_blv->parsed_info;
	
	*p_n_lines = pi->p_outline_section->count;
	*p_lines = malloc(sizeof(struct point)*2*(*p_n_lines));

 	for (uint32_t i=0; i < *p_n_lines; i++) {
 		struct outline o = pi->outlines[i];

		if ( (o.i_v1 == 0) && (o.i_v2 == 0) ) {
			//ignore
	 		continue;
		}

		struct vertex v1 = pi->vertices[o.i_v1];
		struct vertex v2 = pi->vertices[o.i_v2];

 		(*p_lines)[i*2].x = v1.x;
 		(*p_lines)[i*2].y = v1.y;
 		(*p_lines)[i*2+1].x = v2.x;
 		(*p_lines)[i*2+1].y = v2.y;
 	}
}