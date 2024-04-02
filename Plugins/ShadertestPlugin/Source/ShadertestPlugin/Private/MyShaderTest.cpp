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

    static bool ShouldCache(EShaderPlatform Platform)  
    {  
        return true;  
    }  
 
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)  
    {  
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);  
        return true;  
    }
 
    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)  
    {  
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);  
        OutEnvironment.SetDefine(TEXT("TEST_MICRO"), 1);  
    }  
};  

// BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, )
// SHADER_PARAMETER(FVector4, SimpleColor)
// END_GLOBAL_SHADER_PARAMETER_STRUCT()
//
// IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMyUniformStructData, "FMyUnifrom");

class FShaderTestPS : public FGlobalShader  
{
    DECLARE_GLOBAL_SHADER(FShaderTestPS);

    
public:  
    FShaderTestPS() {}  




    FShaderTestPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)  
        : FGlobalShader(Initializer)  
    {  
       SimpleColorVal.Bind(Initializer.ParameterMap, TEXT("SimpleColor"));
       //  UE_LOG(LogTemp, Error, TEXT("harylv: error"));
    }

   

    static bool ShouldCache(EShaderPlatform Platform)  
    {  
        return true;  
    }  
 
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)  
    {  
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);  
    }
   
    
    void SetParameters(  
       FRHICommandListImmediate& RHICmdList,  
       const FVector4& MyColor  
       )  
    {
        // FShaderTestPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FShaderTestPS::FParameters>();
        // PassParameters->SimpleColor = MyColor;
        FRHIPixelShader* PS = RHICmdList.GetBoundPixelShader();
        SetShaderValue(RHICmdList, PS, SimpleColorVal, MyColor);
        // SetUniformBufferParameterImmediate(RHICmdList, PS, GetUniformBufferParameter<FMyUniformStructData>(), ShaderData);
    }

private:
    
    LAYOUT_FIELD(FShaderParameter, SimpleColorVal);
};




IMPLEMENT_SHADER_TYPE(, FShaderTestVS, TEXT("/Plugin/ShadertestPlugin/Private/MyShader.usf"), TEXT("MainVS"), SF_Vertex)  
IMPLEMENT_SHADER_TYPE(, FShaderTestPS, TEXT("/Plugin/ShadertestPlugin/Private/MyShader.usf"), TEXT("MainPS"), SF_Pixel)  
 
static void DrawTestShaderRenderTarget_RenderThread(  
    FRHICommandListImmediate& RHICmdList,   
    FTextureRenderTargetResource* OutputRenderTargetResource,  
    ERHIFeatureLevel::Type FeatureLevel,
    FTexture2DRHIRef InTexture,
    FName TextureRenderTargetName,  
    FLinearColor MyColor  
)  
{
    check(IsInRenderingThread());  
    FRHITexture2D* RenderTargetTexture = OutputRenderTargetResource->GetRenderTargetTexture();
    // RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, RenderTargetTexture);
    RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::SRVMask, ERHIAccess::RTV));  
    FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::DontLoad_Store);

    //设置渲染目标  
    RHICmdList.BeginRenderPass(RPInfo, TEXT("shaderTest"));
    {
        FIntPoint ViewportSize(OutputRenderTargetResource->GetSizeX(), OutputRenderTargetResource->GetSizeY());

        // Update viewport
        RHICmdList.SetViewport(0, 0, 0.0f, ViewportSize.X, ViewportSize.Y, 1.0f);

        // Get Shaders
        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(FeatureLevel);  
        TShaderMapRef<FShaderTestVS> VertexShader(ShaderMap);  
        TShaderMapRef<FShaderTestPS> PixelShader(ShaderMap);

        // Setup Pipeline state
        FGraphicsPipelineStateInitializer GraphicsPSOInit;  
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);  
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();  
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();  
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();  
        GraphicsPSOInit.PrimitiveType = PT_TriangleStrip;  
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();  
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI =   VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();  
        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Update shader uniform parameters
        PixelShader->SetParameters(RHICmdList, MyColor);

        // Create VertexBuffer and setup
        static const uint32 VERTEX_SIZE = sizeof(FVector4) * 4;
        FRHIResourceCreateInfo CreateInfo;
        FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VERTEX_SIZE, BUF_Static, CreateInfo);
        void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VERTEX_SIZE, RLM_WriteOnly);

        FVector4 Vertices[4];  
        Vertices[0].Set(-1.0f, 1.0f, 0, 1.0f);  
        Vertices[1].Set(1.0f, 1.0f, 0, 1.0f);  
        Vertices[2].Set(-1.0f, -1.0f, 0, 1.0f);  
        Vertices[3].Set(1.0f, -1.0f, 0, 1.0f);
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
    UTexture* MyTexture
)  
{
    UE_LOG(LogTemp, Warning, TEXT("harylv: DrawTestShaderRenderTarget"));
    check(IsInGameThread());  
 
    if (!OutputRenderTarget)  
    {  
        return;  
    }  
    OutputRenderTarget->UpdateResourceImmediate(true);
    FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();  
    UWorld* World = Ac->GetWorld();  
    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();  
    FName TextureRenderTargetName = OutputRenderTarget->GetFName();
    // FTextureReferenceRHIRef TextureRHI = MyTexture->TextureReference.TextureReferenceRHI;

    ENQUEUE_RENDER_COMMAND(CaptureCommand)(  
        [TextureRenderTargetResource, FeatureLevel, MyColor, TextureRenderTargetName](FRHICommandListImmediate& RHICmdList)  
        {
            DrawTestShaderRenderTarget_RenderThread(RHICmdList,TextureRenderTargetResource, FeatureLevel, nullptr, TextureRenderTargetName, MyColor);  
        }  
    );  
 
}  
 
#undef LOCTEXT_NAMESPACE  