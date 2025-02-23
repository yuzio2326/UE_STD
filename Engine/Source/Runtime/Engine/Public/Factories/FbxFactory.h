#pragma once
#include "Factory.h"
#include "RenderCore.h"
#include "HierarchyNode.h"
#include "FbxFactory.generated.h"

namespace fbxsdk
{
	class FbxScene;
	class FbxManager;
	class FbxNode;
}
//class fbxsdk::FbxScene;
//class fbxsdk::FbxManager;

struct FMeshData
{
	FString Name;
	TArray<FPositionNormalUV> Vertices;
	TArray<uint32> Indices;

	uint32 NumPrimitives = 0;

	bool OwnedBone = false;
	TArray<FString> Bones;
};

UCLASS()
class UFbxFactory : public UFactory
{
	GENERATED_BODY()

public:
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual TObjectPtr<UObject> FactoryCreateFile(const FName InName, const FString& InFileName, const TCHAR* Params) override;

#if !SERVER
protected:
	fbxsdk::FbxScene* LoadFbxScene(fbxsdk::FbxManager* InFbxManager, const char* InFileName);

	void ExtractFbx(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData);
	void ExtractFbxAnim(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData);
	void Ready_HierarchyNodes(fbxsdk::FbxNode* InNode, UHierarchy* pParent, uint32 iDepth);


private:
	vector<class UHierarchy*>			UHierarchyNodes;
#endif
};