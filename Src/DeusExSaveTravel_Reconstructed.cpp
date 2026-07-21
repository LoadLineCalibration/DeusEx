/*=============================================================================
	DeusExSaveTravel_Reconstructed_Reconstructed.cpp
	Reconstruction pass for DeusEx.dll DDeusExGameEngine save/load/travel core.

	Pass17 changes:
	- Tightens GetDeusExLevelInfo() against the decompiled original: it scans
	  loaded objects for ADeusExLevelInfo instead of walking GLevel->Actors.
	  This is intentionally broader than a level actor list and matches the
	  original TObjectIterator-like pattern.

	Pass16 changes:
	- Tightens SaveCurrentLevel() against ASM: no GLevel NULL pre-guard; the
	  DeusExLevelInfo-missing path logs the original warning and exits without
	  EndSlowTask(), LevelAction reset, BrushTracker rebuild, or cache flush.
	  This preserves the original bug/edge behaviour instead of a safer rewrite.

	Pass21 changes:
	- Tightens first-viewport actor acquisition against ASM: after Client != NULL
	  and Viewports.Num() > 0, the original dereferences Viewports(0) directly.
	  The reconstruction-side Viewports(0) NULL guard is removed.

	Pass15 changes:
	- Keeps Pass14 SaveGame() strict player/snapshot behavior.
	- Tightens PruneTravelActors() against ASM: the first viewport actor is used
	  directly as ADeusExPlayer, with no Cast<> type guard and no global GLevel
	  pre-guard.
	- Tightens bStartingNewGame helpers used by Browse(): direct bitfield access
	  through ADeusExPlayer*, matching the ASM offsets.
=============================================================================

	Pass18 packaging note:
	- This is the canonical full-content source snapshot for moving to the next
	  dialogue. Previous pass suffixes were removed from filenames only; runtime
	  code remains based on the latest strict passes.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

static const TCHAR* DxTextOrEmpty(const FString& Value)
{
	if (Value.Len() > 0)
		return *Value;
	return TEXT("");
}

static const TCHAR* DxSaveExtOrEmpty(void)
{
	if (FURL::DefaultSaveExt.Len() > 0)
		return *FURL::DefaultSaveExt;
	return TEXT("");
}

static FString DxSaveSlotName(INT DirectoryIndex)
{
	if (DirectoryIndex == -2)
		return FString(SAVE_CurrentDirectory);
	if (DirectoryIndex == -1)
		return FString(SAVE_QuickSaveDirectory);
	return FString::Printf(TEXT("%s%04d"), SAVE_DirPrefix, DirectoryIndex);
}

static FString DxSaveSlotPath(INT DirectoryIndex)
{
	FString SlotName = DxSaveSlotName(DirectoryIndex);
	return FString::Printf(TEXT("%s\\%s"), DxTextOrEmpty(GSys->SavePath), *SlotName);
}

static FString DxMapSavePath(const FString& DirectoryName, const FString& MapName)
{
	return FString::Printf(
		TEXT("%s\\%s\\%s.%s"),
		DxTextOrEmpty(GSys->SavePath),
		DxTextOrEmpty(DirectoryName),
		DxTextOrEmpty(MapName),
		DxSaveExtOrEmpty());
}

static APlayerPawn* DxFirstViewportActor(UClient* Client)
{
	if (Client == NULL)
		return NULL;
	if (Client->Viewports.Num() <= 0)
		return NULL;

	// Original SaveGame(), PruneTravelActors() and Browse() only check the
	// viewport array count.  They dereference Viewports(0) directly and then
	// test/use its Actor.  Keep that stricter behaviour instead of adding a
	// reconstruction-side viewport-object NULL guard.
	return Client->Viewports(0)->Actor;
}

static UBOOL DxPlayerBlocksSameMissionTravel(APlayerPawn* Player)
{
	if (Player == NULL)
		return FALSE;

	// ASM tests byte ptr [Player+0xAF8], 1. The original does not perform
	// a dynamic Cast<> here after taking the first viewport actor.
	ADeusExPlayer* DeusExPlayer = (ADeusExPlayer*)Player;
	if (DeusExPlayer->bStartingNewGame != 0)
		return TRUE;
	return FALSE;
}

static void DxClearPlayerStartingNewGame(APlayerPawn* Player)
{
	if (Player == NULL)
		return;

	// ASM clears the low bit at Player+0xAF8 directly.
	ADeusExPlayer* DeusExPlayer = (ADeusExPlayer*)Player;
	DeusExPlayer->bStartingNewGame = 0;
}

DDeusExGameEngine::DDeusExGameEngine()
{
	guard(DDeusExGameEngine::DDeusExGameEngine);
	unguard;
}

void DDeusExGameEngine::Init(void)
{
	guard(DDeusExGameEngine::Init);
	DeusExInitNames();
	Super::Init();
	unguard;
}

UBOOL DDeusExGameEngine::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	guard(DDeusExGameEngine::Exec);

	const TCHAR* Str = Cmd;
	if (ParseCommand(&Str, TEXT("SAVEGAME")) == TRUE)
	{
		if (*Str >= TEXT('0') && *Str <= TEXT('9'))
		{
			SaveGame(appAtoi(Str), TEXT(""));
		}
		return TRUE;
	}

	Str = Cmd;
	if (ParseCommand(&Str, TEXT("DELETEGAME")) == TRUE)
	{
		if (*Str >= TEXT('0') && *Str <= TEXT('9'))
		{
			DeleteGame(appAtoi(Str));
		}
		return TRUE;
	}

	return Super::Exec(Cmd, Ar);
	unguard;
}

void DDeusExGameEngine::SaveGame(INT Position, FString SaveDesc)
{
	guard(DDeusExGameEngine::SaveGame);

	APlayerPawn* PlayerPawn = DxFirstViewportActor(Client);
	if (PlayerPawn == NULL)
		return;

	ADeusExPlayer* DeusExPlayer = (ADeusExPlayer*)PlayerPawn;

	ADeusExLevelInfo* DeusInfo = GetDeusExLevelInfo();
	if (DeusInfo == NULL)
	{
		GLog->Logf(TEXT("WARNING!  DeusExLevelInfo MISSING!"));
		return;
	}

	UPackage* SaveInfoPackage = UObject::CreatePackage(NULL, SAVE_SaveInfoFilename);
	if (SaveInfoPackage == NULL)
	{
		GLog->Logf(TEXT("WARNING!  Unable to create SaveInfoPackage!"));
		return;
	}

	UDeusExSaveInfo* SaveInfo = GetSaveInfo(SaveInfoPackage);
	if (SaveInfo == NULL)
	{
		GLog->Logf(TEXT("WARNING!  Unable to create DeusExSaveInfo Object!"));
		return;
	}

	SaveInfo->MapName = DeusInfo->MapName;
	if (appStricmp(*SaveDesc, TEXT("")) != 0)
	{
		SaveInfo->Description = SaveDesc;
	}
	else
	{
		ALevelInfo* LevelInfo = CastChecked<ALevelInfo>(GLevel->Actors(0));
		SaveInfo->Description = LevelInfo->Title;
	}

	SaveInfo->DirectoryIndex = Position;

	DeusExPlayer->saveCount++;
	SaveInfo->saveCount = DeusExPlayer->saveCount;
	SaveInfo->saveTime = (INT)DeusExPlayer->saveTime;

	// Original SaveGame writes bCheatsEnabled after UpdateTimeStamp().
	// The value is the same, but keep the store order ASM-aligned.
	SaveInfo->UpdateTimeStamp();
	SaveInfo->bCheatsEnabled = DeusExPlayer->bCheatsEnabled;

	FString GameRenderDeviceName;
	GConfig->GetString(TEXT("Engine.Engine"), TEXT("GameRenderDevice"), GameRenderDeviceName);
	if (appStricmp(*GameRenderDeviceName, TEXT("OpenGLDrv.OpenGLRenderDevice")) != 0)
	{
		SaveInfo->Snapshot = SaveInfo->CreateTexture();
		if (SaveInfo->Snapshot != NULL)
		{
			// Original ASM directly dereferences Player->rootWindow after
			// CreateTexture() succeeds.  No Cast<> check and no rootWindow NULL
			// guard are present in the DLL.
			DeusExPlayer->rootWindow->SetSnapshotSize(160.0f, 120.0f);
			DeusExPlayer->rootWindow->GenerateSnapshot(SaveInfo->Snapshot, TRUE, FALSE, FALSE);
		}
	}
	else
	{
		SaveInfo->Snapshot = NULL;
	}

	SaveInfo->MissionLocation = DeusInfo->MissionLocation;

	if (SaveInfo->DirectoryIndex == 0)
	{
		XGameDirectory* Directory = new(UObject::GetTransientPackage(), NAME_None) XGameDirectory();

		// Original ASM calls both methods unconditionally after the allocation / constructor expression.  It only tests the pointer before invoking the
		// destructor.  Do not add a reconstruction-side safety guard here; a failed
		// allocation would behave like the original and crash on the method call.
		Directory->GetSaveGamesDirectory();
		SaveInfo->DirectoryIndex = Directory->GetNewSaveFileIndex();

		if (Directory != NULL)
			delete Directory;
	}

	FString DestinationDirectory;
	if (Position == -1)
		DestinationDirectory = FString::Printf(TEXT("%s\\%s"), DxTextOrEmpty(GSys->SavePath), SAVE_QuickSaveDirectory);
	else
		DestinationDirectory = FString::Printf(TEXT("%s\\%s%04d"), DxTextOrEmpty(GSys->SavePath), SAVE_DirPrefix, SaveInfo->DirectoryIndex);

	GFileManager->MakeDirectory(*DestinationDirectory, TRUE);

	FString CurrentDirectory = FString::Printf(TEXT("%s\\%s"), DxTextOrEmpty(GSys->SavePath), SAVE_CurrentDirectory);

	FString ActualDestinationDirectory;
	if (Position == -1)
		ActualDestinationDirectory = FString::Printf(TEXT("%s\\%s"), DxTextOrEmpty(GSys->SavePath), SAVE_QuickSaveDirectory);
	else
		ActualDestinationDirectory = FString::Printf(TEXT("%s\\%s%04d"), *GSys->SavePath, SAVE_DirPrefix, SaveInfo->DirectoryIndex);

	DeleteSaveGameFiles(ActualDestinationDirectory);
	CopySaveGameFiles(CurrentDirectory, ActualDestinationDirectory);

	FString SaveInfoFilename;
	if (Position == -1)
	{
		SaveInfoFilename = FString::Printf(
			TEXT("%s\\%s\\%s.%s"),
			*GSys->SavePath,
			SAVE_QuickSaveDirectory,
			SAVE_SaveInfoFilename,
			DxSaveExtOrEmpty());
	}
	else
	{
		SaveInfoFilename = FString::Printf(
			TEXT("%s\\%s%04d\\%s.%s"),
			*GSys->SavePath,
			SAVE_DirPrefix,
			SaveInfo->DirectoryIndex,
			SAVE_SaveInfoFilename,
			DxSaveExtOrEmpty());
	}

	UObject::SavePackage(SaveInfoPackage, SaveInfo, 0, *SaveInfoFilename, GWarn, NULL);
	SaveCurrentLevel(SaveInfo->DirectoryIndex, FALSE);

	unguard;
}

void DDeusExGameEngine::PruneTravelActors(void)
{
	guard(DDeusExGameEngine::PruneTravelActors);

	APlayerPawn* Player = DxFirstViewportActor(Client);
	if (Player == NULL)
		return;

	// Original ASM uses the first viewport actor directly. There is no Cast<>
	// guard here; the offsets resolve to ADeusExPlayer generated fields.
	ADeusExPlayer* DeusExPlayer = (ADeusExPlayer*)Player;

	if (DeusExPlayer->AugmentationSystem != NULL)
	{
		AAugmentation* Augmentation = DeusExPlayer->AugmentationSystem->FirstAug;
		while (Augmentation != NULL)
		{
			AAugmentation* NextAugmentation = Augmentation->Next;
			GLevel->DestroyActor(Augmentation, FALSE);
			Augmentation = NextAugmentation;
		}

		GLevel->DestroyActor(DeusExPlayer->AugmentationSystem, FALSE);
		DeusExPlayer->AugmentationSystem = NULL;
	}

	if (DeusExPlayer->SkillSystem != NULL)
	{
		ASkill* Skill = DeusExPlayer->SkillSystem->FirstSkill;
		while (Skill != NULL)
		{
			ASkill* NextSkill = Skill->Next;
			GLevel->DestroyActor(Skill, FALSE);
			Skill = NextSkill;
		}

		GLevel->DestroyActor(DeusExPlayer->SkillSystem, FALSE);
		DeusExPlayer->SkillSystem = NULL;
	}

	if (DeusExPlayer->flagBase != NULL)
	{
		DeusExPlayer->flagBase->DeleteAllFlags();
		if (DeusExPlayer->flagBase != NULL)
			delete DeusExPlayer->flagBase;
		DeusExPlayer->flagBase = NULL;
	}

	if (Player->carriedDecoration != NULL)
	{
		GLevel->DestroyActor(Player->carriedDecoration, FALSE);
		Player->carriedDecoration = NULL;
	}

	unguard;
}

void DDeusExGameEngine::SaveCurrentLevel(INT DirectoryIndex, bool bSavePlayer)
{
	guard(DDeusExGameEngine::SaveCurrentLevel);

	(void)bSavePlayer;

	FString DirectoryName = DxSaveSlotName(DirectoryIndex);
	FString DirectoryPath = DxSaveSlotPath(DirectoryIndex);
	GFileManager->MakeDirectory(*DirectoryPath, TRUE);

	ALevelInfo* LevelInfo = CastChecked<ALevelInfo>(GLevel->Actors(0));
	LevelInfo->LevelAction = LEVACT_Saving;
	PaintProgress();

    // Original SaveCurrentLevel() does NOT prune travel actors.
    // Browse() explicitly calls PruneTravelActors() before SaveCurrentLevel(-2, FALSE)
    // on same-mission travel.  SaveGame() calls SaveCurrentLevel() for ordinary saves
    // and must not destroy the player-owned augmentation/skill/flag chains.

    if (DirectoryIndex != -1)
        GWarn->BeginSlowTask(LocalizeGeneral(TEXT("Saving"), TEXT("DeusEx")), TRUE, FALSE);

	if (GLevel->BrushTracker != NULL)
	{
		delete GLevel->BrushTracker;
		GLevel->BrushTracker = NULL;
	}

	GLevel->CleanupDestroyed(TRUE);

	ADeusExLevelInfo* DeusInfo = GetDeusExLevelInfo();
	if (DeusInfo == NULL)
	{
		// Original edge path: log and exit.  It does not close the slow task,
		// restore LevelAction, rebuild BrushTracker, or flush GCache here.
		GLog->Logf(TEXT("WARNING!  DeusExLevelInfo MISSING!"));
		return;
	}

	FString Filename = DxMapSavePath(DirectoryName, DeusInfo->MapName);
	UObject::SavePackage(GLevel->GetOuter(), GLevel, 0, *Filename, GLog, NULL);

	for (INT ActorIndex = 0; ActorIndex < GLevel->Actors.Num(); ActorIndex++)
	{
		AMover* Mover = Cast<AMover>(GLevel->Actors(ActorIndex));
		if (Mover != NULL)
		{
			Mover->SavedPos = FVector(-1.0f, -1.0f, -1.0f);
		}
	}

	GLevel->BrushTracker = GNewBrushTracker(GLevel);

	if (DirectoryIndex != -1)
		GWarn->EndSlowTask();

	LevelInfo->LevelAction = LEVACT_None;
	GCache.Flush(0, ~0, FALSE);

	unguard;
}

void DDeusExGameEngine::CopySaveGameFiles(FString& SourceDirectory, FString& DestinationDirectory)
{
	guard(DDeusExGameEngine::CopySaveGameFiles);

	FString SourceDirectorySearchPath = FString::Printf(TEXT("%s\\*.%s"), *SourceDirectory, *FURL::DefaultSaveExt);
	TArray<FString> SourceDirectoryFiles = GFileManager->FindFiles(*SourceDirectorySearchPath, TRUE, FALSE);

	GFileManager->MakeDirectory(*DestinationDirectory);

	for (INT iFile = 0; iFile < SourceDirectoryFiles.Num(); iFile++)
	{
		FString SourceFilename = FString::Printf(TEXT("%s\\%s"), *SourceDirectory, *SourceDirectoryFiles(iFile));
		FString DestinationFilename = FString::Printf(TEXT("%s\\%s"), *DestinationDirectory, *SourceDirectoryFiles(iFile));
		GFileManager->Copy(*DestinationFilename, *SourceFilename);
	}

	unguard;
}

void DDeusExGameEngine::DeleteSaveGameFiles(FString SaveDirectory)
{
	guard(DDeusExGameEngine::DeleteSaveGameFiles);

	FString Directory = SaveDirectory;
	if (Directory.Len() == 0)
		Directory = DxSaveSlotPath(-2);

	if (GLevel != NULL)
		UObject::ResetLoaders(GLevel->GetOuter(), FALSE, FALSE);

	FString Wildcard = FString::Printf(TEXT("%s\\*.*"), *Directory);
	TArray<FString> Files = GFileManager->FindFiles(*Wildcard, TRUE, FALSE);

	for (INT FileIndex = 0; FileIndex < Files.Num(); FileIndex++)
	{
		FString FullFilename = FString::Printf(TEXT("%s\\%s"), *Directory, *Files(FileIndex));
		GFileManager->Delete(*FullFilename, FALSE, FALSE);
	}

	unguard;
}

void DDeusExGameEngine::DeleteGame(INT Position)
{
	guard(DDeusExGameEngine::DeleteGame);

	if (Position >= 0)
	{
		FString Directory = DxSaveSlotPath(Position);
		GFileManager->DeleteDirectory(*Directory, FALSE, TRUE);
	}

	unguard;
}

UDeusExSaveInfo* DDeusExGameEngine::GetSaveInfo(UPackage* SaveInfoPackage)
{
	guard(DDeusExGameEngine::GetSaveInfo);

	UDeusExSaveInfo* SaveInfo = LoadSaveInfo(-2);
	if (SaveInfoPackage != NULL && SaveInfo == NULL)
	{
		FName SaveInfoName(SAVEINFO_Name, FNAME_Add);
		SaveInfo = new(SaveInfoPackage, SaveInfoName, RF_Public) UDeusExSaveInfo();
	}
	return SaveInfo;

	unguard;
}

UDeusExSaveInfo* DDeusExGameEngine::LoadSaveInfo(INT DirectoryIndex)
{
	guard(DDeusExGameEngine::LoadSaveInfo);

	FString DirectoryName = DxSaveSlotName(DirectoryIndex);
	FString Filename = FString::Printf(
		TEXT("%s\\%s\\%s.%s"),
		DxTextOrEmpty(GSys->SavePath),
		*DirectoryName,
		SAVE_SaveInfoFilename,
		DxSaveExtOrEmpty());

	UObject::CreatePackage(NULL, SAVE_SaveInfoFilename);
	return Cast<UDeusExSaveInfo>(UObject::StaticLoadObject(
		UDeusExSaveInfo::StaticClass(),
		NULL,
		SAVEINFO_Name,
		*Filename,
		LOAD_NoWarn,
		NULL));

	unguard;
}

UBOOL DDeusExGameEngine::Browse(FURL URL, const TMap<FString,FString>* TravelInfo, FString& Error)
{
	guard(DDeusExGameEngine::Browse);

	Error = TEXT("");

	// Original builds this local string before calling the parent Browse().  It is
	// not otherwise consumed in the recovered control flow, but keeping it makes
	// the source-level lifetime/order match the ASM more closely.
	FString UrlString;
	UrlString = URL.String(FALSE);

	if (Super::Browse(URL, TravelInfo, Error) == TRUE)
		return TRUE;

	/*
		Original Browse() state machine, ASM proc 00187:
		  restart   -> rebuild URL from DeusExLevelInfo.MapName or LastURL, add loadonly,
		               delete Current, then fall through to local-map loading.
		  loadgame= -> copy selected save slot into Current, load Current\Map.dx with
		               ?load?loadonly?loadgame, update LastURL only on success.
		  load=     -> target map is load= option.
		  otherwise -> local internal URL target is URL.Map.
		  same mission and not bStartingNewGame -> save current map into Current and
		               prefer already-saved Current\NextMap.dx if it exists.
		  mission change / loadonly / unknown mission -> clear Current, load target,
		               then fallback to previous map on failure.
	*/

	if (URL.HasOption(TEXT("restart")) == TRUE)
	{
		ADeusExLevelInfo* DeusInfo = GetDeusExLevelInfo();
		if (DeusInfo != NULL)
		{
			FURL RestartURL(*DeusInfo->MapName);
			RestartURL.Portal = LastURL.Portal;
			RestartURL.AddOption(TEXT("loadonly"));
			URL = RestartURL;
		}
		else
		{
			URL = LastURL;
		}

		DeleteSaveGameFiles(TEXT(""));

		if (URL.IsLocalInternal() == TRUE)
			return LoadMap(URL, NULL, TravelInfo, Error) != NULL;

		if (URL.IsInternal() == TRUE && GIsClient == FALSE)
		{
			Error = LocalizeError("ServerOpen", TEXT("DeusEx"), NULL);
			return FALSE;
		}

		return FALSE;
	}

	const TCHAR* LoadGameOption = URL.GetOption(TEXT("loadgame="), NULL);
	if (LoadGameOption != NULL)
	{
		GLog->Logf(TEXT("Doing loadgame, not load"));

		INT LoadDirectoryIndex = appAtoi(LoadGameOption);
		UDeusExSaveInfo* SaveInfo = LoadSaveInfo(LoadDirectoryIndex);
		if (SaveInfo == NULL)
			return FALSE;

		FString CurrentDirectory = FString::Printf(TEXT("%s\\%s"), DxTextOrEmpty(GSys->SavePath), SAVE_CurrentDirectory);

		FString SourceDirectory;
		if (SaveInfo->DirectoryIndex == -1)
		{
			SourceDirectory = FString::Printf(TEXT("%s\\%s"), DxTextOrEmpty(GSys->SavePath), SAVE_QuickSaveDirectory);
		}
		else
		{
			// ASM uses the loadgame= option value here, not SaveInfo->DirectoryIndex.
			SourceDirectory = FString::Printf(TEXT("%s\\%s%04d"), DxTextOrEmpty(GSys->SavePath), SAVE_DirPrefix, LoadDirectoryIndex);
		}

		DeleteSaveGameFiles(CurrentDirectory);
		CopySaveGameFiles(SourceDirectory, CurrentDirectory);

		FString LoadURLText = FString::Printf(
			TEXT("%s\\%s.%s?load?loadonly?loadgame"),
			*CurrentDirectory,
			*SaveInfo->MapName,
			DxSaveExtOrEmpty());

		FURL LoadURL(&URL, *LoadURLText, TRAVEL_Absolute);

		// Original loadgame branch passes a local FString to LoadMap(), not the
		// caller's Error parameter.  On failure Browse() returns FALSE with Error
		// still equal to the empty string initialized above.
		FString LoadGameError;
		ULevel* LoadedLevel = LoadMap(LoadURL, NULL, NULL, LoadGameError);
		if (LoadedLevel != NULL)
		{
			LastURL = GLevel->URL;
			return TRUE;
		}

		return FALSE;
	}

	const TCHAR* LoadOption = URL.GetOption(TEXT("load="), NULL);
	if (LoadOption == NULL && URL.IsLocalInternal() == FALSE)
	{
		if (URL.IsLocalInternal() == TRUE)
			return LoadMap(URL, NULL, TravelInfo, Error) != NULL;

		if (URL.IsInternal() == TRUE && GIsClient == FALSE)
		{
			Error = LocalizeError("ServerOpen", TEXT("DeusEx"), NULL);
			return FALSE;
		}

		return FALSE;
	}

	FString NextMapName;
	if (LoadOption != NULL)
		NextMapName = LoadOption;
	else
		NextMapName = URL.Map;

	FString PreviousMapName;
	if (GLevel != NULL)
		PreviousMapName = GLevel->URL.Map;
	else
		PreviousMapName = TEXT("Dx.Dx");

	GLog->Logf(TEXT("Doing load, not loadgame"));

	INT CurrentMissionNumber = GetCurrentMissionNumber();
	INT NextMissionNumber = GetNextMissionNumber(NextMapName);
	GLog->Logf(TEXT("Current mission number is %d, next is %d"), CurrentMissionNumber, NextMissionNumber);

	if (NextMissionNumber != -1 && URL.HasOption(TEXT("loadonly")) == FALSE)
	{
		GLog->Logf(TEXT("Attempting to get player"));

		APlayerPawn* Player = DxFirstViewportActor(Client);
		if (NextMissionNumber == CurrentMissionNumber && DxPlayerBlocksSameMissionTravel(Player) == FALSE)
		{
			PruneTravelActors();
			SaveCurrentLevel(-2, FALSE);

			TCHAR* DotDx = appStrstr(*NextMapName, TEXT(".dx"));
			if (DotDx != NULL)
			{
				INT DotDxIndex = (INT)(DotDx - *NextMapName);
				if (DotDxIndex != -1)
					NextMapName = NextMapName.Left(DotDxIndex);
			}

			FString CurrentMapSave = FString::Printf(
				TEXT("%s\\%s\\%s.%s"),
				DxTextOrEmpty(GSys->SavePath),
				SAVE_CurrentDirectory,
				*NextMapName,
				DxSaveExtOrEmpty());

			TArray<FString> MatchingFiles;
			MatchingFiles = GFileManager->FindFiles(*CurrentMapSave, TRUE, FALSE);
			if (MatchingFiles.Num() == 0)
			{
				ULevel* LoadedLevel = LoadMap(URL, NULL, TravelInfo, Error);
				return LoadedLevel != NULL;
			}

			FURL SavedMapURL(&URL, *CurrentMapSave, TRAVEL_Partial);
			SavedMapURL.Portal = URL.Portal;

			// Original computes the URL string and discards it.  Keep the lifetime and
			// side effects of FURL::String() for reconstruction fidelity.
			FString SavedUrlString;
			SavedUrlString = SavedMapURL.String(FALSE);

			ULevel* LoadedSavedLevel = LoadMap(SavedMapURL, NULL, TravelInfo, Error);
			if (LoadedSavedLevel == NULL)
				return FALSE;

			LastURL = GLevel->URL;
			return TRUE;
		}

		DeleteSaveGameFiles(TEXT(""));
		DxClearPlayerStartingNewGame(Player);
	}

	DeleteSaveGameFiles(TEXT(""));
	if (LoadMap(URL, NULL, TravelInfo, Error) != NULL)
		return TRUE;

	URL.Map = PreviousMapName;
	return LoadMap(URL, NULL, TravelInfo, Error) != NULL;

	unguard;
}


