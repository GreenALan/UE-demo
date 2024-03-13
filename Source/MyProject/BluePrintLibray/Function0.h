// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Function0.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UFunction0 : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category="test")
	static bool TestFunction0()
	{
		return true;
	}
};
