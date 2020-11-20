#include "assimp/vector3.h"
#include <utility>
#ifndef ASSIMP_BUILD_NO_PRM_IMPORTER

#include "PRMLoader.h"
#include <assimp/fast_atof.h>
#include <assimp/StreamReader.h>
#include <assimp/scene.h>
#include <assimp/IOSystem.hpp>
#include <assimp/ParsingUtils.h>
#include <assimp/importerdesc.h>
#include <filesystem>


using namespace Assimp;

static const aiImporterDesc prm_desc = {
	"PRM Importer",
	"",
	"",
	"",
	aiImporterFlags_SupportBinaryFlavour,
	0,
	0,
	0,
	0,
	"prm"
};

PRMImporter::PRMImporter() {}
PRMImporter::~PRMImporter() {}

bool PRMImporter::CanRead(const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const {
	return SimpleExtensionCheck(pFile, "prm");
}

const aiImporterDesc* PRMImporter::GetInfo() const {
	return &prm_desc;
}

aiVector3D getVector3D(StreamReaderLE& stream) {
	aiVector3D vector;
	vector.x = stream.GetF4();
	vector.y = stream.GetF4();
	vector.z = stream.GetF4();
	return vector;
}

static void readMesh(StreamReaderLE& stream, aiMesh *mesh, const std::string& meshPath) {
	mesh->mNumFaces = static_cast<unsigned int>(stream.GetU2());
	unsigned int initialNumVertices = static_cast<unsigned int>(stream.GetU2());
    auto initialVertices = new aiVector3D[initialNumVertices];
    auto initialNormals = new aiVector3D[initialNumVertices];

    mesh->mNumVertices = mesh->mNumFaces * 4;

	mesh->mFaces = new aiFace[mesh->mNumFaces];
	mesh->mVertices = new aiVector3D[mesh->mNumVertices];
	mesh->mNormals = new aiVector3D[mesh->mNumVertices];
	mesh->mColors[0] = new aiColor4D[mesh->mNumVertices];
	mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
    mesh->mNumUVComponents[0] = 2;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face;
		std::uint16_t flags = stream.GetU2();
		std::int16_t textureIndex = stream.GetI2();
        if (textureIndex != -1) {
            std::string texturePath = meshPath;
            texturePath.append(1, 'a' + textureIndex);
            texturePath += ".bmp";
        }
		face.mNumIndices = 3 + (flags & 1);
		face.mIndices = new unsigned int[face.mNumIndices];
		for (unsigned int j = 0; j < 4; j++) {
            std::uint16_t index = stream.GetU2();
            if (j < face.mNumIndices)
                face.mIndices[j] = index;
		}
		for (unsigned int j = 0; j < 4; j++) {
			aiColor4D color;
			color.r = static_cast<float>(stream.GetU1()) / 255;
			color.g = static_cast<float>(stream.GetU1()) / 255;
			color.b = static_cast<float>(stream.GetU1()) / 255;
			color.a = static_cast<float>(stream.GetU1()) / 255;
			if (j < face.mNumIndices)
				mesh->mColors[0][i * 4 + j] = color;
		}
		for (unsigned int j = 0; j < 4; j++) {
			aiVector3D textureCoord;
			textureCoord.x = stream.GetF4();
			textureCoord.y = stream.GetF4();
			if (j < face.mNumIndices)
				mesh->mTextureCoords[0][i * 4 + j] = textureCoord;
		}

		mesh->mFaces[i] = face;
	}

	for (unsigned int i = 0; i < initialNumVertices; i++) {
		initialVertices[i] = getVector3D(stream);
		initialNormals[i] = getVector3D(stream);
	}

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            mesh->mVertices[i * 4 + j] = initialVertices[face.mIndices[j]];
            mesh->mNormals[i * 4 + j] = initialNormals[face.mIndices[j]];
            face.mIndices[j] = i * 4 + j;
        }
    }
}

void PRMImporter::InternReadFile(const std::string& pFile, aiScene *pScene, IOSystem *pIOHandler) {
	StreamReaderLE stream(pIOHandler->Open(pFile, "rb"));

	aiNode *root = pScene->mRootNode = new aiNode();
	root->mName.Set(pFile);

	pScene->mMeshes = new aiMesh *[pScene->mNumMeshes = 1];
	aiMesh *mesh = pScene->mMeshes[0] = new aiMesh();

	root->mMeshes = new unsigned int[root->mNumMeshes = 1];
	root->mMeshes[0] = 0;

	readMesh(stream, mesh, pFile);
}

static const aiImporterDesc rvw_desc = {
	"World Importer",
	"",
	"",
	"",
	aiImporterFlags_SupportBinaryFlavour,
	0,
	0,
	0,
	0,
	"w"
};

RVWImporter::RVWImporter() {}
RVWImporter::~RVWImporter() {}

bool RVWImporter::CanRead(const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const {
	return SimpleExtensionCheck(pFile, "w");
}

const aiImporterDesc* RVWImporter::GetInfo() const {
	return &rvw_desc;
}

void RVWImporter::InternReadFile(const std::string& pFile, aiScene *pScene, IOSystem *pIOHandler) {
	StreamReaderLE stream(pIOHandler->Open(pFile, "rb"));

	aiNode *root = pScene->mRootNode = new aiNode();
	root->mName.Set(basename(pFile.data()));

	pScene->mNumMeshes = static_cast<unsigned int>(stream.GetU4());
	pScene->mMeshes = new aiMesh *[pScene->mNumMeshes];
	root->mNumMeshes = pScene->mNumMeshes;
	root->mMeshes = new unsigned int[root->mNumMeshes];
	for (unsigned int i = 0; i < root->mNumMeshes; i++) root->mMeshes[i] = i;

	for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
		pScene->mMeshes[i] = new aiMesh();

		stream.IncPtr(12);
		stream.IncPtr(4);
		stream.IncPtr(24);
		readMesh(stream, pScene->mMeshes[i], pFile);
	}
}

#endif
