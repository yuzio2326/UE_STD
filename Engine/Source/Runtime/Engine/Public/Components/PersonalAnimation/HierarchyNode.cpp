#include "HierarchyNode.h"
#include <DirectXMath.h>

UHierarchy::UHierarchy()
{
}

HRESULT UHierarchy::Initialize(FName pAINodeName, FTransform mTransformation, class UHierarchy* pParent, unsigned int iDepth)
{

	/* 뼈 이름 보관. */
	FString NodeNameString = pAINodeName.ToString();		//Name을 String으로 바꾸기
	//	NodeNameString으로 나눈 각 문자를 static_cast로 casting해서 m_szName에 보관하기
	for (int i = 0; i < NodeNameString.length() && i < MAX_PATH - 1; ++i)
		m_szName[i] = static_cast<char>(NodeNameString[i]);

	//나중에 m_szName로 일치하는 문자를 찾아서 행렬정보를 받아가지고 계속 갱신을 할 예정


	/* 씬객체로 부터 행렬정보를 받아올때는 반드시 전치해서 받아와라. */
	XMStoreFloat4x4(&m_OffsetMatrix, XMMatrixIdentity());	//DirectXMath로딩 하기
	//XMMatrixIdentity가 인식이 될때가 있고 안될때가 있음 해당 문제의 원인은 나중에 찾기 ㄱㄱ

	memcpy(&m_Transformation, &mTransformation, sizeof(FMatrix));
	XMStoreFloat4x4(&m_Transformation, XMMatrixTranspose(XMLoadFloat4x4(&m_Transformation)));

	m_iDepth = iDepth;
	m_pParent = pParent;

	XMStoreFloat4x4(&m_CombinedTransformation, XMMatrixIdentity());

	return S_OK;
}

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

UHierarchy* UHierarchy::Create(FBXNode* pAINode, UHierarchy* pParent, unsigned int iDepth)
{
	UHierarchy* pInstance = new UHierarchy();

	//if (FAILED(pInstance->Initialize(pAINode, pParent, iDepth)))
	//{
	//	_ASSERT(TEXT("Failed To Created : CHierarchyNode"));
	//}

	return pInstance;
}

//Create지우고 대신 여기에 세팅중? 아니면 create에 만들까? 일단 적당히 
void UHierarchy::SetupHierarchy(FBXNode* pAINode, UHierarchy* pParent, unsigned int iDepth)
{

	if (HierarchyNode == NULL)
	{
		HierarchyNode = NewObject<UHierarchy>(this, UHierarchy::StaticClass(), TEXT("Create_HierarchyNode"));
		HierarchyNode->Initialize(pAINode->NodeName, pAINode->LocalTransform, pParent, iDepth);
	}

}
