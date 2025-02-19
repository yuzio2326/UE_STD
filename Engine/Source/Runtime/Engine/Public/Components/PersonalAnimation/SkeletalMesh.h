#pragma once
#include "CoreMinimal.h"
#include "VertexFactory.h"
#include "Materials/Material.h"
#include "MeshContainer.generated.h"


struct FMeshData;
class UMaterial;

//MeshContainer 역할을 하던 곳입니다.

struct FSkeletalMeshRenderData
{
public:
	void Create(class USkeletalMesh* Outer, const FMeshData& NewMeshData);

	FString Name;
	FVertexFactory VertexFactory;

	TObjectPtr<UMaterial> Material;
	uint32 NumVertices = 0;		//정점 갯수
	uint32 NumPrimitives = 0;

	//FindAttributeChecked 에서 HierarchyNode를 세팅할 예정입니다. 
	//기존 //HRESULT HierarchyNode(); 부분의 방식을 이쪽으로 세팅을 바꾸기 ㄱㄱ
	//HRESULT CMeshContainer::SetUp_HierarchyNodes(CModel * pModel, aiMesh* pAIMesh)		
	// aiMesh는 Assimp Lib에 있으니 언리얼 임포팅 부분으로 바꾸기
	/*
	*	
		사용한 Assimp의 aiMesh 데이터 구조체

		Vertex Positions (정점 위치)
		Normals (노멀)
		Tangents & Bitangents (탄젠트 및 비탄젠트)
		UVs (텍스처 좌표)
		Indices (인덱스 버퍼, 삼각형 면을 구성)
		Material Index (사용할 머티리얼 정보)

		언리얼에는 FRawMesh 가 있음
		정 귀찮으면 StaticMesh에서 해당 요소를 가지고 와가지고 하는것도 나쁘지 않아보임
		ㄴ> staticMesh 에서 가지고 와가지고 세팅 ㄱㄱ

	*/


	//HRESULT URigHierarchy();		//HierarchyNode 가 될 녀석입니다. setup방법은 기존 unreal을 참고해서 만드는중
	//참고로 URigHierarchy는 class CONTROLRIG_API URigHierarchy : public UObject

};

UCLASS()
class ENGINE_API USkeletalMesh : public UObject
{
	GENERATED_BODY()
public:
	//USkeletalMesh();


	//virtual void Create(const TArray<FMeshData>& NewMeshData);

	//TArray<FSkeletalMeshRenderData>& GetRenderData() { return RenderData; }

	//TEnginePtr<UMaterial> GetMaterial(const int32 MaterialIndex)
	//{
	//	return RenderData[MaterialIndex].Material;
	//}

	//uint32 GetRenderDataCount() { return RenderData.size(); }

	//bool SetUpHierarchy();


protected:
	//TArray<FSkeletalMeshRenderData> RenderData;


};