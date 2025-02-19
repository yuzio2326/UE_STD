#pragma once
#include "CoreMinimal.h"


UCLASS()
class UChannel : public UObject
{
	GENERATED_BODY()
public:
	UChannel();

public:
	const char* Get_Name() const {
		return m_szName;
	}

public:
	//HRESULT Initialize(FNodeAnim* pAIChannel);
	//unsigned int Update_Transformation(float fPlayTime, unsigned int iCurrentKeyFrame, class UHierarchy* pNode);


private:
	char							m_szName[MAX_PATH] = "";

	unsigned int					m_iNumKeyFrames = 0;
	vector<KEYFRAME>				m_KeyFrames;


public:
	static UChannel* Create(FNodeAnim* pAIChannel);
	//virtual void Free() override;

};