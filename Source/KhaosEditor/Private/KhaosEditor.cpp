#include "KhaosEditor.h"

#include "ActorCustomizations/KhaosActorCustomization.h"
#include "Khaos/Public/Actors/KhaosActor.h"

#define LOCTEXT_NAMESPACE "FKhaosEditorModule"

void FKhaosEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(AKhaosActor::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FKhaosActorCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FKhaosEditorModule::ShutdownModule()
{
	if(FModuleManager::Get().IsModuleLoaded(KHAOS_EDITOR_MODULE_NAME))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(AKhaosActor::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FKhaosEditorModule, KhaosEditor)