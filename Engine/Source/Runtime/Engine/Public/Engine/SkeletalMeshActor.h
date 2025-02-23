#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "SkeletalMeshActor.generated.h"

UCLASS()
class ASkeletalMeshActor : public AActor
{
public:


	virtual void OnPropertyChanged(FProperty&) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;


private:
	//UPROPERTY()
	USkeltalMeshComponent* SkeltalMeshComponent;


	UPROPERTY(EditAnywhere)
	int RasterizerState = 0;
};
