#pragma once
#include "CoreMinimal.h"

inline void ApplyWhiteMaterial(UStaticMeshComponent* MeshComponent, UObject* Owner)
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	UMaterialInterface* WhiteBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (WhiteBaseMaterial == nullptr)
	{
		return;
	}

	UMaterialInstanceDynamic* WhiteMaterial = UMaterialInstanceDynamic::Create(WhiteBaseMaterial, Owner);
	if (WhiteMaterial == nullptr)
	{
		return;
	}

	WhiteMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::White);
	WhiteMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
	WhiteMaterial->SetVectorParameterValue(TEXT("Base Color"), FLinearColor::White);
	MeshComponent->SetMaterial(0, WhiteMaterial);
}
