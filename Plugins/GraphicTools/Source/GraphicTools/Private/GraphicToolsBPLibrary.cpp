// Copyright Epic Games, Inc. All Rights Reserved.

#include "GraphicToolsBPLibrary.h"
#include "GraphicTools.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "GlobalShader.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "SceneUtils.h"
#include "SceneInterface.h"
#include "ShaderParameterUtils.h"
#include "Logging/MessageLog.h"
#include "Internationalization/Internationalization.h"
#include "Runtime/RenderCore/Public/RenderTargetPool.h"


UGraphicToolsBPLibrary::UGraphicToolsBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

class FCheckerBoardComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FCheckerBoardComputeShader, Global)

public:
	FCheckerBoardComputeShader() {}
	FCheckerBoardComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		OutputSurface.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
	}

	void SetParameters(
		FRHICommandList& RHICmdList,
		FTexture2DRHIRef& InOutputSurfaceValue,
		FUnorderedAccessViewRHIRef& UAV
	)
	{
		FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();

		RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface.SetTexture(RHICmdList, ShaderRHI, InOutputSurfaceValue, UAV);
	}

	void UnsetParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef& UAV)
	{
		RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface.UnsetUAV(RHICmdList, RHICmdList.GetBoundComputeShader());
	}

private:

	LAYOUT_FIELD(FRWShaderParameter, OutputSurface);
};
IMPLEMENT_SHADER_TYPE(, FCheckerBoardComputeShader, TEXT("/Plugin/GraphicTools/Private/CheckerBoard.usf"), TEXT("MainCS"), SF_Compute);


static void DrawCheckerBoard_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* TextureRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel
)
{
	check(IsInRenderingThread());

	FTexture2DRHIRef RenderTargetTexture = TextureRenderTargetResource->GetRenderTargetTexture();
	uint32 GGroupSize = 32;
	FIntPoint FullResolution = FIntPoint(RenderTargetTexture->GetSizeX(), RenderTargetTexture->GetSizeY());
	uint32 GroupSizeX = FMath::DivideAndRoundUp((uint32)RenderTargetTexture->GetSizeX(), GGroupSize);
	uint32 GroupSizeY = FMath::DivideAndRoundUp((uint32)RenderTargetTexture->GetSizeY(), GGroupSize);

	TShaderMapRef<FCheckerBoardComputeShader>ComputeShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());

	FRHIResourceCreateInfo CreateInfo;
	//Create a temp resource
	FTexture2DRHIRef GSurfaceTexture2D = RHICreateTexture2D(RenderTargetTexture->GetSizeX(), RenderTargetTexture->GetSizeY(), PF_FloatRGBA, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	FUnorderedAccessViewRHIRef GUAV = RHICreateUnorderedAccessView(GSurfaceTexture2D);

	ComputeShader->SetParameters(RHICmdList, RenderTargetTexture, GUAV);
	DispatchComputeShader(RHICmdList, ComputeShader, GroupSizeX, GroupSizeY, 1);
	ComputeShader->UnsetParameters(RHICmdList, GUAV);

	// RHICmdList.TransitionResource(ERHIAccess::CopySrc, GSurfaceTexture2D);
	// RHICmdList.TransitionResource(ERHIAccess::CopyDest, RenderTargetTexture);

	FRHICopyTextureInfo CopyInfo;
	RHICmdList.CopyTexture(GSurfaceTexture2D, RenderTargetTexture, CopyInfo);
}

void UGraphicToolsBPLibrary::DrawCheckerBoard(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget)
{
	check(IsInGameThread());

	if (!OutputRenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("OutputRenderTarget is nullptr"));
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	ERHIFeatureLevel::Type FeatureLevel = WorldContextObject->GetWorld()->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[TextureRenderTargetResource, FeatureLevel](FRHICommandListImmediate& RHICmdList)
		{
			DrawCheckerBoard_RenderThread
			(
				RHICmdList,
				TextureRenderTargetResource,
				FeatureLevel
			);
		}
	);
}

float UGraphicToolsBPLibrary::GraphicToolsSampleFunction(float Param)
{
	return -1;
}

