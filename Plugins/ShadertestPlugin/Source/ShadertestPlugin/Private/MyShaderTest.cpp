#pragma once

#include "MyShaderTest.h"  
 
 
#include "Engine/TextureRenderTarget2D.h"  
#include "Engine/World.h"
#include "GlobalShader.h"  
#include "PipelineStateCache.h"  
#include "RHIStaticStates.h"  
#include "SceneUtils.h"  
#include "SceneInterface.h"  
#include "ShaderParameterUtils.h"  
#include "Logging/MessageLog.h"  
#include "Internationalization/Internationalization.h"  
#include "StaticBoundShaderState.h"
#include "RenderCore/Public/Shader.h"
#include "GlobalShader.h"
#include "ShaderParameterMacros.h"
#include "ShaderParameterStruct.h"
#include "ShadertestPlugin.h"
#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"
#include "RHIResources.h"
#include "RenderingThread.h"
#include "Shader.h"


#define LOCTEXT_NAMESPACE "TestShader"  


class FShaderTestVS : public FGlobalShader  
{  
    DECLARE_GLOBAL_SHADER(FShaderTestVS);
 
public:  
    FShaderTestVS(){}  
 
    FShaderTestVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)  
        : FGlobalShader(Initializer)  
    {  
 
    }

    
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)  
    {  
        return true;  
    }
 
    
};  

// BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, )
// SHADER_PARAMETER(FVector4, SimpleColor)
// END_GLOBAL_SHADER_PARAMETER_STRUCT()

BEGIN_UNIFORM_BUFFER_STRUCT(FMyUniformStructData, )  
SHADER_PARAMETER(FVector4, ColorOne)  
SHADER_PARAMETER(FVector4, ColorTwo)  
SHADER_PARAMETER(FVector4, ColorThree)  
SHADER_PARAMETER(FVector4, ColorFour)  
SHADER_PARAMETER(uint32, ColorIndex)  
END_UNIFORM_BUFFER_STRUCT()

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, "FMyUniform");

class FShaderTestPS : public FGlobalShader  
{
    DECLARE_GLOBAL_SHADER(FShaderTestPS);

    
public:  
    FShaderTestPS() {}  

    FShaderTestPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)  
        : FGlobalShader(Initializer)  
    {  
        Color.Bind(Initializer.ParameterMap, TEXT("MyColor"));
        ColorScale.Bind(Initializer.ParameterMap, TEXT("MyColorScale"));
        Texture.Bind(Initializer.ParameterMap, TEXT("MyTexture"));
        TextureSampler.Bind(Initializer.ParameterMap, TEXT("MyTextureSampler"));
    }
    
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)  
    {  
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);  
    }
    
    void SetParameters(  
       FRHICommandListImmediate& RHICmdList,  
       FTextureRHIRef TextureRHI, const FLinearColor& InColor, float InColorScale,
       FMyShaderStructData& ShaderStructData
       )  
    {
        // FShaderTestPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FShaderTestPS::FParameters>();
        // PassParameters->SimpleColor = MyColor;
        FRHIPixelShader* PS = RHICmdList.GetBoundPixelShader();
        SetTextureParameter(RHICmdList, PS, Texture, TextureRHI);
        SetShaderValue(RHICmdList, PS, Color, InColor);
        SetShaderValue(RHICmdList, PS, ColorScale, InColorScale);
    	FMyUniformStructData UniformData;  
    	UniformData.ColorOne = ShaderStructData.ColorOne;  
    	UniformData.ColorTwo = ShaderStructData.ColorTwo;  
    	UniformData.ColorThree = ShaderStructData.ColorThree;  
    	UniformData.ColorFour = ShaderStructData.ColorFour;  
    	UniformData.ColorIndex = ShaderStructData.ColorIndex;

    	TUniformBufferRef<FMyUniformStructData> UniformBuffer = CreateUniformBufferImmediate(UniformData, UniformBuffer_MultiFrame);
    	SetUniformBufferParameter(RHICmdList, PS, GetUniformBufferParameter<FMyUniformStructData>(), UniformBuffer);
    }

private:
    LAYOUT_FIELD(FShaderParameter, Color);
    LAYOUT_FIELD(FShaderParameter, ColorScale);
    LAYOUT_FIELD(FShaderResourceParameter, Texture);
    LAYOUT_FIELD(FShaderResourceParameter, TextureSampler);

};

//***************************************************************************************//
class FMyComputeShader : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FMyComputeShader)
	
