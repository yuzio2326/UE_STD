#include "AnimChannel.h"
#include "HierarchyNode.h"

#if !SERVER
#include "Fbx.h"
#endif


UAnimChannel::UAnimChannel()
{
}

uint32 UAnimChannel::Update_Transformation(FLOAT fPlayTime, uint32 iCurrentKeyFrame, UHierarchy* pHierarchyNode)
{
	Vector4 vRotation;
	Vector3	vScale, vPosition;

	//마지막 키프레임 넘어가면 유지하도록 하기
	if (fPlayTime >= m_KeyFrames.back().fTime)
	{
		vScale = m_KeyFrames.back().vScale;
		vRotation = m_KeyFrames.back().vRotation;
		vPosition = m_KeyFrames.back().vPosition;
		//유지 말고 반복 시키고 싶으면 초기 상태로 되돌리기

	}
	else 
	{
		while (fPlayTime >= m_KeyFrames[iCurrentKeyFrame + 1].fTime)
			++iCurrentKeyFrame;
		//현재 재생 시간 값과 미래의 재생할 시간을 계산해서 부드럽게 움직일수 있도록함
		FLOAT		fRatio = (fPlayTime - m_KeyFrames[iCurrentKeyFrame].fTime) /
			(m_KeyFrames[iCurrentKeyFrame + 1].fTime - m_KeyFrames[iCurrentKeyFrame].fTime);

		Vector3		vSourScale, vDestScale;
		Vector4		vSourRotation, vDestRotation;
		Vector3		vSourPosition, vDestPosition;

		vSourScale = m_KeyFrames[iCurrentKeyFrame].vScale;
		vDestScale = m_KeyFrames[iCurrentKeyFrame + 1].vScale;

		vSourRotation = m_KeyFrames[iCurrentKeyFrame].vRotation;
		vDestRotation = m_KeyFrames[iCurrentKeyFrame + 1].vRotation;

		vSourPosition = m_KeyFrames[iCurrentKeyFrame].vPosition;
		vDestPosition = m_KeyFrames[iCurrentKeyFrame + 1].vPosition;

		Vector3 Vec3LerpScale = Vector3::Lerp(vSourScale, vDestScale, fRatio);
		Vector4 Vec3LerpRotation = Vector4::Lerp(vSourRotation, vDestRotation, fRatio);
		Vector3 Vec3LerpPosition = Vector3::Lerp(vSourPosition, vDestPosition, fRatio);

		XMStoreFloat3(&vScale, Vec3LerpScale);
		XMStoreFloat4(&vRotation, Vec3LerpRotation);
		XMStoreFloat3(&vPosition, Vec3LerpPosition);

	}

	//XMMatrixAffineTransformation XMVectorSet XMVectorSetW 이 안됌...  
	// 일단 함수 만들어서 사용중
	FMatrix		TransformationMatrix = MatrixAffineTransformation(vScale, vRotation, vPosition);


	if (nullptr != pHierarchyNode)
		pHierarchyNode->Set_Transformation(TransformationMatrix);

	return iCurrentKeyFrame;
}

UAnimChannel* UAnimChannel::Create(fbxsdk::FbxAnimLayer* InAnimLayer, fbxsdk::FbxNode* InNode)
{
	UAnimChannel* pInstanceAnimChannel = new UAnimChannel();

	if (!pInstanceAnimChannel)
		return nullptr;

	//init
	{
		pInstanceAnimChannel->m_FStringName = ANSI_TO_TCHAR(InNode->GetName());

		//scale rot transform 구하기
        FbxAnimCurve* ScalingCurve = InNode->LclScaling.GetCurve(InAnimLayer);
        FbxAnimCurve* RotationCurve = InNode->LclRotation.GetCurve(InAnimLayer);
        FbxAnimCurve* TranslationCurve = InNode->LclTranslation.GetCurve(InAnimLayer);

		// 키프레임 구하기
		int32 NumScalingKeys = ScalingCurve ? ScalingCurve->KeyGetCount() : 0;
		int32 NumRotationKeys = RotationCurve ? RotationCurve->KeyGetCount() : 0;
		int32 NumPositionKeys = TranslationCurve ? TranslationCurve->KeyGetCount() : 0;

		pInstanceAnimChannel->m_iNumKeyFrames = max({ NumScalingKeys, NumRotationKeys, NumPositionKeys });

		Vector4 vRotation;
		Vector3	vScale, vPosition;

		//keyfram에 bone이 움직여야하는 채널 저장하기
		//사용시 해당하는 keyfram을 update하면서 각 scale, rot, position 갱신하면서 불러내면 됌
		for (uint32 i = 0; i < pInstanceAnimChannel->m_iNumKeyFrames; ++i)
		{
			KEYFRAME			KeyFrame;
			ZeroMemory(&KeyFrame, sizeof(KEYFRAME));

			
			if (i < NumScalingKeys)
			{
				memcpy(&vScale, &pInstanceAnimChannel->mScalingKeys[i].mValue, sizeof(Vector3));
				KeyFrame.fTime = pInstanceAnimChannel->mScalingKeys[i].mTime;
			}
			if (i < NumRotationKeys)
			{
				vRotation.x = pInstanceAnimChannel->mNumRotationKeys[i].mValue.x;
				vRotation.y = pInstanceAnimChannel->mNumRotationKeys[i].mValue.y;
				vRotation.z = pInstanceAnimChannel->mNumRotationKeys[i].mValue.z;
				vRotation.w = pInstanceAnimChannel->mNumRotationKeys[i].mValue.w;
				KeyFrame.fTime = pInstanceAnimChannel->mNumRotationKeys[i].mTime;
			}
			if (i < NumPositionKeys)
			{
				memcpy(&vPosition, &pInstanceAnimChannel->mPositionKeys[i].mValue, sizeof(Vector3));
				KeyFrame.fTime = pInstanceAnimChannel->mPositionKeys[i].mTime;
			}

			KeyFrame.vScale = vScale;
			KeyFrame.vRotation = vRotation;
			KeyFrame.vPosition = vPosition;

			pInstanceAnimChannel->m_KeyFrames.push_back(KeyFrame);
		}
	}


	return pInstanceAnimChannel;
}
