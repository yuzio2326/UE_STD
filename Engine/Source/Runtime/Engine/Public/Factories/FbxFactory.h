#pragma once
#include "Factory.h"
#include "RenderCore.h"
#include "FbxFactory.generated.h"

namespace fbxsdk
{
	class FbxScene;
	class FbxManager;
	class FbxNode;
	class FbxMesh;
}
//class fbxsdk::FbxScene;
//class fbxsdk::FbxManager;

struct FMeshData
{
	FString Name;
	TArray<FPositionNormalUV> Vertices;
	TArray<uint32> Indices;

	uint32 NumPrimitives = 0;

	//bool OwnedBone = false;
	TArray<const char *> BonesName;
	//뼈
	vector<class UHierarchy*>	Bones;
	uint32 BoneNumber = 0;
};

//class UHierarchy;

UCLASS()
class UFbxFactory : public UFactory
{
	GENERATED_BODY()

public:
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual TObjectPtr<UObject> FactoryCreateFile(const FName InName, const FString& InFileName, const TCHAR* Params) override;

	//PlayAnim할때 HierarchyNode 가지고와서 사용
	class UHierarchy* Get_HierarchyNode(const char* pNodeName);



#if !SERVER
protected:
	fbxsdk::FbxScene* LoadFbxScene(fbxsdk::FbxManager* InFbxManager, const char* InFileName);

	void ExtractFbx(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData);
	void ExtractFbxAnim(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData);
	void Ready_HierarchyNodes(fbxsdk::FbxNode* InNode, UHierarchy* pParent, uint32 iDepth);
	void SetUp_HierarchyNodes(fbxsdk::FbxNode* InNode, FMeshData MeshData);
	void Ready_Animations(fbxsdk::FbxNode* InNode, FMeshData MeshData);
	

private:
	vector<class UHierarchy*>			UHierarchyNodes;
	FString								NodeName;
	TArray<FPositionNormalUV>			m_Vertice;
	TArray<uint32>						m_Indices;
	uint32								m_NumPrimitives = 0;
	fbxsdk::FbxMesh* m_Mesh;

	bool								m_SetBones = false;

#endif
};