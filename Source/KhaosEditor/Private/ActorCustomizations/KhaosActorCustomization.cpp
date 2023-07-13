#include "KhaosActorCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Khaos/Public/KhaosDefinitions.h"
#include "Khaos/Public/Actors/KhaosActionableActor.h"

#define LOCTEXT_NAMESPACE "FKhaosEditorModule"

TSharedRef<IDetailCustomization> FKhaosActorCustomization::MakeInstance()
{
	return MakeShareable(new FKhaosActorCustomization());
}

void FKhaosActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& KhaosTagsCategory = DetailBuilder.EditCategory("KhaosTags");

	TArray<TWeakObjectPtr<UObject>> SelectedObjects = DetailBuilder.GetSelectedObjects();

	SelectedActors.Reserve(SelectedObjects.Num());

	TMap<FName, TArray<bool>> TagsState =
		{
			{ ActorTags::Actionable, TArray<bool>() },
			{ ActorTags::Grabbable, TArray<bool>() }
		};

	bool bIsActionable = false;

	for (TWeakObjectPtr<UObject> SelectedObject : SelectedObjects)
	{
		AActor* SelectedActor = Cast<AActor>(SelectedObject);

		TagsState[ActorTags::Actionable].AddUnique(SelectedActor->Tags.Contains(ActorTags::Actionable));
		TagsState[ActorTags::Grabbable].AddUnique(SelectedActor->Tags.Contains(ActorTags::Grabbable));

		if(SelectedActor->IsA(AKhaosActionableActor::StaticClass()))
		{
			bIsActionable = true;
		}

		SelectedActors.Add(SelectedActor);
	}

#define LABEL_BUILDER(tag, always_viz)																	\
	+SVerticalBox::Slot()																				\
		.AutoHeight()																					\
		.Padding(5.0f)																					\
		[																								\
			SNew(STextBlock)																			\
				.Text(FText::FromName(ActorTags::##tag))												\
				.Visibility((always_viz || bIsActionable) ? EVisibility::Visible : EVisibility::Hidden)	\
		]																								\

#define CHECKBOX_BUILDER(tag, always_viz)																\
	+SVerticalBox::Slot()																				\
		.AutoHeight()																					\
		.Padding(5.0f)																					\
	[																									\
		SNew(SCheckBox)																					\
			.IsChecked(DetermineCheckBoxState(TagsState[ActorTags::##tag]))								\
			.Visibility((always_viz || bIsActionable) ? EVisibility::Visible : EVisibility::Hidden)		\
			.OnCheckStateChanged(this, &FKhaosActorCustomization::OnTagCheckChanged, ActorTags::##tag)	\
	]																									\

	KhaosTagsCategory.AddCustomRow(LOCTEXT("KhaosTags", "Khaos Tags"))
		.NameContent()
		[
			SNew(SVerticalBox)
				LABEL_BUILDER(Actionable, false)
				LABEL_BUILDER(Grabbable, true)
		]
		.ValueContent()
		[
			SNew(SVerticalBox)
				CHECKBOX_BUILDER(Actionable, false)
				CHECKBOX_BUILDER(Grabbable, true)
		];
#undef LABEL_BUILDER 
#undef CHECKBOX_BUILDER
}

void FKhaosActorCustomization::OnTagCheckChanged(ECheckBoxState InState, FName CheckBoxName)
{
	if (InState == ECheckBoxState::Undetermined) { return; }

	for (const TWeakObjectPtr<AActor> SelectedActor : SelectedActors)
	{
		if(CheckBoxName == ActorTags::Actionable && !SelectedActor->IsA(AKhaosActionableActor::StaticClass()))
		{
			continue;
		}

		if(InState == ECheckBoxState::Checked)
		{
			SelectedActor->Tags.AddUnique(CheckBoxName);
		}
		else 
		{
			SelectedActor->Tags.Remove(CheckBoxName);
		}
	}
}

ECheckBoxState FKhaosActorCustomization::DetermineCheckBoxState(const TArray<bool>& BoolArray)
{
	if(BoolArray.Num() > 1) { return ECheckBoxState::Undetermined; }

	return BoolArray[0] ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

#undef LOCTEXT_NAMESPACE
