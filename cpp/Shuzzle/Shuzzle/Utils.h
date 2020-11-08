
void InversePolygonRotation(ResMesh* mesh)
{
	Assert(mesh->Format.VPP==3);
	Assert(mesh->Format.IsIndexed());
	Index* inds = mesh->Indices.Raw();
	for (int i=0; i<mesh->NumIndices; i+=3)
	{
		Index t = inds[i];
		inds[i] = inds[i+2];
		inds[i+2] = t;
	}
}

void ApplyMatrixOnMesh(ResMesh* mesh, MatrixStruct* mat)
{
	VertType* vt = (VertType*)mesh->VertData.Raw();
	fpnt work;
	for (int i=0; i<mesh->NumVerts; i++)
	{
		Vec3TimesMatrix4(vt[i].mVertex, *mat, work);
		vt[i].mVertex = work;
	}
}

void PaintMesh(ResMesh* mesh, Color4f color)
{
	VertType* data = (VertType*)mesh->VertData.Raw();
	int n = mesh->NumVerts;
	for (int i=0; i<n; i++)
	{
		data[i].mColor = color;
	}
}

ResMesh* GenSphereMesh(int rings, int slices)
{
	ResMesh* mesh = ResMesh::Create( MeshFormat::Create(3, true),
		rings*slices +2, 3*(2*slices + (rings-1)*slices*2));

	VertType* data = (VertType*)mesh->VertData.Raw();
	IndPoly3* inds = (IndPoly3*)mesh->Indices.Raw();

	float scale, x, y, z, d;
	int i;
	for (int r=0; r<rings; r++)
	{
		z = ((float)(r+1)) / ((float)(rings+1));
		z = ((z*2.0f)-1.0f);
		scale = sqrt( 1.0f - (z*z) );
		for (int s=0; s<slices; s++)
		{
			i = (r*slices)+s;
			d = ((float)s)/((float)(slices));
			d *= 2*Pi;
			x = cos(d)*scale;
			y = sin(d)*scale;
			data[i].mVertex.Set(x, y, z);

			if (r!=rings-1)
			{
				i = (r*slices*2)+2*s;
				inds[i].a = (r*slices)+s;
				inds[i].b = (r*slices)+((s+1)%slices);
				inds[i].c = ((r+1)*slices)+s;

				i++;
				inds[i].a = (r*slices)+((s+1)%slices);
				inds[i].b = ((r+1)*slices)+((s+1)%slices);
				inds[i].c = ((r+1)*slices)+s;
			}
		}
	}
	
	data[mesh->NumVerts-2].mVertex.Set(0, 0, -1);
	data[mesh->NumVerts-1].mVertex.Set(0, 0, 1);
	IndPoly3* ito = &inds[ (rings-1)*slices*2 ];
	for (i=0; i<slices; i++)
	{
		ito[i].a = mesh->NumVerts-2;
		ito[i].c = i;
		ito[i].b = (i+1)%slices;
	}
	ito = &inds[ (rings-1)*slices*2 + slices ];
	for (i=0; i<slices; i++)
	{
		ito[i].a = mesh->NumVerts-1;
		ito[i].b = ((rings-1)*slices)+i;
		ito[i].c = ((rings-1)*slices)+(i+1)%slices;
	}

	for (i=0; i<mesh->NumVerts; i++)
	{
		data[i].mColor.Set(1.0, 1.0, 1.0, 0.25);
	}

	return mesh;
}

ResMesh* GenQuadCubeMesh()
{
	ResMesh* mesh = ResMesh::Create(MeshFormat::Create(4, true), 8, 4*6 );

	VertType* data = (VertType*)mesh->VertData.Raw();

	data[0].mVertex.Set(-1, -1, -1);
	data[1].mVertex.Set( 1, -1, -1);
	data[2].mVertex.Set( 1,  1, -1);
	data[3].mVertex.Set(-1,  1, -1);
	data[4].mVertex.Set(-1, -1,  1);
	data[5].mVertex.Set( 1, -1,  1);
	data[6].mVertex.Set( 1,  1,  1);
	data[7].mVertex.Set(-1,  1,  1);

	for (int i=0; i<8; i++)
	{
		data[i].mColor.Set(1.0, 1.0, 1.0, 1.0);
	}

	IndPoly4* tri = ((IndPoly4*)mesh->Indices.Raw());

	//back
	tri[0].Set(3, 2, 1, 0);

	//front
	tri[1].Set(4, 5, 6, 7);

	//side 1
	tri[2].Set(0, 4, 7, 3);

	//side 2
	tri[3].Set(1, 2, 6, 5);

	//top 
	tri[4].Set(3, 7, 6, 2);

	//bottom:
	tri[5].Set(0, 1, 5, 4);

	return mesh;
}

