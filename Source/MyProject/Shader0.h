// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shader0.generated.h"

UCLASS()
class MYPROJECT_API AShader0 : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShader0();

	UFUNCTION(BlueprintCallable, Category="test")
	void TestShaderDraw(class UTextureRenderTarget2D* OutputRenderTarget, AActor* Ac, FLinearColor MyColor)
	{
		// UTestShaderBlueprintLibrary::DrawTestShaderRenderTarget(OutputRenderTarget, Ac, MyColor);	
	};
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
