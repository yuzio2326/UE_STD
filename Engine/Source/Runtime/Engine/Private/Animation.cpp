#include "Animation.h"
#include "AnimChannel.h"

#if !SERVER
#include "Fbx.h"
#endif

UAnimation::UAnimation()
{

}

UAnimation* UAnimation::Create(FbxAnimStack* AnimStack)
{
	UAnimation* InstanceAnimation = new UAnimation();
	
	//init
	if(AnimStack!=nullptr)
	{
		FbxTimeSpan AnimTimeSpan = AnimStack->GetLocalTimeSpan();
		//duration ticksecond
		InstanceAnimation->m_Duration = AnimTimeSpan.GetDuration().GetSecondDouble();
		InstanceAnimation->m_fTickPerSecond = FbxTime::GetFrameRate(AnimStack->GetScene()->GetGlobalSettings().GetTimeMode());
		//bone 갯수 저장
		InstanceAnimation->m_iNumChannels = AnimStack->GetMemberCount<FbxAnimLayer>();

		// 현재 애니에미션에서 제어해야하는 뼈 정보들을 생성해서 보관하는 과정
		for (uint32 i = 0; i < InstanceAnimation->m_iNumChannels; ++i)
		{
			//TODO:: UAnnimChannel 만들기 + 제어해야하는 뼈 정보를 생성하고있는 이 부분을 끝내기
			//aiNodeAnim : mChannel은 키프레임 정보들을 가지도록 만들고 사용 할때는 필요한 부분만 갱신하도록 만들기

		}


	}

	return nullptr;
}
