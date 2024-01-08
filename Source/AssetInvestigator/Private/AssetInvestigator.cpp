// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetInvestigator.h"
#include "AssetData.h"
#include "AssetInfo.h"
#include "AssetInvestigatorUtility.h"

#include "AssetInvestigatorStyle.h"
#include "AssetInvestigatorCommands.h"
#include "LevelEditor.h"
#include "ToolMenus.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "AssetManagerEditorModule.h"

#include "ImGuiModule.h"
#include "ImGuiDelegates.h"
#include "ImGuiWidgetEd.h"

void FAssetInvestigatorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FAssetInvestigatorStyle::Initialize();
	FAssetInvestigatorStyle::ReloadTextures();

	FAssetInvestigatorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FAssetInvestigatorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FAssetInvestigatorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetInvestigatorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AssetInvestigatorTabName, FOnSpawnTab::CreateRaw(this, &FAssetInvestigatorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FAssetInvestigatorTabTitle", "AssetInvestigator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	IAssetManagerEditorModule& ManagerEditorModule = IAssetManagerEditorModule::Get();

	CurrentRegistrySource = ManagerEditorModule.GetCurrentRegistrySource();
}

void FAssetInvestigatorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAssetInvestigatorStyle::Shutdown();

	FAssetInvestigatorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AssetInvestigatorTabName);
}

TSharedRef<SDockTab> FAssetInvestigatorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// the unique index to create context for different editor windows
	int contextIdx = 1;

	if (bHasImGuiRegistered)
	{
		UnregisterImGuiTab();
		bHasImGuiRegistered = false;
	}

	bHasImGuiRegistered = true;

	// OnGUI function, contains imgui draw instructions
	TDelegate<void()> OnGUI = FImGuiDelegate::CreateLambda([=]()
		{
			//ImGui::ShowDemoWindow();
			InitializeUI();
		});

	// bind the OnGUI function to the Editor Window Context
	Delegate = FImGuiModule::Get().AddEditorWindowImGuiDelegate(OnGUI, contextIdx);

	// create the Slate Widget, the canvas for imgui
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SImGuiWidgetEd)
				.ContextIndex(contextIdx)
		];
}

void FAssetInvestigatorModule::UnregisterImGuiTab()
{
	if (Delegate.IsValid()) {
		FImGuiModule::Get().RemoveImGuiDelegate(Delegate);
	}
}

void FAssetInvestigatorModule::CollectAssets()
{
	CachedAssets.Empty();
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	Filter.PackagePaths.Add("/Game");

	progress = 0.f;
	animateProgressBar = true;

	TArray<FAssetData> FoundAssets;
	FAssetRegistryModule::GetRegistry().GetAssets(Filter, FoundAssets);

	singleStep = 1.f / FoundAssets.Num();
	animateProgressBar = true;
	for (const FAssetData& Asset : FoundAssets)
	{
		GatherAssetInformation(Asset);

		progress += singleStep;
	}

	CachedAssets.Sort([](const FAssetInfo& Info1, const FAssetInfo& Info2) {
		return  Info1.AssetSizeInfo.MemorySize > Info2.AssetSizeInfo.MemorySize;
		});

	animateProgressBar = false;
}

void FAssetInvestigatorModule::ClearAssets()
{
	CachedAssets.Empty();
	node_clicked = -1;
}

void FAssetInvestigatorModule::CreateUtilityButtons()
{
	if (ImGui::SmallButton("OpenSizeMapUI"))
	{
		IAssetManagerEditorModule::Get().OpenSizeMapUI({ FAssetIdentifier(CachedAssets[node_clicked].PackagePath) });
	}
	ImGui::SameLine();

	if (ImGui::SmallButton("OpenReferenceViewer"))
	{
		IAssetManagerEditorModule::Get().OpenReferenceViewerUI({ FAssetIdentifier(CachedAssets[node_clicked].PackagePath) });
	}
	ImGui::SameLine();

	if (ImGui::SmallButton("OpenAsset"))
	{
		AssetInvestigatorUtility::OpenSelectedAsset(CachedAssets[node_clicked].AssetPath);
	}
	ImGui::SameLine();

	if (ImGui::SmallButton("BrowseAsset"))
	{
		AssetInvestigatorUtility::BrowseToAsset(CachedAssets[node_clicked].AssetPath);
	}
}

