// Copyright (C) 2019-2020 Blue Mountains GmbH. All Rights Reserved.

#include "PakLoader.h"
#include "PakLoaderModule.h"
#include "Runtime/Core/Public/Serialization/ArrayReader.h"
#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "AssetRegistryState.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "LogHelper.h"

FPakLoader *FPakLoader::Instance = nullptr;

FPakLoader::FPakLoader()
{
	UE_LOG(LogPakLoader, Log, TEXT("FPakLoader::FPakLoader()"));
}

FPakLoader::~FPakLoader()
{
	UE_LOG(LogPakLoader, Log, TEXT("FPakLoader::FPakLoader()"));

	ResetPlatformFile();
}

FPakLoader *FPakLoader::Get()
{
	if (!Instance)
	{
		Instance = new FPakLoader();
	}

	return Instance;
}

FPakPlatformFile *FPakLoader::GetPakPlatformFile()
{
	if (!PakPlatformFile)
	{
		/*
			Packaged shipping builds will have a PakFile platform.
			For other build types a new pak platform file will be created.
		*/
		// IPlatformFile *CurrentPlatformFile = FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile"));
		IPlatformFile *CurrentPlatformFile = FPlatformFileManager::Get().FindPlatformFile(FPakPlatformFile::GetTypeName());
		if (CurrentPlatformFile)
		{
			FLogHelper::Log(LL_VERBOSE, TEXT("Found PakPlatformFile"));

			PakPlatformFile = static_cast<FPakPlatformFile*>(CurrentPlatformFile);
		}
		else
		{
			PakPlatformFile = new FPakPlatformFile();

			ensure(PakPlatformFile != nullptr);

			IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

#if WITH_EDITOR
			// Keep the original platform file for non packaged builds.
			OriginalPlatformFile = &PlatformFile;
#endif

			if (PakPlatformFile->Initialize(&PlatformFile, TEXT("")))
			{
				FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
			}
			else
			{
				FLogHelper::Log(LL_VERBOSE, TEXT("Failed to initialize PakPlatformFile"));
			}
		}
	}

	ensure(PakPlatformFile != nullptr);
	return PakPlatformFile;
}

void FPakLoader::ResetPlatformFile()
{
#if WITH_EDITOR

	/*
		In editor builds the original platform file must be restored upon exiting PIE.
		Otherwise the current session will stay in a broken state without being able to save assets.
	*/

	if (OriginalPlatformFile)
	{
		FPlatformFileManager::Get().SetPlatformFile(*OriginalPlatformFile);
	}
#endif
}

TArray<FString> FPakLoader::GetMountedPakFilenames()
{
	TArray<FString> MountedPakFilenames;
	GetPakPlatformFile()->GetMountedPakFilenames(MountedPakFilenames);
	return MountedPakFilenames;
}

bool FPakLoader::IsValidPakFile(const FString &PakFilename, int64 &OutPakSize, bool bSigned)
{
	if (!FPaths::FileExists(PakFilename))
	{
		return false;
	}

	FPakFile* Pak = nullptr;

#if ENGINE_MINOR_VERSION >= 27 || ENGINE_MAJOR_VERSION == 5
	TRefCountPtr<FPakFile> PakFile = new FPakFile(GetPakPlatformFile(), *PakFilename, bSigned);
	Pak = PakFile.GetReference();
#else
	FPakFile PakFile(GetPakPlatformFile(), *PakFilename, bSigned);
	Pak = &PakFile;
#endif

	if (!Pak->IsValid())
	{
#if ENGINE_MINOR_VERSION >= 27 || ENGINE_MAJOR_VERSION == 5
		PakFile.SafeRelease();
#endif
		return false;
	}

	OutPakSize = Pak->TotalSize();

#if ENGINE_MINOR_VERSION >= 27 || ENGINE_MAJOR_VERSION == 5
	PakFile.SafeRelease();
#endif
	return true;
}

int32 FPakLoader::GetPakOrderFromPakFilename(const FString& PakFilePath)
{
	if (PakFilePath.StartsWith(FString::Printf(TEXT("%sPaks/%s-"), *FPaths::ProjectContentDir(), FApp::GetProjectName())))
	{
		return 4;
	}
	else if (PakFilePath.StartsWith(FPaths::ProjectContentDir()))
	{
		return 3;
	}
	else if (PakFilePath.StartsWith(FPaths::EngineContentDir()))
	{
		return 2;
	}
	else if (PakFilePath.StartsWith(FPaths::ProjectSavedDir()))
	{
		return 1;
	}

	return 0;
}

