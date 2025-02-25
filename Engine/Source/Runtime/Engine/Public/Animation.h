#pragma once

#include "CoreMinimal.h"
#include "Animation.generated.h"


class UFbxFactory;

UCLASS()
class ENGINE_API UAnimation : public UObject
{
	GENERATED_BODY()

public:
	UAnimation();

	/* 이 애니메이션을 구동하기위해 사용되는 뼈의 갯수. */
	uint32						m_iNumChannels = 0;
	vector<class UAnimChannel*>		m_Channels;

	/* 애니메이션 재생하는데 걸리는 전체시간. */
	double						m_Duration;

	/* 애니메이션의 초당 재생 속도. */
	FLOAT						m_fTickPerSecond = 0.f;

	FLOAT						m_fPlayTime = 0.f;

private:  /* 복제된 애니메이션 마다 따로 가진다. */
	vector<class CHierarchyNode*>	m_HierarchyNodes;
	vector<uint32>					m_ChannelKeyFrames;

public:
	static UAnimation* Create(fbxsdk::FbxAnimStack * AnimStack, fbxsdk::FbxNode* InNode);
	virtual void Init(class UFbxFactory* pFbxFactory);
	virtual void PlayAnimation(FLOAT fTimeDelta);

};