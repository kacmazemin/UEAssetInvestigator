// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ImGuiDelegates.h"
#include "Misc/AssetRegistryInterface.h"

struct FAssetData;
struct FAssetInfo;
struct FAssetSizeInfo;
struct FAssetManagerEditorRegistrySource;
class FToolBarBuilder;
class FMenuBuilder;

static const FName AssetInvestigatorTabName("AssetInvestigator");

#define LOCTEXT_NAMESPACE "FAssetInvestigatorModule"

/**
 * The module contains functionalities for collecting information about assets, organizing them based on size,
 * and displaying relevant details in an ImGui-based user interface. It includes features such as collecting asset data,
 * clearing asset information, initializing a user interface for asset investigation, and handling UI interactions to open various UE4 Editor tools.
 */
class FAssetInvestigatorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	FImGuiDelegateHandle Delegate;

	bool bHasImGuiRegistered = false;
	void UnregisterImGuiTab();

	// Animate a simple progress bar
	float progress = 0.0f;
	float singleStep = 1.0f;

	int node_clicked = -1;
	bool animateProgressBar = false;

	TArray<FAssetInfo> CachedAssets;
	TSharedPtr<class FUICommandList> PluginCommands;

	/** The registry source to display information for */
	const FAssetManagerEditorRegistrySource* CurrentRegistrySource;

	TArray<UEdGraphNode*> FoundNodes;

	/*
	 * Initializes the User Interface for the Asset Investigator, including buttons and progress bar.
	 */
	void InitializeUI(bool* p_open = NULL);

	/*
	 * Gathers information about assets in the project, populating the CachedAssets array.
	 */
	void CollectAssets();

	/**
	 * Clears the CachedAssets array, resetting any gathered asset information.
	 */
	void ClearAssets();

	/**
	 * Creates utility buttons for the Asset Investigator UI, allowing users to perform specific actions.
	 */
	void CreateUtilityButtons();

	/**
	 * Recursively gathers size information for the specified asset and its dependencies, populating the provided AssetSizeInfo structure.
	 */
	void GatherSizeRecursively(const FAssetData& AssetData, FAssetSizeInfo& AssetSizeInfo, TArray<FName>& VisitedAssets);

	/**
	 * Gathers information about the specified asset, including size and dependencies, and adds it to the CachedAssets array.
	 */
	void GatherAssetInformation(const FAssetData& AssetData);


	void DisplayReferences(intptr_t nodeId, const FString& categoryName, UE::AssetRegistry::EDependencyQuery dependencyQuery);

	void DisplayCastToNodes();
};
