// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AssetInvestigatorStyle.h"

class FAssetInvestigatorCommands : public TCommands<FAssetInvestigatorCommands>
{
public:

	FAssetInvestigatorCommands()
		: TCommands<FAssetInvestigatorCommands>(TEXT("AssetInvestigator"), NSLOCTEXT("Contexts", "AssetInvestigator", "AssetInvestigator Plugin"), NAME_None, FAssetInvestigatorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};