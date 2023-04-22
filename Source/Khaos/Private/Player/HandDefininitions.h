#pragma once

UENUM()
enum EFingers : int 
{
	Thumb = 0,
	Index = 1,
	Middle = 2,
	Ring = 3,
	Little = 4
};

const TMap<int, FString> FINGER_NAMES = {
	{ EFingers::Thumb, "Thumb" },
	{ EFingers::Index, "Index" },
	{ EFingers::Middle, "Middle" },
	{ EFingers::Ring, "Ring" },
	{ EFingers::Little, "Little" }
};
