#include "Animation.h"


#if !SERVER
#include "Fbx.h"
#endif

UAnimation::UAnimation()
{

}

//UAnimation* UAnimation::Create(fbxsdk::FbxAnimStack* AnimStack, fbxsdk::FbxNode* InNode)
UAnimation* UAnimation::Create(fbxsdk::FbxAnimStack* AnimStack, fbxsdk::FbxNode* InNode)

{
	UAnimation* InstanceAnimation = new UAnimation();
	//TObjectPtr<UAnimation> InstanceAnimation = NewObject<UAnimation>(LevelToSpawnIn, Class, NewActorName, ActorFlags, Template);
	//LevelToSpawnIn->Actors.push_back(Actor);
	 

	//init
	if(AnimStack!=nullptr)
	{
		FbxTimeSpan AnimTimeSpan = AnimStack->GetLocalTimeSpan();
		//duration ticksecond
		InstanceAnimation->m_Duration = AnimTimeSpan.GetDuration().GetSecondDouble();
		InstanceAnimation->m_fTickPerSecond = FbxTime::GetFrameRate(AnimStack->GetScene()->GetGlobalSettings().GetTimeMode());
		
		//bone 갯수 관련
		fbxsdk::FbxAnimLayer* AnimLayer = AnimStack->GetMember<FbxAnimLayer>(0);
		if (!AnimLayer) return nullptr;

		fbxsdk::FbxScene* Scene = AnimStack->GetScene();
		fbxsdk::FbxNode* RootNode = Scene->GetRootNode();

		int32 NumBones = 0;		
		for (int i = 0; i < InNode->GetChildCount(); ++i)
		{
			fbxsdk::FbxNode* Node = InNode->GetChild(i);
			bool bHasAnimation = false;

			if (Node->LclTranslation.GetCurve(AnimLayer) ||
				Node->LclRotation.GetCurve(AnimLayer) ||
				Node->LclScaling.GetCurve(AnimLayer))
			{
				bHasAnimation = true;
				NumBones++;
			}

			//TODO:: UAnnimChannel 만들기 + 제어해야하는 뼈 정보를 생성하고있는 이 부분을 끝내기
			//aiNodeAnim : mChannel은 키프레임 정보들을 가지도록 만들고 사용 할때는 필요한 부분만 갱신하도록 만들기
			//animation보유중이면
			if (bHasAnimation)
			{
				UAnimChannel* pChannel = UAnimChannel::Create(AnimLayer, Node);
				if (pChannel != nullptr)
				{
					// 해당 채널을 보관
					InstanceAnimation->m_Channels.push_back(pChannel);
				}
			}
		}
		//bone 갯수 저장
		InstanceAnimation->m_iNumChannels = NumBones;


	}

	return InstanceAnimation;
}

void UAnimation::Init(UFbxFactory* pFbxFactory)
{
	for (uint32 i = 0; i < m_iNumChannels; ++i)
	{
		m_ChannelKeyFrames.push_back(0);

		UHierarchy* pNode = pFbxFactory->Get_HierarchyNode(m_Channels[i]->GetName());

		if (pNode != nullptr)
			m_HierarchyNodes.push_back(pNode);
	}

}

void UAnimation::PlayAnimation(FLOAT fTimeDelta)
{
	m_fPlayTime += m_fTickPerSecond * fTimeDelta;

	if (m_fPlayTime >= m_Duration)
	{
		m_fPlayTime = 0.f;

		for (auto& pChannel : m_Channels)
		{
			for (auto& iCurrentKeyFrame : m_ChannelKeyFrames)
				iCurrentKeyFrame = 0;
		}
	}

	uint32		iChannelIndex = 0;

	for (auto& pChannel : m_Channels)
	{
		m_ChannelKeyFrames[iChannelIndex] = pChannel->Update_Transformation(m_fPlayTime, m_ChannelKeyFrames[iChannelIndex], m_HierarchyNodes[iChannelIndex]);

		++iChannelIndex;
	}

}
