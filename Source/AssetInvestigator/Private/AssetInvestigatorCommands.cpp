// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetInvestigatorCommands.h"

#define LOCTEXT_NAMESPACE "FAssetInvestigatorModule"

void FAssetInvestigatorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "AssetInvestigator", "Bring up AssetInvestigator window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