INT DDeusExGameEngine::GetCurrentMissionNumber(void)
{
	guard(DDeusExGameEngine::GetCurrentMissionNumber);

	if (GLevel != NULL)
	{
		ADeusExLevelInfo* DeusInfo = GetDeusExLevelInfo();
		if (DeusInfo != NULL)
			return DeusInfo->missionNumber;
	}
	return -1;

	unguard;
}

INT DDeusExGameEngine::GetNextMissionNumber(FString& MapName)
{
	guard(DDeusExGameEngine::GetNextMissionNumber);

	if (MapName.Len() >= 2)
	{
		TCHAR Temp[3];
		Temp[0] = (*MapName)[0];
		Temp[1] = (*MapName)[1];
		Temp[2] = 0;
		if (Temp[0] >= TEXT('0') && Temp[0] <= TEXT('9') && Temp[1] >= TEXT('0') && Temp[1] <= TEXT('9'))
			return appAtoi(Temp);
	}

	return -1;
	unguard;
}

ADeusExLevelInfo* DDeusExGameEngine::GetDeusExLevelInfo(void)
{
	guard(DDeusExGameEngine::GetDeusExLevelInfo);

	// Original DeusEx.dll does not walk GLevel->Actors here.  The decompiled
	// code iterates UObject::GObjObjects looking for the first object whose class
	// derives from ADeusExLevelInfo.  TObjectIterator expresses that original
	// pattern without binding the helper to the current level actor array.
	for (TObjectIterator<ADeusExLevelInfo> It; It; ++It)
	{
		return *It;
	}

	return NULL;
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
