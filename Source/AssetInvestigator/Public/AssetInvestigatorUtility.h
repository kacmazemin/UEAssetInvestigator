#pragma once

#include "CoreMinimal.h"

struct FAssetInfo;

/**
 * Utility class for asset investigation in Unreal Engine 4.
 */
class ASSETINVESTIGATOR_API AssetInvestigatorUtility
{
public:

	/**
	 * Opens the selected asset identified by its path.
	 *
	 * @param AssetPath The path of the asset to be opened.
	 */
	static void OpenSelectedAsset(const FName& AssetPath);

	/**
	 * Browses to the specified asset identified by its path.
	 *
	 * @param AssetPath The path of the asset to navigate to.
	 */
	static void BrowseToAsset(const FName& AssetPath);

	/**
	 * Exports a list of asset information to a text file.
	 *
	 * @param AssetInfoList The list of asset information to export.
	 */
	static void ExportListToTxt(const TArray<FAssetInfo>& AssetInfoList);

	/**
	 * Creates a formatted string representation of a size value.
	 *
	 * @param SizeInBytes   The size value in bytes.
	 * @param bHasKnownSize Indicates whether the size is known.
	 * @return              A formatted string representing the size.
	 */
	static FString MakeBestSizeString(const SIZE_T SizeInBytes, const bool bHasKnownSize);

	static TArray<UEdGraphNode*> GetDynamicCastToNodes(const FName& AssetPath);
};