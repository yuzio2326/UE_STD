#pragma once
#include "Components/MeshComponent.h"
#include "StaticMeshComponent.generated.h"


// Load 할때 Bone or Anim이 있는지 체크하고 있으면 Static말고 SkeletalMesh로 로딩 하기
// defaultAnim 은 0번으로 처리해서 IDLE 을 사용하도록 하고 움직이고 싶으면 XYZ 축 관련 
// UI말고 다른것도 추가적으로 넣어가지고 Anim을 바꾸거나 특정 커멘드로 바꿔서 움직이게 하는것도 ㄱㅊ음
// gilford 관련 커멘드 참고도 ㄱㅊ을지도?

// HierarchyNode 필요함
// Model 부분을 얘가 역할 할거임
// Anim이랑 Channel 필요함 
// texture 및 shader 관련은 mat가져가서 사용
// static mesh의 자식으로 해서 해당 매쉬의 일반적인 쉐이딩이나 
// mat관련은 모두 로드 해서 가져오도록 하고 거기에 추가로 skeletal 이 있으면 얘로 설정하도록 합시다



// 순서 관련
// 로딩 할때 skeletone 유무 검사-> 이후 있으면 얘가 호출 없으면 StaticMesh
// 얘는 StaticMesh의 자손으로 만들어 놓고 해당 로드 기능을 모두 구현한걸 그대로 가져다 쓰고
// HierarchyNode를 들리고 skeletone을 모두 조회한 이후 각 channel을 만들어서 skeletine을 만들고 다시 HierarchyNode ㄱㄱ
// HierarchyNode는 이후 Anim을 조회 하고 skeletone에 각자 자신의 이름에 맞는 애니메이션을 설정할 Animation을 세팅
// Animation은 이제 로드를 하면 skeletone의 이름을 조회 하고 각 채널의 맞는 이름에 실행할 애니메이션에 맞춰서 움직임을 부여하기

// meshContainer 만들어서 여러개의 mesh를 로드 하는걸 이쪽에서 로드하도록 하는게 좋을거 같음
// 임시로 만들어 놓고 하나하나 주석 풀면서 만들기 ㄱㄱ



class USkeletalMesh;		// MeshContainer 역할이 얘로 바뀔 예정
class UMaterial;			// Mat가지고 와서 사용 ㄱㄱ




UCLASS()
class ENGINE_API USkeltalMeshComponent : public UMeshComponent
{
	GENERATED_BODY()
public:
	//USkeltalMeshComponent();
	
	//void SetSkeletalMesh(TEnginePtr<USkeletalMesh> NewSkeletalMesh);


	//TEnginePtr<USkeletalMesh> GetSkeletalMesh() const { return SkeletalMesh; }
	//TEnginePtr<UMaterial> GetMaterial(int32 MaterialIndex);
	//TArray<TObjectPtr<UMaterial>>& GetOverrideMaterials() { return OverrideMaterials; }
	//void SetMaterial(TObjectPtr<UMaterial> NewMaterial, int32 MaterialIndex);
	//uint32 GetMaterialCount();

protected:
	//virtual bool ShouldCreateRenderState() const;

private:
	//virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

private:
	UPROPERTY(EditAnywhere)
	TEnginePtr<USkeletalMesh> SkeletalMesh;

	TArray<TObjectPtr<UMaterial>> OverrideMaterials;

	//void SetHierarchy(class USkeltalMeshComponent* pModel, USkeletalMesh* pAIMesh);	//USkeltalMeshComponent 안에 있는 하이어라키 노드 가져오기 <-skeletamesh에서 호출시키기
	//여기 클래스에 하이어라키 노드세팅하기
	
	//USkeltalMeshComponent가 PersonalSetting관련 된 애들이 있고 그걸 얘가 다 사용을 함



	//만드는 순서 하이어라키 노드 -> USkeltalMeshComponent만들면서 Anim이랑 Channel 이후 Meshcontainer부분
	//그러면 지금 당장 필요한건 하이어라키 노드 부분 ㄱㄱ
	

};

