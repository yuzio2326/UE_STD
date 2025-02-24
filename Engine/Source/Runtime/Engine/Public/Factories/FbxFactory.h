#pragma once
#include "Factory.h"
#include "RenderCore.h"
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

	//0개면 하나도 없다는 소리임 0번째 관련이 아니라
	uint32 OwnAnimations = 0;



};

//class UHierarchy;

UCLASS()
class UFbxFactory : public UFactory
{
	GENERATED_BODY()

public:
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual TObjectPtr<UObject> FactoryCreateFile(const FName InName, const FString& InFileName, const TCHAR* Params) override;

	class UHierarchy* Get_HierarchyNode(FString* pNodeName);



#if !SERVER
protected:
	fbxsdk::FbxScene* LoadFbxScene(fbxsdk::FbxManager* InFbxManager, const char* InFileName);

	void ExtractFbx(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData);
	void ExtractFbxAnim(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData);//Scene 도 넣어서 하는게 ㄱㅊ을거 같아보임 나중에 ㄱㄱ
	void Ready_HierarchyNodes(fbxsdk::FbxNode* InNode, UHierarchy* pParent, uint32 iDepth);

	void Ready_Animations(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData);

private:
	vector<class UHierarchy*>			UHierarchyNodes;


#endif
};