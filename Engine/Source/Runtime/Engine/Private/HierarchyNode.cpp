#include "HierarchyNode.h"
#include <DirectXMath.h>


#if !SERVER
#include "Fbx.h"
#endif

UHierarchy::UHierarchy()
{
}



//애니메이션 재생시 사용할 예정입니다
void UHierarchy::Set_CombinedTransformation()
{
	//부모가 있으면 자신의 행렬과 부모의 행렬을 곱해서 부모를 따라서 추가적으로 움직임을 부여
	if (nullptr != m_pParent)
		XMStoreFloat4x4(&m_CombinedTransformation, (XMLoadFloat4x4(&m_Transformation) *XMLoadFloat4x4(&m_pParent->m_CombinedTransformation)));
	//부모가 없으면 자신이 root와 같은 존재가 된다 
	else
		m_CombinedTransformation = m_Transformation;
}

void UHierarchy::Set_OffsetMatrix(FMatrix OffsetMatrix)
{
	XMStoreFloat4x4(&m_OffsetMatrix, OffsetMatrix);
}

UHierarchy* UHierarchy::Create(fbxsdk::FbxNode* InNode, UHierarchy* pParent, uint32 iDepth)
{

	//node없으면 되돌리기
	if (!InNode)
	{
		return nullptr;
	}

	UHierarchy* NewHierarchy = new UHierarchy();

	//init 과정
	{
		//뼈이름 보관	//muzzleflash 부분의 자식이 없어서 멈추는 현상이 있음
		NewHierarchy->m_FStringName= InNode->GetName();

		//씬객체로부터 전치해서 받아올 예정
		fbxsdk::FbxAMatrix IdentityMatrix;
		IdentityMatrix.SetIdentity();
		memcpy(&NewHierarchy->m_OffsetMatrix, &IdentityMatrix, sizeof(FbxAMatrix));
		//XMStoreFloat4x4(&m_OffsetMatrix, XMMatrixIdentity());	같은 내용

		fbxsdk::FbxAMatrix FbxTransform = InNode->EvaluateGlobalTransform();
		memcpy(&NewHierarchy->m_Transformation, &FbxTransform, sizeof(FbxTransform));
		
		XMStoreFloat4x4(&NewHierarchy->m_Transformation, XMMatrixTranspose(XMLoadFloat4x4(&NewHierarchy->m_Transformation)));

		NewHierarchy->m_pParent = pParent;
		NewHierarchy->m_iDepth = iDepth;

		memcpy(&NewHierarchy->m_CombinedTransformation, &IdentityMatrix, sizeof(FbxAMatrix));

		//_ASSERT(TEXT("Failed To Created : CHierarchyNode"));
	}

	return NewHierarchy;
}

//Create지우고 대신 여기에 세팅중? 아니면 create에 만들까? 일단 적당히  
// FbxFactory 에 만듦
//void UHierarchy::SetupHierarchy(fbxsdk::FbxNode* InNode, UHierarchy* pParent, uint32 iDepth)
//{
//	//if (!InNode)
//	//{
//	//	return nullptr;
//	//}
//
//}
