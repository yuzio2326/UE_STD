#pragma once

#include "CoreMinimal.h"

UCLASS()
class ENGINE_API UAnimation : public UObject
{
private:
	UAnimation();
	UAnimation(const UAnimation& rhs);
	virtual ~UAnimation() = default;

public:
	HRESULT Initialize_Prototype(FBXAnimation* pAIAnimation);	//aiAnimation은 LSDK관련 
	HRESULT Initialize(class CModel* pModel);
	HRESULT Play_Animation(float fTimeDelta);

private:
	/* 이 애니메이션을 구동하기위해 사용되는 뼈의 갯수. */
	unsigned int						m_iNumChannels = 0;
	vector<class CChannel*>				m_Channels;

	/* 애니메이션 재생하는데 걸리는 전체시간. */
	float								m_fDuration = 0.f;

	/* 애니메이션의 초당 재생 속도. */
	float								m_fTickPerSecond = 0.f;

	float								m_fPlayTime = 0.f;

private: /* 복제된 애니메이션 마다 따로 가진다. */
	vector<class CHierarchyNode*>		m_HierarchyNodes;
	vector<unsigned int>				m_ChannelKeyFrames;

public:
	static UAnimation* Create(FBXAnimation* pAIAnimation);
	UAnimation* Clone(class CModel* pModel);
	//virtual void Free() override;

};