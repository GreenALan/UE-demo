#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyShaderTest.generated.h"



UCLASS()
class SHADERTESTPLUGIN_API UTestShaderBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ShaderTestPlugin")
	static void DrawTestShaderRenderTarget(class UTextureRenderTarget2D* OutputRenderTarget, AActor* Ac, FLinearColor MyColor, UTexture* MyTexture);

	UFUNCTION(BlueprintCallable, Category = "ShaderTestPlugin")
	static bool TestBluePrint()
	{
		return true;
	}
};


USTRUCT(BlueprintType)
struct FMyShaderData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = ShaderData)
	FVector4 MyColor;
	
};
