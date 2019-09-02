//GrannyMeshDumper by Helia01
//2019
#include "stdafx.h"

static void GrannyError(granny_log_message_type Type,
	granny_log_message_origin Origin,
	char const* File,
	granny_int32x Line,
	char const *Error,
	void *UserData)
{
	std::printf("GRANNY: %s\n", Error);
}

granny_log_callback logger = { &GrannyError };


int main(int argc, char** argv)
{
	SetConsoleTitleA(APP_NAME);
	std::printf("%s by Helia01\n", APP_NAME);

	if (argc < 2)
	{
		return 1;
	}
	
#ifdef _DEBUG
	GrannySetLogCallback(&logger);
#endif

	const char* file_name = argv[1];

	granny_file* gr2_file = GrannyReadEntireFile(file_name);

	if (!gr2_file)
	{
		std::printf("Error open file: \"%s\"!\n", file_name);
		return 1;
	}

	granny_file_info* Info = GrannyGetFileInfo(gr2_file);

	if (!Info)
	{
		std::printf("Error GrannyGetFileInfo!\n");
		return 1;
	}

	if (Info->ModelCount == 0)
	{
		std::printf("Could not find any models in: %s!\n", file_name);
		return 1;
	}

	if (Info->MeshCount == 0)
	{
		std::printf("Could not find any mesh in: %s!\n", file_name);
		return 1;
	}

	granny_skeleton* Skeleton = Info->Models[0]->Skeleton;

	//File
	granny_uint32 FileFormatRevision = gr2_file->Header->Version;
	granny_uint32 FileTag = gr2_file->Header->TypeTag;
	granny_uint32 FileCRC = gr2_file->Header->CRC;
	granny_uint32 FileSections = gr2_file->Header->SectionArrayCount;
	const char* FileSource = Info->FromFileName;

	//Art
	const char* art_tool_name = Info->ArtToolInfo->FromArtToolName;
	granny_int32 ArtToolMajorRevision = Info->ArtToolInfo->ArtToolMajorRevision;
	granny_int32 ArtToolMinorRevision = Info->ArtToolInfo->ArtToolMinorRevision;
	granny_real32 ArtToolUnitsPerMeter = Info->ArtToolInfo->UnitsPerMeter;

	//Exporter
	char* ExporterName = Info->ExporterInfo->ExporterName;
	granny_int32 ExporterMajorRevision = Info->ExporterInfo->ExporterMajorRevision;
	granny_int32 ExporterMinorRevision = Info->ExporterInfo->ExporterMinorRevision;
	granny_int32 ExporterCustomization = Info->ExporterInfo->ExporterCustomization;
	granny_int32 ExporterBuildNumber = Info->ExporterInfo->ExporterBuildNumber;

	//Textures etc...
	granny_int32 TextureCount = Info->TextureCount;
	granny_int32 MaterialCount = Info->MaterialCount;
	granny_int32 SkeletonCount = Info->SkeletonCount;
	granny_int32 MeshCount = Info->MeshCount;
	granny_int32 ModelCount = Info->ModelCount;
	granny_int32 AnimationCount = Info->AnimationCount;
	granny_int32 BoneCount = 0;

	if (Skeleton != NULL)
	{
		BoneCount = Skeleton->BoneCount;
	}
	


	std::printf("File: \"%s\"\n", file_name);
	std::printf("File format revision: %d\nSections: %d\nTag: 0x%x\nCRC: 0x%x\n", FileFormatRevision, FileSections, FileTag, FileCRC);
	std::printf("Source file: \"%s\"\n", FileSource);
	std::printf("Source tool: %s %d.%d\n", art_tool_name, ArtToolMajorRevision, ArtToolMinorRevision);
	std::printf("Exporter: %s\n", ExporterName);
	std::printf("Textures: %d\tMaterials: %d\tSkeletons: %d\tMeshes: %d\tModels: %d\tAnimations: %d\n", TextureCount, MaterialCount, SkeletonCount, MeshCount, ModelCount, AnimationCount);
	if (Skeleton != NULL)
	{
		std::printf("BoneCount: %d\n", BoneCount);
	}
	
	std::printf("\n\n");
	for (granny_int32x i = 0; i < Info->MeshCount; i++)
	{
		granny_mesh* Mesh = Info->Meshes[i];
		if (!Mesh)
			continue;
		std::vector<float> Positions;
		std::vector<float> Normals;
		std::vector<float> TexCoords;

		granny_variant Ignore;
		if (GrannyFindMatchingMember(GrannyGetMeshVertexType(Mesh),
			GrannyGetMeshVertices(Mesh),
			GrannyVertexPositionName,
			&Ignore))
		{
			granny_data_type_definition PosType[] = {
				{ GrannyReal32Member, GrannyVertexPositionName, 0, 3 },
				{ GrannyEndMember }
			};
			Positions.resize(GrannyGetMeshVertexCount(Mesh) * 3);
			GrannyCopyMeshVertices(Mesh, PosType, &Positions[0]);
		}
		if (GrannyFindMatchingMember(GrannyGetMeshVertexType(Mesh),
			GrannyGetMeshVertices(Mesh),
			GrannyVertexNormalName,
			&Ignore))
		{
			granny_data_type_definition NormalType[] = {
				{ GrannyReal32Member, GrannyVertexNormalName, 0, 3 },
				{ GrannyEndMember }
			};
			Normals.resize(GrannyGetMeshVertexCount(Mesh) * 3);
			GrannyCopyMeshVertices(Mesh, NormalType, &Normals[0]);
		}
		if (GrannyFindMatchingMember(GrannyGetMeshVertexType(Mesh),
			GrannyGetMeshVertices(Mesh),
			GrannyVertexTextureCoordinatesName "0",
			&Ignore))
		{
			granny_data_type_definition TexCoordType[] = {
				{ GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2 },
				{ GrannyEndMember }
			};
			TexCoords.resize(GrannyGetMeshVertexCount(Mesh) * 2);
			GrannyCopyMeshVertices(Mesh, TexCoordType, &TexCoords[0]);
		}

		char buffer[512];
		sprintf_s(buffer, "%s_mesh_%d.obj", file_name, i);
		FILE* file;

		fopen_s(&file, buffer, "w");

		for (size_t p = 0; p < Positions.size(); p += 3)
			fprintf(file, "v %f %f %f\n", Positions[p + 0], Positions[p + 1], Positions[p + 2]);
		fprintf(file, "# %d vertices\n\n", (int)Positions.size() / 3);

		for (size_t n = 0; n < Normals.size(); n += 3)
			fprintf(file, "vn %f %f %f\n", Normals[n + 0], Normals[n + 1], Normals[n + 2]);
		fprintf(file, "# %d vertex normals\n\n", (int)Normals.size() / 3);

		for (size_t t = 0; t < TexCoords.size(); t += 2)
			fprintf(file, "vt %f %f\n", TexCoords[t + 0], 1.0 - TexCoords[t + 1]);
		fprintf(file, "# %d vertex texture coords\n\n", (int)TexCoords.size() / 2);

		fprintf(file, "g %s\n", Mesh->Name);
		std::vector<granny_int32> MeshIndices(GrannyGetMeshIndexCount(Mesh));
		GrannyCopyMeshIndices(Mesh, 4, &MeshIndices[0]);

		for (size_t mi = 0; mi < MeshIndices.size(); mi += 3)
		{
			// Silly test to eliminate strictly degenerate tris (they confuse maya)
			{
				int v0 = MeshIndices[mi + 0];
				int v1 = MeshIndices[mi + 1];
				int v2 = MeshIndices[mi + 2];
				if (v0 == v1 || v0 == v2 || v1 == v2)
					continue;
			}

			fprintf(file, "f ");
			for (int tv = 0; tv < 3; ++tv)
			{
				int Index = MeshIndices[mi + tv] + 1;
				fprintf(file, "%d", Index);

				if (!TexCoords.empty())
					fprintf(file, "/%d", Index);
				else
					fprintf(file, "/");

				if (!Normals.empty())
					fprintf(file, "/%d ", Index);
				else
					fprintf(file, "/ ");
			}
			fprintf(file, "\n");
		}
		fclose(file);
	}
	
	
	system("pause");
	return 0;
}