public:
	FMyComputeShader()
	{
		
	}
	
	FMyComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
	{
		OutputSurface1.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
	}

	void SetSurfaces(FRHICommandList& RHICmdList, FTexture2DRHIRef &InOutputSurfaceValue,
		FUnorderedAccessViewRHIRef& UAV, FMyShaderStructData& ShaderStructData)
	{
		FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();
		
		RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface1.SetTexture(RHICmdList, ShaderRHI, InOutputSurfaceValue, UAV);
	
		
		FMyUniformStructData UniformData;  
		UniformData.ColorOne = ShaderStructData.ColorOne;  
		UniformData.ColorTwo = ShaderStructData.ColorTwo;  
		UniformData.ColorThree = ShaderStructData.ColorThree;  
		UniformData.ColorFour = ShaderStructData.ColorFour;  
		UniformData.ColorIndex = ShaderStructData.ColorIndex;
		
		TUniformBufferRef<FMyUniformStructData> UniformBuffer = CreateUniformBufferImmediate(UniformData, UniformBuffer_MultiFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FMyUniformStructData>(), UniformBuffer);

	}

	void UnbindBuffers(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef& UAV)
	{
		FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();

		if (OutputSurface1.IsBound())
		{
			RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, UAV);
			OutputSurface1.UnsetUAV(RHICmdList, RHICmdList.GetBoundComputeShader());
		}
	}

private:
	LAYOUT_FIELD(FRWShaderParameter, OutputSurface1);
};


 IMPLEMENT_SHADER_TYPE(, FMyComputeShader, TEXT("/Plugin/ShadertestPlugin/Private/MyShader.usf"), TEXT("MainCS"), SF_Compute)  
 IMPLEMENT_SHADER_TYPE(, FShaderTestVS, TEXT("/Plugin/ShadertestPlugin/Private/MyShader.usf"), TEXT("MainVS"), SF_Vertex)  
 IMPLEMENT_SHADER_TYPE(, FShaderTestPS, TEXT("/Plugin/ShadertestPlugin/Private/MyShader.usf"), TEXT("MainPS"), SF_Pixel)  

struct FCustomVertex
{
    FVector Pos;
    FVector2D UV;
 
    FCustomVertex()
    {
    }
 
    FCustomVertex(const FVector& VertexPos, const FVector2D& VertexUV)
        :Pos(VertexPos), UV(VertexUV)
    {
    }
};
 
//declare custom vertex format
class FMyTestVertexDeclaration : public FRenderResource
{
public:
    FVertexDeclarationRHIRef VertexDeclarationRHI;
 
    virtual void InitRHI() override
    {
        FVertexDeclarationElementList Elements;
        uint16 Stride = sizeof(FCustomVertex);
 
        //Pos-float3
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCustomVertex, Pos), VET_Float3, 0, Stride));
 
        //UV-float2
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCustomVertex, UV), VET_Float2, 1, Stride));
        VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
    }
 
    virtual void ReleaseRHI() override
    {
        VertexDeclarationRHI.SafeRelease();
    }
};
 
TGlobalResource<FMyTestVertexDeclaration> GCustomVertexDeclaration;



static void DrawTestShaderRenderTarget_RenderThread(  
    FRHICommandListImmediate& RHICmdList,   
    FTextureRenderTargetResource* OutputRenderTargetResource,  
    ERHIFeatureLevel::Type FeatureLevel,
    FTextureRHIRef InTextureResource,
    FName TextureRenderTargetName,  
    FLinearColor MyColor,
    FMyShaderStructData MyShaderStructData
)  
{
   check(IsInRenderingThread());
 
	FRHITexture2D* RenderTargetTexture = OutputRenderTargetResource->GetRenderTargetTexture();
	RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::SRVMask, ERHIAccess::RTV));
 
	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::DontLoad_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("Render Test Shader To Texture"));
	{
		FIntPoint ViewportSize(OutputRenderTargetResource->GetSizeX(), OutputRenderTargetResource->GetSizeY());
	
		// Upate viewport
		RHICmdList.SetViewport(0, 0, 0.0f, ViewportSize.X, ViewportSize.Y, 1.0f);
 
		// Get Shaders
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FShaderTestVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FShaderTestPS> PixelShader(GlobalShaderMap);
 
		// Setup Pipeline state
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCustomVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
 
		// update shader uniform parameters
		PixelShader->SetParameters(RHICmdList, InTextureResource, MyColor, 1.0, MyShaderStructData);
 
		// Create VertexBuffer and setup
		static const uint32 VERTEX_SIZE = sizeof(FCustomVertex) * 4;
		FRHIResourceCreateInfo CreateInfo;
		FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VERTEX_SIZE, BUF_Static, CreateInfo);
		void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VERTEX_SIZE, RLM_WriteOnly);
 
		FCustomVertex Vertices[4];
		Vertices[0] = FCustomVertex(FVector(-1.0f, 1.0f, 0.0f), FVector2D(0.0f, 0.0f));
		Vertices[1] = FCustomVertex(FVector(1.0f, 1.0f, 0), FVector2D(1.0f, 0.0f));
		Vertices[2] = FCustomVertex(FVector(-1.0f, -1.0f, 0), FVector2D(0.0f, 1.0f));
		Vertices[3] = FCustomVertex(FVector(1.0f, -1.0f, 0), FVector2D(1.0f, 1.0f));
		FMemory::Memcpy(VoidPtr, (void*)Vertices, VERTEX_SIZE);
		RHIUnlockVertexBuffer(VertexBufferRHI);
 
		RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
		RHICmdList.DrawPrimitive(0, 2, 1);
	}
	RHICmdList.EndRenderPass();
 
	RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::RTV, ERHIAccess::SRVMask));
}  
 
