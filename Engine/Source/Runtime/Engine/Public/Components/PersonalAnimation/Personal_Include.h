#pragma once

struct FBXNode	//Personal_Include.h 여기로 뺄지는 나중에 판단 ㄱㄱ
{
	FString NodeName;
	FTransform LocalTransform;
	FBXNode* Parent = nullptr;
	TArray<FBXNode*> Children;
};

typedef struct tagKeyFrame
{
    float		fTime;

    Vector3	vScale;
    Vector4	vRotation;
    Vector3	vPosition;
}KEYFRAME;

struct aiVectorKey {
    double mTime;      // 키프레임 시간 (초 단위)
    FVector3D mValue; // 위치 또는 스케일 값 (X, Y, Z)
};

struct aiQuatKey {
    double mTime;     // 키프레임 시간 (초 단위)
    FQuat mValue; // 회전 값 (쿼터니언)
};



struct aiNodeAnim {
    FString mNodeName;             // 애니메이션이 적용되는 본(Bone)의 이름

    unsigned int mNumPositionKeys;  // 위치(Position) 키프레임 개수
    aiVectorKey* mPositionKeys;     // 위치 키프레임 배열

    unsigned int mNumRotationKeys;  // 회전(Rotation) 키프레임 개수
    aiQuatKey* mRotationKeys;       // 회전 키프레임 배열

    unsigned int mNumScalingKeys;   // 스케일(Scaling) 키프레임 개수
    aiVectorKey* mScalingKeys;      // 스케일 키프레임 배열

    unsigned int mPreState;         // 애니메이션 이전 상태 (사용되지 않음)
    unsigned int mPostState;        // 애니메이션 이후 상태 (사용되지 않음)
};

