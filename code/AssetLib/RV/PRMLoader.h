#ifndef AI_PRMLOADER_H_INCLUDED
#define AI_PRMLOADER_H_INCLUDED

#include <assimp/BaseImporter.h>
#include <assimp/types.h>

namespace Assimp {

class PRMImporter : public BaseImporter {
public:
	PRMImporter();
	~PRMImporter();


public:
	bool CanRead(const std::string& pFile, IOSystem* pIOHandler, bool checkSig) const;

protected:
	const aiImporterDesc* GetInfo() const;

	void InternReadFile(const std::string& pFile, aiScene* pScene, IOSystem* pIOHandler);
};

}

#endif