void UTestShaderBlueprintLibrary::DrawTestShaderRenderTarget(  
    UTextureRenderTarget2D* OutputRenderTarget,   
    AActor* Ac,  
    FLinearColor MyColor,
    UTexture* MyTexture,
    FMyShaderStructData MyShaderStructData
)  
{
    UE_LOG(LogTemp, Warning, TEXT("harylv: DrawTestShaderRenderTarget"));
    check(IsInGameThread());  
 
    if (!OutputRenderTarget)  
    {  
        return;  
    }

    if (MyTexture == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("MyTexture Is NULL"));
        return;
    }
    FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
    FTexture* TextureSource = MyTexture->Resource;
    UWorld* World = Ac->GetWorld();  
    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();  
    FName TextureRenderTargetName = OutputRenderTarget->GetFName();
    // FTextureReferenceRHIRef TextureRHI = MyTexture->TextureReference.TextureReferenceRHI;

    ENQUEUE_RENDER_COMMAND(CaptureCommand)(  
        [TextureRenderTargetResource,TextureSource, FeatureLevel, MyColor, TextureRenderTargetName, MyShaderStructData](FRHICommandListImmediate& RHICmdList)  
        {
           // DrawTestShaderRenderTarget_RenderThread(RHICmdList,TextureRenderTargetResource, FeatureLevel, TextureSource->TextureRHI, TextureRenderTargetName, MyColor, MyShaderStructData);  
        }  
    );  
 
}  

/*****************************************************/
static void UseComputeShader_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* OutputRenderTargetResource,
	FMyShaderStructData ShaderStructData,
	ERHIFeatureLevel::Type FeatureLevel
)
{
	check(IsInRenderingThread());

	TShaderMapRef<FMyComputeShader> ComputeShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());

	int32 SizeX = OutputRenderTargetResource->GetSizeX();
	int32 SizeY = OutputRenderTargetResource->GetSizeY();
	
	FRHIResourceCreateInfo CreateInfo;
	FTexture2DRHIRef RenderTargetTexture = OutputRenderTargetResource->GetRenderTargetTexture();
	FTexture2DRHIRef GSurfaceTexture2D = RHICreateTexture2D(RenderTargetTexture->GetSizeX(), RenderTargetTexture->GetSizeY(), PF_FloatRGBA, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	FUnorderedAccessViewRHIRef TextureUAV = RHICreateUnorderedAccessView(GSurfaceTexture2D);
	ComputeShader->SetSurfaces(RHICmdList, RenderTargetTexture, TextureUAV, ShaderStructData);
	DispatchComputeShader(RHICmdList, ComputeShader, SizeX/32, SizeY/32, 1);
	ComputeShader->UnbindBuffers(RHICmdList, TextureUAV);


	// FRHICopyTextureInfo CopyInfo;
	// RHICmdList.CopyTexture(GSurfaceTexture2D, RenderTargetTexture, CopyInfo);


	FName Test("harylv");
	DrawTestShaderRenderTarget_RenderThread(RHICmdList,OutputRenderTargetResource, FeatureLevel, static_cast<FTextureRHIRef>(GSurfaceTexture2D), Test, FLinearColor(), ShaderStructData);  


}

 void UTestShaderBlueprintLibrary::UseMyComputeShader(
		UTextureRenderTarget2D* OutputRenderTarget,
		AActor* Ac,
		FMyShaderStructData ShaderStructData
		)
{
	check(IsInGameThread());

	if (Ac == nullptr && OutputRenderTarget == nullptr)
	{
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	UWorld* World = Ac->GetWorld();
	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel, ShaderStructData](FRHICommandListImmediate& RHICmdList)
	{
		UseComputeShader_RenderThread
		(
			RHICmdList,
			TextureRenderTargetResource,
			ShaderStructData,
			FeatureLevel
		);
	});
}
#undef LOCTEXT_NAMESPACE  