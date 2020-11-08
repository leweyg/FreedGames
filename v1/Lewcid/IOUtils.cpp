#include <fstream.h>

/*
ResMesh* IO_Mesh_CreateFromOFF(char* filename)
{
	ifstream fin(filename);
	ResMesh* mesh = new ResMesh();

	int numverts, numpoly, other;
	char buffer[10];

	fin >> buffer >> numverts >> numpoly >> other;

	mesh->VertData.ResizeTo( numverts * sizeof(VertType) );
	VertType* verts = (VertType*)mesh->VertData.Raw();
	mesh->NumVerts = numverts;

	for (int v=0; v<numverts; v++)
	{
		fin >> verts[v].mVertex.x;
		fin >> verts[v].mVertex.y;
		fin >> verts[v].mVertex.z;

		verts[v].mColor.Set( 0.5, 0.5, 0.5, 1.0);
	}

	int vpp;
	fin >> vpp;
	mesh->Format = MeshFormat::Create(vpp, true);
	mesh->NumIndices = vpp * numpoly;
	mesh->Indices.ResizeTo( mesh->NumIndices );

	for (int k=0; k<vpp; k++)
	{
		fin >> mesh->Indices[k];
	}

	for (int r=1; r<numpoly; r++)
	{
		fin >> vpp;
		for (int f=0; f<vpp; f++)
		{
			fin >> mesh->Indices[f + vpp*r];
		}
	}
	
	Core.Register(mesh);
	return mesh;
}
*/

#include "IOFF.h"

void IO_Mesh_ExportToIOFF(ResMesh* mesh, char* modelname, char* filename, bool color, bool normals)
{
	Assert( mesh->Format.IsIndexed() );

	ofstream fout(filename);
	if (!fout)
		return;

	fout << "IOFF( " << modelname << ", ";

	fout << mesh->NumVerts << ", " << mesh->NumPolys() << ", " << mesh->Format.VPP << ", ";
	if (color)
		fout << "C4F_";
	if (normals)
		fout << "N3F_";
	fout << "V3F ";

	int vpp = mesh->Format.VPP;
	VertType* verts = (VertType*)mesh->VertData.Raw();

	fout << ")\n{\n";
	for (int i=0; i<mesh->NumVerts; i++)
	{
		fout << "\t{ ";
		if (color)
		{
			Color4f* c = &(verts[i].mColor);
			fout << c->Red << ", " << c->Green << ", ";
			fout << c->Blue << ", " << c->Alpha << ", ";
		}
		if (normals)
		{
			fpnt* n = &(verts[i].mNormal);
			fout << n->x << ", " << n->y << ", " << n->z << ", ";
		}
		fpnt* p = &(verts[i].mVertex);
		fout << p->x << ", " << p->y << ", " << p->z;
		fout << "},\n";
	}

	fout << "},\n{\n";
	for (int p=0; p<mesh->NumIndices; p+=vpp)
	{
		fout << "\t";
		for (int j=0; j<vpp; j++)
		{
			fout << mesh->Indices[p+j] << ", ";
		}
		fout << "\n";
	}
	fout << "}\nIOFF_END(" << modelname << ")\n";

	fout.close();
}

void IO_Mesh_ExportToOFF(ResMesh* mesh, char* filename)
{
	Assert( mesh->Format.IsIndexed() );

	ofstream fout(filename);
	if (!fout)
		return;

	fout << "OFF\n";

	fout << mesh->NumVerts << " " << mesh->NumPolys() << " " << mesh->NumPolys()*mesh->Format.VPP << "\n";

	int vpp = mesh->Format.VPP;
	VertType* verts = (VertType*)mesh->VertData.Raw();

	for (int i=0; i<mesh->NumVerts; i++)
	{
		fpnt* p = &(verts[i].mVertex);
		fout << p->x << " " << p->y << " " << p->z << "\n";
	}

	for (int p=0; p<mesh->NumIndices; p+=vpp)
	{
		fout << vpp << " ";
		for (int j=0; j<vpp; j++)
		{
			fout << mesh->Indices[p+j] << " ";
		}
		fout << "\n";
	}

	fout.close();
}
