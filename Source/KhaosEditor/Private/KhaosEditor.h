#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define KHAOS_EDITOR_MODULE_NAME "KhaosEditor"

class FKhaosEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