ResMesh* GenCubeMesh()
{
	ResMesh* mesh = ResMesh::Create(MeshFormat::Create(3, true), 8, 3*12 );

	VertType* data = (VertType*)mesh->VertData.Raw();

	data[0].mVertex.Set(-1, -1, -1);
	data[1].mVertex.Set( 1, -1, -1);
	data[2].mVertex.Set( 1,  1, -1);
	data[3].mVertex.Set(-1,  1, -1);
	data[4].mVertex.Set(-1, -1,  1);
	data[5].mVertex.Set( 1, -1,  1);
	data[6].mVertex.Set( 1,  1,  1);
	data[7].mVertex.Set(-1,  1,  1);

	for (int i=0; i<8; i++)
	{
		data[i].mColor.Set(1.0, 1.0, 1.0, 1.0);
	}

	IndPoly3* tri = ((IndPoly3*)mesh->Indices.Raw());

	//back
	tri[0].Set(2, 1, 0);
	tri[1].Set(0, 3, 2);

	//front
	tri[4].Set(4, 5, 6);
	tri[5].Set(6, 7, 4);

	//side 1
	tri[2].Set(0, 4, 7);
	tri[3].Set(7, 3, 0);

	//side 2
	tri[10].Set(6, 5, 1);
	tri[11].Set(1, 2, 6);

	//top 
	tri[6].Set(3, 7, 6);
	tri[7].Set(6, 2, 3);

	//bottom:
	tri[8].Set(5, 4, 0);
	tri[9].Set(0, 1, 5);

	return mesh;
}

/*
ResMesh* GenTriMesh()
{
	ResMesh* mesh = ResMesh::Create(
		MeshFormat::Create(3, false), 12, 0 );

	VertType* data = (VertType*)mesh->VertData.Raw();

	data[0].mVertex.Set(-1, -1, 1);
	data[1].mVertex.Set(1, -1, 1);
	data[2].mVertex.Set(0, 1, 0);
	
	data[3].mVertex.Set(-1, -1, 1);
	data[5].mVertex.Set(0, -1, -1);
	data[4].mVertex.Set(0, 1, 0);

	data[6].mVertex.Set(1, -1, 1);
	data[7].mVertex.Set(0, -1, -1);
	data[8].mVertex.Set(0, 1, 0);

	data[9].mVertex.Set(-1, -1, 1);
	data[11].mVertex.Set(1, -1, 1);
	data[10].mVertex.Set(0, -1, -1);

	for (int i=0; i<9; i++)
	{
		data[i].mColor.Set(0.15, 0.65, 0.15, 0.5);
	}

	mesh.UpdateNormals();

	return mesh;
}

ResMesh* GenQuadCubeMesh()
{
	ResMesh* mesh = ResMesh::Create(MeshFormat::Create(4, false), 16, 0);

	VertType* data = (VertType*)mesh->VertData.Raw();

	data[0].mVertex.Set(-1,  1, -1);
	data[1].mVertex.Set( 1,  1, -1);
	data[2].mVertex.Set( 1, -1, -1);
	data[3].mVertex.Set(-1, -1, -1);

	data[4].mVertex.Set(-1, -1,  1);
	data[5].mVertex.Set( 1, -1,  1);
	data[6].mVertex.Set( 1,  1,  1);
	data[7].mVertex.Set(-1,  1,  1);

	data[11].mVertex.Set(-1, -1, -1);
	data[10].mVertex.Set(-1,  1, -1);
	data[9].mVertex.Set(-1,  1,  1);
	data[8].mVertex.Set(-1, -1,  1);

	data[12].mVertex.Set( 1, -1, -1);
	data[13].mVertex.Set( 1,  1, -1);
	data[14].mVertex.Set( 1,  1,  1);
	data[15].mVertex.Set( 1, -1,  1);

	for (int i=0; i<16; i++)
	{
		data[i].mColor.Set(0.15, 0.65, 0.15, 0.5);
	}

	mesh.UpdateNormals();

	return mesh;
}
*/


