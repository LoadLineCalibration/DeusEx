/*=============================================================================
	UDeusExSaveInfo_Reconstructed_Pass09.cpp
	DeusEx.dll reconstruction pass 09.

	Small class, but important for save menu metadata.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

UDeusExSaveInfo::UDeusExSaveInfo()
{
	guard(UDeusExSaveInfo::UDeusExSaveInfo);
	DirectoryIndex = 0;
	unguard;
}

void UDeusExSaveInfo::Destroy(void)
{
	guard(UDeusExSaveInfo::Destroy);
	Super::Destroy();
	unguard;
}

void UDeusExSaveInfo::UpdateTimeStamp(void)
{
	guard(UDeusExSaveInfo::UpdateTimeStamp);

	INT DayOfWeek = 0;
	INT Milliseconds = 0;
	appSystemTime(Year, Month, DayOfWeek, Day, Hour, Minute, Second, Milliseconds);

	unguard;
}

UTexture* UDeusExSaveInfo::CreateTexture(void)
{
	guard(UDeusExSaveInfo::CreateTexture);

	UTexture* Texture = new(GetOuter(), NAME_None) UTexture();

	// ASM note: after UTexture::UTexture(), DeusEx.dll writes a local UTexture
	// vtable whose only observed difference is the destructor thunk for this DLL.
	// There is no portable source-level spelling for that generated VC98 vtable,
	// so the reconstruction keeps ordinary UTexture construction and records the
	// binary-only detail in Reports/Pass19_Findings.md.
	return Texture;

	unguard;
}

void UDeusExSaveInfo::execUpdateTimeStamp(FFrame& Stack, RESULT_DECL)
{
	guard(UDeusExSaveInfo::execUpdateTimeStamp);
	P_FINISH;
	UpdateTimeStamp();
	unguardexec;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