bool FPakLoader::MountPakFile(const FString &PakFilename, int32 PakOrder, const FString &MountPath)
{
	if (PakOrder == INDEX_NONE)
	{
		PakOrder = GetPakOrderFromPakFilename(PakFilename);
	}

	bool bResult = false;
	if (MountPath.Len() > 0)
	{
		bResult = GetPakPlatformFile()->Mount(*PakFilename, PakOrder, *MountPath);
	}
	else
	{
		// NULL will make the mount to use the pak's mount point
		bResult = GetPakPlatformFile()->Mount(*PakFilename, PakOrder, NULL);
	}
	return bResult;
}

bool FPakLoader::UnmountPakFile(const FString &PakFilename)
{
	return GetPakPlatformFile()->Unmount(*PakFilename);
}

void FPakLoader::RegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::RegisterMountPoint(RootPath, ContentPath);
}

void FPakLoader::UnRegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::UnRegisterMountPoint(RootPath, ContentPath);
}

TArray<FString> FPakLoader::GetFilesInDirectory(const FString &Directory)
{
	FPakLoaderFileVisitor Visitor;
	GetPakPlatformFile()->IterateDirectory(*Directory, Visitor);
	return Visitor.Files;
}

TArray<FString> FPakLoader::GetFilesInDirectoryRecursively(const FString &Directory)
{
	FPakLoaderFileVisitor Visitor;
	GetPakPlatformFile()->IterateDirectoryRecursively(*Directory, Visitor);
	return Visitor.Files;
}

TArray<FString> FPakLoader::GetFilesInPak(const FString &PakFilename, bool bUAssetOnly)
{
	FPakFile* Pak = nullptr;

#if ENGINE_MINOR_VERSION >= 27 || ENGINE_MAJOR_VERSION == 5
	TRefCountPtr<FPakFile> PakFile = new FPakFile(GetPakPlatformFile(), *PakFilename, false);
	Pak = PakFile.GetReference();
#else
	FPakFile PakFile(GetPakPlatformFile(), *PakFilename, false);
	Pak = &PakFile;
#endif

	TArray<FString> PakItemsNames;
	if (Pak->IsValid())
	{
#if ENGINE_MINOR_VERSION <= 25 && ENGINE_MAJOR_VERSION == 4
		TArray<FPakFile::FFileIterator> Records;

		for (FPakFile::FFileIterator It(PakFile, false); It; ++It)
#else
		TArray<FPakFile::FFilenameIterator> Records;

		for (FPakFile::FFilenameIterator It(*Pak, false); It; ++It)
#endif
		{
			Records.Add(It);
		}

		for (auto &It : Records)
		{
			if (bUAssetOnly)
			{
				if (FPaths::GetExtension(It.Filename()) == TEXT("uasset"))
				{
					PakItemsNames.Add(It.Filename());
				}
			}
			else
			{
				PakItemsNames.Add(It.Filename());
			}

		}
	}

#if ENGINE_MINOR_VERSION >= 27 || ENGINE_MAJOR_VERSION == 5
	PakFile.SafeRelease();
#endif
	return PakItemsNames;
}

void FPakLoader::LoadAssetRegistryFile(const FString &AssetRegistryFile)
{
	if (DoesFileExist(AssetRegistryFile))
	{
		FArrayReader SerializedAssetData;

		if (FFileHelper::LoadFileToArray(SerializedAssetData, *AssetRegistryFile))
		{
			SerializedAssetData.Seek(0);

			static const FName AssetRegistryModuleName(TEXT("AssetRegistry"));
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryModuleName);

#if ENGINE_MINOR_VERSION >= 27 || ENGINE_MAJOR_VERSION == 5
			FAssetRegistryState PakState;
			PakState.Load(SerializedAssetData);
			AssetRegistryModule.Get().AppendState(PakState);
#else
			AssetRegistryModule.Get().Serialize(SerializedAssetData);
#endif
		}
	}
}

bool FPakLoader::DoesDirectoryExist(const FString &Directory)
{
	return GetPakPlatformFile()->DirectoryExists(*Directory);
}

bool FPakLoader::DoesFileExist(const FString &Filename)
{
	return GetPakPlatformFile()->FileExists(*Filename);
}

UClass *FPakLoader::LoadClassFromPak(const FString &Filename)
{
	const FString Name = Filename + TEXT(".") + FPackageName::GetShortName(Filename) + TEXT("_C");
	return StaticLoadClass(UObject::StaticClass(), nullptr, *Name);
}

bool FPakLoader::ReadStringFromPak(const FString &Filename, FString &OutStr)
{
	return FFileHelper::LoadFileToString(OutStr, *Filename);
}