/*
ResMesh* GenCubeOutline()
{
	ResMesh* mesh = ResMesh::Create(MeshFormat::Create(LC_VF_Vf, LC_MF_Line | LC_MF_Indexed), 8, 2*12 );

	VertData_Vf* data = (VertData_Vf*)mesh->VertData.Raw();
	data[0].mVertex.Set(-1, -1, -1);
	data[1].mVertex.Set( 1, -1, -1);
	data[2].mVertex.Set( 1,  1, -1);
	data[3].mVertex.Set(-1,  1, -1);
	data[4].mVertex.Set(-1, -1,  1);
	data[5].mVertex.Set( 1, -1,  1);
	data[6].mVertex.Set( 1,  1,  1);
	data[7].mVertex.Set(-1,  1,  1);

	IndPoly2* tri = ((IndPoly2*)mesh->Indices.Raw());

	tri[0].Set(0, 1);
	tri[1].Set(1, 2);
	tri[2].Set(2, 3);
	tri[3].Set(3, 0);

	tri[4].Set(4, 5);
	tri[5].Set(5, 6);
	tri[6].Set(6, 7);
	tri[7].Set(7, 4);

	tri[8].Set(0, 4);
	tri[9].Set(1, 5);
	tri[10].Set(2, 6);
	tri[11].Set(3, 7);

	return mesh;
}

ResMesh* GenQuadCubeMesh()
{
	ResMesh* mesh = ResMesh::Create(16, LC_VF_Q_C4fVf, 0);

	VertData_C4fVf* data = (VertData_C4fVf*)mesh->PackedVertData;

	data[0].mVertex.Set(-1,  1, -1);
	data[1].mVertex.Set( 1,  1, -1);
	data[2].mVertex.Set( 1, -1, -1);
	data[3].mVertex.Set(-1, -1, -1);

	data[4].mVertex.Set(-1, -1,  1);
	data[5].mVertex.Set( 1, -1,  1);
	data[6].mVertex.Set( 1,  1,  1);
	data[7].mVertex.Set(-1,  1,  1);

	data[11].mVertex.Set(-1, -1, -1);
	data[10].mVertex.Set(-1,  1, -1);
	data[9].mVertex.Set(-1,  1,  1);
	data[8].mVertex.Set(-1, -1,  1);

	data[12].mVertex.Set( 1, -1, -1);
	data[13].mVertex.Set( 1,  1, -1);
	data[14].mVertex.Set( 1,  1,  1);
	data[15].mVertex.Set( 1, -1,  1);

	for (int i=0; i<16; i++)
	{
		data[i].mColor.Set(1.0, 1.0, 1.0, 1.0);
	}

	return mesh;
}

ResMesh* GenTriMesh()
{
	ResMesh* mesh = ResMesh::Create(12, LC_VF_T_C4fVf, 0 );

	VertData_C4fVf* data = (VertData_C4fVf*)mesh->PackedVertData;

	data[0].mVertex.Set(-1, -1, 1);
	data[1].mVertex.Set(1, -1, 1);
	data[2].mVertex.Set(0, 1, 0);
	
	data[3].mVertex.Set(-1, -1, 1);
	data[5].mVertex.Set(0, -1, -1);
	data[4].mVertex.Set(0, 1, 0);

	data[6].mVertex.Set(1, -1, 1);
	data[7].mVertex.Set(0, -1, -1);
	data[8].mVertex.Set(0, 1, 0);

	data[9].mVertex.Set(-1, -1, 1);
	data[11].mVertex.Set(1, -1, 1);
	data[10].mVertex.Set(0, -1, -1);

	for (int i=0; i<9; i++)
	{
		data[i].mColor.Set(0.15, 0.65, 0.15, 0.5);
	}

	return mesh;
}

ResMesh* GenCubeMesh()
{
	ResMesh* mesh = ResMesh::Create(8, LC_VF_T_IC4fVf, 3*12 );

	VertData_C4fVf* data = (VertData_C4fVf*)mesh->PackedVertData;

	data[0].mVertex.Set(-1, -1, -1);
	data[1].mVertex.Set( 1, -1, -1);
	data[2].mVertex.Set( 1,  1, -1);
	data[3].mVertex.Set(-1,  1, -1);
	data[4].mVertex.Set(-1, -1,  1);
	data[5].mVertex.Set( 1, -1,  1);
	data[6].mVertex.Set( 1,  1,  1);
	data[7].mVertex.Set(-1,  1,  1);

	for (int i=0; i<8; i++)
	{
		data[i].mColor.Set(0.5, 0.5, 0.5, 0.5);
	}

	IndPoly3* tri = ((IndPoly3*)mesh->Indices);

	//back
	tri[0].Set(2, 1, 0);
	tri[1].Set(0, 3, 2);

	//front
	tri[4].Set(4, 5, 6);
	tri[5].Set(6, 7, 4);

	//side 1
	tri[2].Set(0, 4, 7);
	tri[3].Set(7, 3, 0);

	//side 2
	tri[10].Set(6, 5, 1);
	tri[11].Set(1, 2, 6);

	//top 
	tri[6].Set(3, 7, 6);
	tri[7].Set(6, 2, 3);

	//bottom:
	tri[8].Set(5, 4, 0);
	tri[9].Set(0, 1, 5);

	return mesh;
}

ResMesh* GenCubeOutline()
{
	ResMesh* mesh = ResMesh::Create(8, LC_VF_L_IVf, 2*12 );

	VertData_Vf* data = (VertData_Vf*)mesh->PackedVertData;
	data[0].mVertex.Set(-1, -1, -1);
	data[1].mVertex.Set( 1, -1, -1);
	data[2].mVertex.Set( 1,  1, -1);
	data[3].mVertex.Set(-1,  1, -1);
	data[4].mVertex.Set(-1, -1,  1);
	data[5].mVertex.Set( 1, -1,  1);
	data[6].mVertex.Set( 1,  1,  1);
	data[7].mVertex.Set(-1,  1,  1);

	IndPoly2* tri = ((IndPoly2*)mesh->Indices);

	tri[0].Set(0, 1);
	tri[1].Set(1, 2);
	tri[2].Set(2, 3);
	tri[3].Set(3, 0);

	tri[4].Set(4, 5);
	tri[5].Set(5, 6);
	tri[6].Set(6, 7);
	tri[7].Set(7, 4);

	tri[8].Set(0, 4);
	tri[9].Set(1, 5);
	tri[10].Set(2, 6);
	tri[11].Set(3, 7);

	return mesh;
}
*/
