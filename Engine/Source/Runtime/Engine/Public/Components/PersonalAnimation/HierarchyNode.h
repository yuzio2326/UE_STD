#pragma once
#include "CoreMinimal.h"
#include "D3D11Resources.h"
#include "Personal_Include.h"

//struct aiNode
//{
//	//ASSIMP 구성중 하나 언리얼로 포팅 하려면 해당 요소의 언리얼 함수를 찾아서 만들어야함
//
//	FName mName;
//	FMatrix mTransformation;
//
//	//FindNode 관련도 만들어야 함... 쉣!!
//
//};

//struct FBXNode	//Personal_Include.h 여기로 뺄지는 나중에 판단 ㄱㄱ or SimpleMath에 넣을지 고민중
//{
//	FString NodeName;
//	FTransform LocalTransform;
//	FBXNode* Parent = nullptr;
//	TArray<FBXNode*> Children;
//};


UCLASS()
class UHierarchy : public UObject
{
	GENERATED_BODY()
	//월드 상에 존재해야 하니 일단 UObject 안에 박아놓고

private:
	UHierarchy();
	virtual ~UHierarchy() = default;

public:
	const char* Get_Name() const {
		return m_szName;
	}
	unsigned int Get_Depth() const {
		return m_iDepth;
	}

	FMatrix Get_OffSetMatrix() {
		return XMLoadFloat4x4(&m_OffsetMatrix);
	}

	FMatrix Get_CombinedTransformation() {
		return XMLoadFloat4x4(&m_CombinedTransformation);
	}


public:
	void Set_Transformation(FMatrix Transformation) {
		XMStoreFloat4x4(&m_Transformation, Transformation);
	}

public:
	HRESULT Initialize(FName pAINodeName, FTransform mTransformation, class UHierarchy* pParent, unsigned int iDepth);
	void Set_CombinedTransformation();
	void Set_OffsetMatrix(FMatrix OffsetMatrix);

private:
	char			m_szName[MAX_PATH] = "";
	FMatrix			m_OffsetMatrix;
	FMatrix			m_Transformation;
	FMatrix			m_CombinedTransformation;
	UHierarchy*		m_pParent = nullptr;
	unsigned int	m_iDepth = 0;

	TObjectPtr<class UHierarchy> HierarchyNode;
public:
	static UHierarchy* Create(FBXNode* pAINode, class UHierarchy* pParent, unsigned int iDepth);

	//create 및 init 관련 하나로 할거임
	virtual void SetupHierarchy(FBXNode* pAINode, class UHierarchy* pParent, unsigned int iDepth);

	virtual void Free();

};