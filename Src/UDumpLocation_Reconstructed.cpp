/*=============================================================================
	UDumpLocation_Reconstructed.cpp
	DeusEx.dll reconstruction pass 27.

	Original role:
	- Native noexport helper for developer-ish dump/load player locations.
	- Stores .DMP files containing fixed-size DumpLocationFileStruct records.

	Pass27 notes:
	- UDumpLocation is no longer deferred.
	- GetDumpFileLocationInfo() is intentionally empty, matching original ASM.
	  SelectDumpFileLocation() performs the actual native-to-script struct copy.
	- AddDumpFileLocation() intentionally keeps the original unsafe assumptions:
	  no currentDumpFileLocation null guard, no DeusExLevelInfo fallback, no fixed
	  string bounds checks beyond appStrcpy() behaviour.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"
#include <stdio.h>

static UBOOL bDxDumpLocationSaved = 0;
static FVector DxDumpSavedLocation(0.0f, 0.0f, 0.0f);
static FRotator DxDumpSavedViewRotation(0, 0, 0);

static const TCHAR* DxDumpEmptyText(void)
{
	return TEXT("");
}

static const TCHAR* DxDumpTextPtr(const FString& Value)
{
	if (Value.Len() > 0)
		return *Value;
	return DxDumpEmptyText();
}

static FILE* DxDumpOpenFile(const TCHAR* Filename, const TCHAR* Mode)
{
#if UNICODE
	if (GUnicodeOS)
		return _wfopen(Filename, Mode);
	return fopen(TCHAR_TO_ANSI(Filename), TCHAR_TO_ANSI(Mode));
#else
	return fopen(Filename, Mode);
#endif
}

static ADeusExLevelInfo* DxDumpFindLevelInfo(ADeusExPlayer* Player)
{
	ULevel* Level = Player->GetLevel();
	for (INT ActorIndex = 0; ActorIndex < Level->Actors.Num(); ActorIndex++)
	{
		AActor* Actor = Level->Actors(ActorIndex);
		if (Actor != NULL)
		{
			for (UClass* TestClass = Actor->GetClass(); TestClass != NULL; TestClass = Cast<UClass>(TestClass->SuperField))
			{
				if (TestClass == ADeusExLevelInfo::StaticClass())
					return (ADeusExLevelInfo*)Actor;
			}
		}
	}
	return NULL;
}

UDumpLocation::UDumpLocation()
: currentDumpFileLocation(NULL)
, currentDumpFileIndex(-1)
, currentDumpLocationIndex(-1)
, dumpFile(NULL)
, dumpLocationCount(0)
, player(NULL)
{
	guard(UDumpLocation::UDumpLocation);

	TCHAR UserName[64];
	UserName[0] = 0;
	GConfig->GetString(TEXT("DeusEx.DumpLocation"), TEXT("CurrentUser"), UserName, ARRAY_COUNT(UserName), TEXT("User.ini"));
	currentUser = UserName;

	currentDumpFileLocation = (DumpLocationFileStruct*)GMalloc->Malloc(sizeof(DumpLocationFileStruct), TEXT("new"));

	PopulateDumpDirectory();
	unguard;
}

void UDumpLocation::Destroy(void)
{
	guard(UDumpLocation::Destroy);

	CloseDumpFile();
	dumpFileDirectory.Empty();

	player = NULL;
	GMalloc->Free(currentDumpFileLocation);
	currentDumpFileLocation = NULL;

	Super::Destroy();
	unguard;
}

void UDumpLocation::PopulateDumpDirectory(void)
{
	guard(UDumpLocation::PopulateDumpDirectory);

	dumpFileDirectory.Empty();

	TCHAR Search[256];
	appSprintf(Search, TEXT("*.%s"), DUMPLOCATION_EXTENSION);
	dumpFileDirectory = GFileManager->FindFiles(Search, 1, 0);

	unguard;
}

FString UDumpLocation::GetFirstDumpFile(void)
{
	guard(UDumpLocation::GetFirstDumpFile);

	if (dumpFileDirectory.Num() <= 0)
		return FString(TEXT(""));

	currentDumpFileIndex = 0;
	return dumpFileDirectory(0);

	unguard;
}

FString UDumpLocation::GetNextDumpFile(void)
{
	guard(UDumpLocation::GetNextDumpFile);

	currentDumpFileIndex++;
	if (currentDumpFileIndex >= dumpFileDirectory.Num())
	{
		currentDumpFileIndex = -1;
		return FString(TEXT(""));
	}

	return dumpFileDirectory(currentDumpFileIndex);

	unguard;
}

INT UDumpLocation::GetDumpFileIndex(void)
{
	guard(UDumpLocation::GetDumpFileIndex);
	return currentDumpFileIndex;
	unguard;
}

INT UDumpLocation::GetDumpFileCount(void)
{
	guard(UDumpLocation::GetDumpFileCount);
	return dumpFileDirectory.Num();
	unguard;
}

bool UDumpLocation::OpenDumpFile(FString& Filename)
{
	guard(UDumpLocation::OpenDumpFile);

	CloseDumpFile();

	const TCHAR* NameText = DxDumpTextPtr(Filename);
	if (appStricmp(NameText, TEXT("")) == 0)
		return false;

	TCHAR FullFilename[256];
	appSprintf(FullFilename, TEXT("%s.%s"), NameText, DUMPLOCATION_EXTENSION);

	dumpFile = DxDumpOpenFile(FullFilename, TEXT("r+b"));
	if (dumpFile == NULL)
		return CreateDumpFile(Filename);

	TCHAR Header[64];
	BYTE Marker;

	fread(Header, appStrlen(DUMPLOCATION_HEADER), 1, dumpFile);
	fread(&Marker, 1, 1, dumpFile);
	fread(&dumpLocationCount, sizeof(INT), 1, dumpFile);

	currentDumpLocationIndex = -1;
	return true;

	unguard;
}

bool UDumpLocation::CreateDumpFile(FString& Filename)
{
	guard(UDumpLocation::CreateDumpFile);

	BYTE Marker = 0x1A;

	CloseDumpFile();

	const TCHAR* NameText = DxDumpTextPtr(Filename);
	if (appStricmp(NameText, TEXT("")) == 0)
		return false;

	TCHAR FullFilename[256];
	appSprintf(FullFilename, TEXT("%s.%s"), NameText, DUMPLOCATION_EXTENSION);

	dumpFile = DxDumpOpenFile(FullFilename, TEXT("w+b"));
	if (dumpFile == NULL)
		return false;

	currentDumpLocationIndex = -1;
	dumpLocationCount = 0;

	fwrite(DUMPLOCATION_HEADER, appStrlen(DUMPLOCATION_HEADER), 1, dumpFile);
	fwrite(&Marker, 1, 1, dumpFile);
	fwrite(&dumpLocationCount, sizeof(INT), 1, dumpFile);

	return true;

	unguard;
}

void UDumpLocation::CloseDumpFile(void)
{
	guard(UDumpLocation::CloseDumpFile);

	if (dumpFile != NULL)
	{
		fclose(dumpFile);
		dumpFile = NULL;
		currentDumpLocationIndex = -1;
		dumpLocationCount = 0;
	}

	unguard;
}

void UDumpLocation::DeleteDumpFile(FString& Filename)
{
	guard(UDumpLocation::DeleteDumpFile);

	TCHAR FullFilename[256];
	appSprintf(FullFilename, TEXT("%s.%s"), DxDumpTextPtr(Filename), DUMPLOCATION_EXTENSION);
	GFileManager->Delete(FullFilename, 0, 0);

	unguard;
}

bool UDumpLocation::GetFirstDumpFileLocation(void)
{
	guard(UDumpLocation::GetFirstDumpFileLocation);

	if (dumpFile == NULL)
		return false;

	currentDumpLocationIndex = -1;
	return GetNextDumpFileLocation();

	unguard;
}

bool UDumpLocation::GetNextDumpFileLocation(void)
{
	guard(UDumpLocation::GetNextDumpFileLocation);

	if (dumpFile == NULL)
		return false;

	if (dumpLocationCount <= 0)
	{
		currentDumpLocationIndex = -1;
		return false;
	}

	currentDumpLocationIndex++;
	if (currentDumpLocationIndex >= dumpLocationCount)
	{
		currentDumpLocationIndex = -1;
		return false;
	}

	while (true)
	{
		SelectDumpFileLocation(currentDumpLocationIndex);
		if (currentDumpFileLocation->bDeleted != true)
			return true;

		currentDumpLocationIndex++;
		if (currentDumpLocationIndex >= dumpLocationCount)
		{
			currentDumpLocationIndex = -1;
			return false;
		}
	}

	unguard;
}

INT UDumpLocation::GetDumpLocationIndex(void)
{
	guard(UDumpLocation::GetDumpLocationIndex);
	return currentDumpLocationIndex;
	unguard;
}

bool UDumpLocation::SelectDumpFileLocation(INT DumpLocationID)
{
	guard(UDumpLocation::SelectDumpFileLocation);

	if (dumpFile == NULL)
		return false;
	if (DumpLocationID == -1)
		return false;
	if (DumpLocationID >= dumpLocationCount)
		return false;

	SeekPastHeader();
	fseek(dumpFile, sizeof(INT), SEEK_CUR);
	fseek(dumpFile, sizeof(DumpLocationFileStruct) * DumpLocationID, SEEK_CUR);
	fread(currentDumpFileLocation, sizeof(DumpLocationFileStruct), 1, dumpFile);

	currentDumpLocation.bDeleted = currentDumpFileLocation->bDeleted;
	currentDumpLocation.LocationID = currentDumpFileLocation->LocationID;
	currentDumpLocation.MapName = currentDumpFileLocation->MapName;
	currentDumpLocation.Location = currentDumpFileLocation->Location;
	currentDumpLocation.ViewRotation = currentDumpFileLocation->ViewRotation;
	currentDumpLocation.GameVersion = currentDumpFileLocation->GameVersion;
	currentDumpLocation.Title = currentDumpFileLocation->Title;
	currentDumpLocation.Desc = currentDumpFileLocation->Desc;

	return true;

	unguard;
}

void UDumpLocation::SeekPastHeader(void)
{
	guard(UDumpLocation::SeekPastHeader);

	if (dumpFile != NULL)
		fseek(dumpFile, appStrlen(DUMPLOCATION_HEADER) + 1, SEEK_SET);

	unguard;
}

void UDumpLocation::GetDumpFileLocationInfo()
{
	guard(UDumpLocation::GetDumpFileLocationInfo);
	// Original function body is empty. SelectDumpFileLocation() copies the native
	// on-disk record into currentDumpLocation directly.
	unguard;
}

void UDumpLocation::DeleteDumpFileLocation(INT DumpLocationID)
{
	guard(UDumpLocation::DeleteDumpFileLocation);

	if (dumpFile != NULL)
	{
		if (SelectDumpFileLocation(DumpLocationID))
		{
			currentDumpFileLocation->bDeleted = true;
			SeekPastHeader();
			fseek(dumpFile, sizeof(INT), SEEK_CUR);
			fseek(dumpFile, sizeof(DumpLocationFileStruct) * DumpLocationID, SEEK_CUR);
			fwrite(currentDumpFileLocation, sizeof(DumpLocationFileStruct), 1, dumpFile);
			currentDumpLocationIndex = DumpLocationID;
		}
	}

	unguard;
}

void UDumpLocation::AddDumpFileLocation(FString& Filename, FString& NewTitle, FString& NewDescription)
{
	guard(UDumpLocation::AddDumpFileLocation);

	if (player == NULL)
		return;
	if (OpenDumpFile(Filename) == false)
		return;

	ADeusExLevelInfo* DeusInfo = DxDumpFindLevelInfo(player);

	currentDumpFileLocation->bDeleted = false;
	currentDumpFileLocation->LocationID = dumpLocationCount;
	appStrcpy(currentDumpFileLocation->MapName, DxDumpTextPtr(DeusInfo->MapName));
	currentDumpFileLocation->Location = player->Location;
	currentDumpFileLocation->ViewRotation = player->ViewRotation;
	appStrcpy(currentDumpFileLocation->GameVersion, player->GetDeusExVersion());
	appStrcpy(currentDumpFileLocation->Title, DxDumpTextPtr(NewTitle));
	appStrcpy(currentDumpFileLocation->Desc, DxDumpTextPtr(NewDescription));

	dumpLocationCount++;
	fseek(dumpFile, 0, SEEK_END);
	fwrite(currentDumpFileLocation, sizeof(DumpLocationFileStruct), 1, dumpFile);

	currentDumpLocationIndex = -1;
	SeekPastHeader();
	fwrite(&dumpLocationCount, sizeof(INT), 1, dumpFile);

	unguard;
}

INT UDumpLocation::GetNextDumpFileLocationID(void)
{
	guard(UDumpLocation::GetNextDumpFileLocationID);

	if (dumpFile != NULL)
		return dumpLocationCount;
	return -1;

	unguard;
}

FString UDumpLocation::GetCurrentUser(void)
{
	guard(UDumpLocation::GetCurrentUser);
	return currentUser;
	unguard;
}

void UDumpLocation::SetPlayer(ADeusExPlayer* NewPlayer)
{
	guard(UDumpLocation::SetPlayer);
	player = NewPlayer;
	unguard;
}

void UDumpLocation::SaveLocation(void)
{
	guard(UDumpLocation::SaveLocation);

	bDxDumpLocationSaved = 1;
	DxDumpSavedLocation = currentDumpLocation.Location;
	DxDumpSavedViewRotation = currentDumpLocation.ViewRotation;

	unguard;
}

void UDumpLocation::LoadLocation(void)
{
	guard(UDumpLocation::LoadLocation);

	if (bDxDumpLocationSaved)
	{
		bDxDumpLocationSaved = 0;
		currentDumpLocation.Location = DxDumpSavedLocation;
		currentDumpLocation.ViewRotation = DxDumpSavedViewRotation;
	}

	unguard;
}

UBOOL UDumpLocation::HasLocationBeenSaved(void)
{
	guard(UDumpLocation::HasLocationBeenSaved);
	return bDxDumpLocationSaved;
	unguard;
}

INT UDumpLocation::GetDumpFileLocationCount(FString& Filename)
{
	guard(UDumpLocation::GetDumpFileLocationCount);

	if (OpenDumpFile(Filename) == false)
		return -1;

	INT Count = dumpLocationCount;
	CloseDumpFile();
	return Count;

	unguard;
}

void UDumpLocation::execGetFirstDumpFile(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetFirstDumpFile);
	P_FINISH;
	*(FString*)Result = GetFirstDumpFile();
	unguardexec;
}

void UDumpLocation::execGetNextDumpFile(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetNextDumpFile);
	P_FINISH;
	*(FString*)Result = GetNextDumpFile();
	unguardexec;
}

void UDumpLocation::execGetDumpFileIndex(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetDumpFileIndex);
	P_FINISH;
	*(INT*)Result = GetDumpFileIndex();
	unguardexec;
}

void UDumpLocation::execGetDumpFileCount(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetDumpFileCount);
	P_FINISH;
	*(INT*)Result = GetDumpFileCount();
	unguardexec;
}

void UDumpLocation::execOpenDumpFile(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execOpenDumpFile);
	P_GET_STR(Filename);
	P_FINISH;
	*(UBOOL*)Result = OpenDumpFile(Filename);
	unguardexec;
}

void UDumpLocation::execCloseDumpFile(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execCloseDumpFile);
	P_FINISH;
	CloseDumpFile();
	unguardexec;
}

void UDumpLocation::execDeleteDumpFile(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execDeleteDumpFile);
	P_GET_STR(Filename);
	P_FINISH;
	DeleteDumpFile(Filename);
	unguardexec;
}

void UDumpLocation::execGetFirstDumpFileLocation(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetFirstDumpFileLocation);
	P_FINISH;
	*(UBOOL*)Result = GetFirstDumpFileLocation();
	unguardexec;
}

void UDumpLocation::execGetNextDumpFileLocation(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetNextDumpFileLocation);
	P_FINISH;
	*(UBOOL*)Result = GetNextDumpFileLocation();
	unguardexec;
}

void UDumpLocation::execGetDumpLocationIndex(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetDumpLocationIndex);
	P_FINISH;
	*(INT*)Result = GetDumpLocationIndex();
	unguardexec;
}

void UDumpLocation::execSelectDumpFileLocation(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execSelectDumpFileLocation);
	P_GET_INT(DumpLocationID);
	P_FINISH;
	*(UBOOL*)Result = SelectDumpFileLocation(DumpLocationID);
	unguardexec;
}

void UDumpLocation::execGetDumpFileLocationInfo(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetDumpFileLocationInfo);
	P_FINISH;
	GetDumpFileLocationInfo();
	unguardexec;
}

void UDumpLocation::execDeleteDumpFileLocation(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execDeleteDumpFileLocation);
	P_GET_INT(DumpLocationID);
	P_FINISH;
	DeleteDumpFileLocation(DumpLocationID);
	unguardexec;
}

void UDumpLocation::execAddDumpFileLocation(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execAddDumpFileLocation);
	P_GET_STR(Filename);
	P_GET_STR(NewTitle);
	P_GET_STR(NewDescription);
	P_FINISH;
	AddDumpFileLocation(Filename, NewTitle, NewDescription);
	unguardexec;
}

void UDumpLocation::execGetNextDumpFileLocationID(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetNextDumpFileLocationID);
	P_FINISH;
	*(INT*)Result = GetNextDumpFileLocationID();
	unguardexec;
}

void UDumpLocation::execGetCurrentUser(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetCurrentUser);
	P_FINISH;
	*(FString*)Result = GetCurrentUser();
	unguardexec;
}

void UDumpLocation::execSetPlayer(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execSetPlayer);
	P_GET_OBJECT(ADeusExPlayer, NewPlayer);
	P_FINISH;
	SetPlayer(NewPlayer);
	unguardexec;
}

void UDumpLocation::execSaveLocation(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execSaveLocation);
	P_FINISH;
	SaveLocation();
	unguardexec;
}

void UDumpLocation::execLoadLocation(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execLoadLocation);
	P_FINISH;
	LoadLocation();
	unguardexec;
}

void UDumpLocation::execHasLocationBeenSaved(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execHasLocationBeenSaved);
	P_FINISH;
	*(UBOOL*)Result = HasLocationBeenSaved();
	unguardexec;
}

void UDumpLocation::execGetDumpFileLocationCount(FFrame& Stack, RESULT_DECL)
{
	guard(UDumpLocation::execGetDumpFileLocationCount);
	P_GET_STR(Filename);
	P_FINISH;
	*(INT*)Result = GetDumpFileLocationCount(Filename);
	unguardexec;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
