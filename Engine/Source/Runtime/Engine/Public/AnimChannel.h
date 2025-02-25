#pragma once

#include "CoreMinimal.h"
#include "AnimChannel.generated.h"

class UFbxFactory;



UCLASS()
class ENGINE_API UAnimChannel : public UObject
{
	GENERATED_BODY()
public:
	UAnimChannel();
	virtual ~UAnimChannel() = default;


	const FString GetName() const { return m_FStringName; }

	uint32 Update_Transformation(FLOAT fPlayTime, uint32 iCurrentKeyFrame, class UHierarchy* pHierarchyNode);



private:
	FString							m_FStringName;
	uint32							m_iNumKeyFrames = 0;
	vector<KEYFRAME>				m_KeyFrames;

	aiVectorKey* mScalingKeys;
	aiQuatKey* mNumRotationKeys;
	aiVectorKey* mPositionKeys;
	//VectorKey* mPositionKeys;



public:
	static UAnimChannel* Create(fbxsdk::FbxAnimLayer* InAnimLayer, fbxsdk::FbxNode* InNode);

    FMatrix MatrixAffineTransformation
    (
        Vector3& Scaling,
        Vector4& RotationQuaternion,
        Vector3& Translation
    ) const
    {
        FMatrix ScaleMatrix;
        FMatrix RotationMatrix;
        FMatrix TranslationMatrix;
        FMatrix ResultMatrix;

        ScaleMatrix._11 = Scaling.x;
        ScaleMatrix._22 = Scaling.y;
        ScaleMatrix._33 = Scaling.z;
        ScaleMatrix._44 = 1.0f;

        //RotationQuaternion.w
        FQuat RotationQuat(RotationQuaternion.x, RotationQuaternion.y, RotationQuaternion.z, RotationQuaternion.w);
        RotationMatrix._11 = 1 - 2 * (RotationQuaternion.y * RotationQuaternion.y + RotationQuaternion.z * RotationQuaternion.z);
        RotationMatrix._12 = 2 * (RotationQuaternion.x * RotationQuaternion.y - RotationQuaternion.w * RotationQuaternion.z);
        RotationMatrix._13 = 2 * (RotationQuaternion.x * RotationQuaternion.z + RotationQuaternion.w * RotationQuaternion.y);
        RotationMatrix._14 = 0;

        RotationMatrix._21 = 2 * (RotationQuaternion.x * RotationQuaternion.y + RotationQuaternion.w * RotationQuaternion.z);
        RotationMatrix._22 = 1 - 2 * (RotationQuaternion.x * RotationQuaternion.x + RotationQuaternion.z * RotationQuaternion.z);
        RotationMatrix._23 = 2 * (RotationQuaternion.y * RotationQuaternion.z - RotationQuaternion.w * RotationQuaternion.x);
        RotationMatrix._24 = 0;

        RotationMatrix._31 = 2 * (RotationQuaternion.x * RotationQuaternion.z - RotationQuaternion.w * RotationQuaternion.y);
        RotationMatrix._32 = 2 * (RotationQuaternion.y * RotationQuaternion.z + RotationQuaternion.w * RotationQuaternion.x);
        RotationMatrix._33 = 1 - 2 * (RotationQuaternion.x * RotationQuaternion.x + RotationQuaternion.y * RotationQuaternion.y);
        RotationMatrix._34 = 0;

        RotationMatrix._41 = 0;
        RotationMatrix._42 = 0;
        RotationMatrix._43 = 0;
        RotationMatrix._44 = 1;

        TranslationMatrix = FMatrix::Identity;
        TranslationMatrix._41 = Translation.x;
        TranslationMatrix._42 = Translation.y;
        TranslationMatrix._43 = Translation.z;

        ResultMatrix = ScaleMatrix * RotationMatrix * TranslationMatrix;

        return ResultMatrix;

    }



};