// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * A Container class that include info about the asset such as MemorySize, HardReferences...
 */

struct ASSETINVESTIGATOR_API FAssetSizeInfo
{
	int64 MemorySize = 0;
	int64 DiskSize = 0;
};

struct ASSETINVESTIGATOR_API FAssetInfo
{
	FName AssetPath;
	FName PackagePath;
	FAssetSizeInfo AssetSizeInfo;

	TArray<FName> HardReferences;
	TArray<FName> SoftReferences;
	TArray<FName> HardCastToNodes;
};