void FAssetInvestigatorModule::InitializeUI(bool* p_open)
{
	ImGui::Begin("Asset Investigator");

	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing imgui context!"); 

	if (ImGui::Button("CollectAssets"))
	{
		CollectAssets();
	}

	if (ImGui::Button("Clear"))
	{
		ClearAssets();
		progress = 0.f;
	}

	ImGui::SameLine();

	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));

	if (animateProgressBar)
	{
		progress += singleStep * ImGui::GetIO().DeltaTime * 0.1f;
	}

	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Progress ");

	if (ImGui::Button("ExportCurrentListToText"))
	{
		AssetInvestigatorUtility::ExportListToTxt(CachedAssets);
	}
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("The text file will be exported to ProjectDir() ");

	static ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove;
	ImGui::BeginChild("Details", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, ImGui::GetWindowHeight()), false, WindowFlags);
	static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	static int selection_mask = (1 << 2); 

	for (int i = 0; i < CachedAssets.Num(); i++)
	{
		// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
		ImGuiTreeNodeFlags node_flags = base_flags;
		const bool is_selected = (selection_mask & (1 << i)) != 0;
		if (is_selected)
		{
			node_flags |= ImGuiTreeNodeFlags_Selected;
		}

		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "%s, %s", TCHAR_TO_ANSI(*CachedAssets[i].AssetPath.ToString()), TCHAR_TO_ANSI(*AssetInvestigatorUtility::MakeBestSizeString(CachedAssets[i].AssetSizeInfo.MemorySize, true)));

		if (ImGui::IsItemClicked())
		{
			node_clicked = i;
		}
	}

	if (node_clicked != -1)
	{
		// Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
		if (ImGui::GetIO().KeyCtrl)
		{
			selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
		}
		else
		{
			selection_mask = (1 << node_clicked);           // Click to single-select
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("Child2", ImVec2(0, ImGui::GetWindowHeight()), true, ImGuiWindowFlags_NoScrollWithMouse);

	if (node_clicked != -1)
	{
		ImGui::Text("DiskSize %s", TCHAR_TO_ANSI(*AssetInvestigatorUtility::MakeBestSizeString(CachedAssets[node_clicked].AssetSizeInfo.DiskSize, true)));
		ImGui::Text("MemorySize %s", TCHAR_TO_ANSI(*AssetInvestigatorUtility::MakeBestSizeString(CachedAssets[node_clicked].AssetSizeInfo.MemorySize, true)));

		CreateUtilityButtons();

		//todo: create a generic function
		if (ImGui::TreeNode((void*)(intptr_t)2, "Hard References"))
		{
			TArray<FName> HardReferences;
			IAssetRegistry::Get()->GetDependencies(CachedAssets[node_clicked].PackagePath, HardReferences, UE::AssetRegistry::EDependencyCategory::All, UE::AssetRegistry::EDependencyQuery::Hard);

			for (int n = 0; n < HardReferences.Num(); n++)
			{
				char label[256];
				sprintf(label, "%s", TCHAR_TO_ANSI(*HardReferences[n].ToString()));
				if (ImGui::Selectable(label)) {}
				ImGui::NextColumn();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode((void*)(intptr_t)3, "Soft References"))
		{
			TArray<FName> SoftReferences;
			IAssetRegistry::Get()->GetDependencies(CachedAssets[node_clicked].PackagePath, SoftReferences, UE::AssetRegistry::EDependencyCategory::All, UE::AssetRegistry::EDependencyQuery::Soft);

			for (int n = 0; n < SoftReferences.Num(); n++)
			{
				char label[256];
				sprintf(label, "%s", TCHAR_TO_ANSI(*SoftReferences[n].ToString()));
				if (ImGui::Selectable(label)) {}
				ImGui::NextColumn();
			}

			ImGui::TreePop();
		}
	}

	ImGui::EndChild();

	ImGui::End();
}

void FAssetInvestigatorModule::GatherSizeRecursively(const FAssetData& AssetData, FAssetSizeInfo& AssetSizeInfo, TArray<FName>& VisitedAssets)
{
	TArray<FName> HardReferences;

	IAssetRegistry::Get()->GetDependencies(AssetData.PackageName, HardReferences, UE::AssetRegistry::EDependencyCategory::All, UE::AssetRegistry::EDependencyQuery::Hard);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//add self size
	{
		if (!VisitedAssets.Contains(AssetData.PackageName))
		{
			UObject* Asset = AssetData.GetAsset();

			if (Asset)
			{
				AssetSizeInfo.MemorySize += Asset->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
			}

			if (!VisitedAssets.Contains(AssetData.PackageName))
			{
				const FAssetPackageData* FoundData = CurrentRegistrySource->RegistryState->GetAssetPackageData(AssetData.PackageName);
				if (FoundData)
				{
					AssetSizeInfo.DiskSize += FoundData->DiskSize;
				}
			}

			VisitedAssets.Add(AssetData.PackageName);
		}
	}

	for (const FName& AssetName : HardReferences)
	{
		const FAssetData& AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(AssetName);
		if (AssetRegistryModule.Get().GetAssetByObjectPath(AssetName).IsValid())
		{

			if (VisitedAssets.Contains(AssetData.PackageName)) {
				continue;
			}

			// Resource size can currently only be calculated for loaded assets, so load and check
			UObject* Asset = AssetData.GetAsset();

			if (Asset)
			{
				AssetSizeInfo.MemorySize += Asset->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
			}

			const FAssetPackageData* FoundData = CurrentRegistrySource->RegistryState->GetAssetPackageData(AssetData.PackageName);
			AssetSizeInfo.DiskSize += FoundData->DiskSize;

			VisitedAssets.Add(AssetData.PackageName);

			GatherSizeRecursively(AssetData, AssetSizeInfo, VisitedAssets);
		}
	}
}

void FAssetInvestigatorModule::GatherAssetInformation(const FAssetData& AssetData)
{
	FAssetInfo AssetInfo;
	AssetInfo.AssetPath = AssetData.ObjectPath;
	AssetInfo.PackagePath = AssetData.PackageName;

	TArray<FName> VisitedAssets;

	GatherSizeRecursively(AssetData, AssetInfo.AssetSizeInfo, VisitedAssets);

	CachedAssets.Add(AssetInfo);
}

void FAssetInvestigatorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AssetInvestigatorTabName);
}

void FAssetInvestigatorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FAssetInvestigatorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAssetInvestigatorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAssetInvestigatorModule, AssetInvestigator)