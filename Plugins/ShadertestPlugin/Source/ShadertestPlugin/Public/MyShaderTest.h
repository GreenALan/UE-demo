#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyShaderTest.generated.h"

USTRUCT(BlueprintType)
struct FMyShaderStructData  
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)  
	FLinearColor ColorOne;  
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)  
	FLinearColor ColorTwo;  
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)  
	FLinearColor ColorThree;  
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)  
	FLinearColor ColorFour;  
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)  
	int32 ColorIndex;  
	
};

UCLASS()
class SHADERTESTPLUGIN_API UTestShaderBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ShaderTestPlugin")
	static void DrawTestShaderRenderTarget(class UTextureRenderTarget2D* OutputRenderTarget, AActor* Ac, FLinearColor MyColor, UTexture* MyTexture, FMyShaderStructData MyShaderStructData);

	UFUNCTION(BlueprintCallable, Category = "ShaderTestPlugin")
	static void UseMyComputeShader(
		UTextureRenderTarget2D* OutputRenderTarget,
		AActor* Ac,
		FMyShaderStructData ShaderStructData
		);

	UFUNCTION(BlueprintCallable, Category = "ShaderTestPlugin")
	static bool TestBluePrint()
	{
		return true;
	}
};



