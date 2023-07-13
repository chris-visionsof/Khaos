#pragma once
#include "IDetailCustomization.h"

class FKhaosActorCustomization : public IDetailCustomization 
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
protected:
	virtual void OnTagCheckChanged(ECheckBoxState InState, FName CheckBoxName);

	TArray<TWeakObjectPtr<AActor>> SelectedActors;

private:
	static ECheckBoxState DetermineCheckBoxState(const TArray<bool>& BoolArray);
};
