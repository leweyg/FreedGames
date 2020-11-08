
// Includable Object File Format
//
//This file contains the code needed to include IOFF models directly into C/C++ source code
// and contains an example rendering function for OpenGL
//
//Once you have included this file, you can include IOFF models into your app
// as easily as:
//	#include "FILENAME.ioff"
//
//The model will be global of type IncludedIOFF and will be named whatever name
//is given in the ioff file.
//
//

#ifndef Has_IOFF
#define Has_IOFF	1

struct IOFFModel
{
	int NumVertices;
	int NumPolygons;
	int NumVPP;
	GLenum VertType;
	void* Vertices;
	unsigned short* Indices;
};

struct IOFF_VertType_V2F 
{
	float vert_x, vert_y, vert_z;
};

struct IOFF_VertType_V3F 
{
	float vert_x, vert_y, vert_z;
};

struct IOFF_VertType_C3F_V3F
{
	float col_red, col_green, col_blue;
	float vert_x, vert_y, vert_z;
};

struct IOFF_VertType_C4F_N3F_V3F
{
	float col_red, col_green, col_blue, col_alpha;
	float norm_x, norm_y, norm_z;
	float vert_x, vert_y, vert_z;
};

struct IOFF_VertType_T2F_V3F
{
	float text_u, text_v;
	float vert[3];
};

#define IOFF_ModelName(name)	name

#define IOFF(name, nverts, npolys, nvpp, type) struct _tempstruct_##name {\
	int NumVertices, NumPolygons, NumVPP; GLenum VertType;\
	IOFF_VertType_##type myverts[nverts];\
	unsigned short myinds[npolys*nvpp]; };\
	_tempstruct_##name _tempmodel_##name = { nverts, npolys, nvpp, GL_##type, 

#define IOFF_END(name) };\
	IOFFModel IOFF_ModelName(##name) = { _tempmodel_##name.NumVertices, \
	_tempmodel_##name.NumPolygons, _tempmodel_##name.NumVPP, _tempmodel_##name.VertType, \
	&_tempmodel_##name.myverts[0], &_tempmodel_##name.myinds[0] } ;


void DrawIncludedIOFF(IOFFModel* ioff)
{
	GLenum type;
	switch(ioff->NumVPP)
	{
	case 1:
		type = GL_POINTS;
		break;
	case 2:
		type = GL_LINES;
		break;
	case 3:
		type = GL_TRIANGLES;
		break;
	case 4:
		type = GL_QUADS;
		break;
	default:
		return;
	}

	glInterleavedArrays( ioff->VertType, 0, ioff->Vertices );
	glDrawElements( type, ioff->NumPolygons*ioff->NumVPP, 
		GL_UNSIGNED_SHORT, ioff->Vertices );
}

#endif

//Here is the general IOFF format:
/*
IOFF( model_name, number_of_vertices, number_of_indexed_polygons, 
	vertices_per_polygon, vertex_type )
{
	{ vertex_data },
	{ vertex_data },
	...,
}
,
{
	index1, index2, ..., indexVPP,
	...,
}
IOFF_END( model_name );
*/

//Here is an example IOFF File:
//This is a cube made of triangles with a pink colour:
/*
IOFF( Cube, 8, 12, 3, C3F_V3F )
{
	{ 1, 0.5, 0.5, -1, -1, -1},
	{ 1, 0.5, 0.5, 1, -1, -1},
	{ 1, 0.5, 0.5, 1, 1, -1},
	{ 1, 0.5, 0.5, -1, 1, -1},
	{ 1, 0.5, 0.5, -1, -1, 1},
	{ 1, 0.5, 0.5, 1, -1, 1},
	{ 1, 0.5, 0.5, 1, 1, 1},
	{ 1, 0.5, 0.5, -1, 1, 1},
},
{
	2, 1, 0, 
	0, 3, 2, 
	0, 4, 7, 
	7, 3, 0, 
	4, 5, 6, 
	6, 7, 4, 
	3, 7, 6, 
	6, 2, 3, 
	5, 4, 0, 
	0, 1, 5, 
	6, 5, 1, 
	1, 2, 6, 
}
IOFF_END(Cube)
*/