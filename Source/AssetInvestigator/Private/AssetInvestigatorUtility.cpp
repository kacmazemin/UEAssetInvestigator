// Fill out your copyright notice in the Description page of Project Settings.

#include "AssetInvestigatorUtility.h"
#include "AssetInfo.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

#include "Misc/ScopedSlowTask.h"
#include <string>

#define LOCTEXT_NAMESPACE "FAssetInvestigatorModule"

void AssetInvestigatorUtility::OpenSelectedAsset(const FName& AssetPath)
{
	const FAssetData& AssetData = IAssetRegistry::Get()->GetAssetByObjectPath(AssetPath);
	if (AssetData.IsValid())
	{
		FScopedSlowTask SlowTask(0, LOCTEXT("LoadingSelectedObject", "Editing assets..."));
		SlowTask.MakeDialogDelayed(.1f);

		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

		UObject* EditObject = AssetData.GetAsset();
		if (EditObject)
		{
			AssetEditorSubsystem->OpenEditorForAsset(EditObject);
		}
	}
}

void AssetInvestigatorUtility::BrowseToAsset(const FName& AssetPath)
{
	const FAssetData& AssetData = IAssetRegistry::Get()->GetAssetByObjectPath(AssetPath);

	if (AssetData.IsValid())
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets({ AssetData });
	}
}

void AssetInvestigatorUtility::ExportListToTxt(const TArray<FAssetInfo>& AssetInfoList)
{
	if (AssetInfoList.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("The empty list can not be exported"));
		return;
	}

	const FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("AssetInvestigatorReport.txt");

	// Create a file handle
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* FileHandle = PlatformFile.OpenWrite(*FilePath);

	if (FileHandle)
	{
		// Iterate through the FString array and write each string to the file
		for (const FAssetInfo& SingleAssetData : AssetInfoList)
		{
			// Convert FString to ANSI string and write to the file
			const std::string FilePathAnsi(TCHAR_TO_ANSI(*SingleAssetData.AssetPath.ToString()));
			const std::string SizeAnsi(TCHAR_TO_ANSI(*MakeBestSizeString(SingleAssetData.AssetSizeInfo.MemorySize, true)));

			FileHandle->Write((const uint8*)FilePathAnsi.c_str(), FilePathAnsi.length());
			FileHandle->Write((const uint8*)"=> ", 3);  
			FileHandle->Write((const uint8*)SizeAnsi.c_str(), SizeAnsi.length());
			FileHandle->Write((const uint8*)"\n", 1);  // Add a newline after each string
		}

		// Close the file handle
		delete FileHandle;
	}
	else
	{
		// Handle the case where the file couldn't be created
		UE_LOG(LogTemp, Error, TEXT("Failed to create file for exporting FString array: %s"), *FilePath);
	}
}

FString AssetInvestigatorUtility::MakeBestSizeString(const SIZE_T SizeInBytes, const bool bHasKnownSize)
{
	FText SizeText;

	if (SizeInBytes < 1000)
	{
		// We ended up with bytes, so show a decimal number
		SizeText = FText::AsMemory(SizeInBytes, EMemoryUnitStandard::SI);
	}
	else
	{
		// Show a fractional number with the best possible units
		FNumberFormattingOptions NumberFormattingOptions;
		NumberFormattingOptions.MaximumFractionalDigits = 1;
		NumberFormattingOptions.MinimumFractionalDigits = 0;
		NumberFormattingOptions.MinimumIntegralDigits = 1;

		SizeText = FText::AsMemory(SizeInBytes, &NumberFormattingOptions, nullptr, EMemoryUnitStandard::SI);
	}

	if (!bHasKnownSize)
	{
		if (SizeInBytes == 0)
		{
			SizeText = LOCTEXT("UnknownSize", "unknown size");
		}
		else
		{
			SizeText = FText::Format(LOCTEXT("UnknownSizeButAtLeastThisBigFmt", "at least {0}"), SizeText);
		}
	}

	return SizeText.ToString();
}
