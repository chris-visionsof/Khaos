#pragma once

using FKhaosShowFlag = TCustomShowFlag<EShowFlagShippingValue::ForceDisabled>;

struct FKhaosShowFlags
{
	inline static FKhaosShowFlag Interactivity = FKhaosShowFlag(TEXT("Interactivity"), false, SFG_Normal);
};
