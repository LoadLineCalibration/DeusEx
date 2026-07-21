/*=============================================================================
	XGameDirectory_Reconstructed_Pass41.cpp
	DeusEx.dll reconstruction pass 09.

	Pass09 focus:
	- Earlier passes kept UDumpLocation deferred; Pass27 reconstructs it separately.
	- Align XGameDirectory more tightly with DeusEx.dll ASM.
	- Keep original save/menu strings at their observed call sites.

	Pass22 change:
	- GetSaveFreeSpace() now calls GetProcAddress(GetModuleHandleA("KERNEL32"),
	  "GetDiskFreeSpaceExA") without the reconstruction-side Kernel32 guard,
	  matching the original call order.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

static const TCHAR* DxEmptyText(void)
{
	return TEXT("");
}

static const TCHAR* DxTextPtr(const FString& Value)
{
	if (Value.Len() > 0)
		return *Value;
	return DxEmptyText();
}

static const TCHAR* DxDefaultSaveExt(void)
{
	if (FURL::DefaultSaveExt.Len() > 0)
		return *FURL::DefaultSaveExt;
	return DxEmptyText();
}

static FString DxSaveDirectoryNameFromIndex(INT SaveIndex)
{
	if (SaveIndex == -1)
		return FString(TEXT("QuickSave"));

	return FString::Printf(TEXT("%s%04d"), TEXT("Save"), SaveIndex);
}

static FString DxSaveInfoPathFromSaveIndex(INT SaveIndex)
{
	if (SaveIndex == -1)
	{
		return FString::Printf(
			TEXT("%s\\%s\\%s.%s"),
			DxTextPtr(GSys->SavePath),
			TEXT("QuickSave"),
			TEXT("SaveInfo"),
			DxDefaultSaveExt());
	}

	return FString::Printf(
		TEXT("%s\\%s%04d\\%s.%s"),
		DxTextPtr(GSys->SavePath),
		TEXT("Save"),
		SaveIndex,
		TEXT("SaveInfo"),
		DxDefaultSaveExt());
}

static FString DxSaveInfoPathFromDirectoryName(const FString& DirectoryName)
{
	return FString::Printf(
		TEXT("%s\\%s\\%s.%s"),
		DxTextPtr(GSys->SavePath),
		DxTextPtr(DirectoryName),
		TEXT("SaveInfo"),
		DxDefaultSaveExt());
}

static UDeusExSaveInfo* DxLoadDeusExSaveInfo(const FString& Filename)
{
	return Cast<UDeusExSaveInfo>(UObject::StaticLoadObject(
		UDeusExSaveInfo::StaticClass(),
		NULL,
		TEXT("MyDeusExSaveInfo"),
		*Filename,
		LOAD_NoWarn | LOAD_Quiet,
		NULL));
}

static void DxAddLoadedSaveInfoPointer(XGameDirectory* Directory, UDeusExSaveInfo* SaveInfo)
{
	if (Directory != NULL)
		Directory->loadedSaveInfoPointers.AddItem(SaveInfo);
}


XGameDirectory::XGameDirectory()
: gameDirectoryType(GD_Maps)
, currentFilter()
, directoryList()
, loadedSaveInfoPointers()
, tempSaveInfo(NULL)
{
	guard(XGameDirectory::XGameDirectory);
	unguard;
}

void XGameDirectory::Destroy(void)
{
	guard(XGameDirectory::Destroy);

	directoryList.Empty();
	PurgeAllSaveInfo();

	if (tempSaveInfo != NULL)
	{
		// Original Destroy() directly invokes the save-info destructor. It does not
		// ResetLoaders() here and does not bother clearing tempSaveInfo afterwards
		// because the XGameDirectory object is being destroyed.
		delete tempSaveInfo;
	}

	Super::Destroy();
	unguard;
}

void XGameDirectory::Serialize(FArchive& Ar)
{
	guard(XGameDirectory::Serialize);
	Super::Serialize(Ar);
	Ar << directoryList;
	unguard;
}

void XGameDirectory::GetGameDirectory(void)
{
	guard(XGameDirectory::GetGameDirectory);

	if (gameDirectoryType == GD_Maps)
	{
		GetMapsDirectory();
	}
	else if (gameDirectoryType == GD_SaveGames)
	{
		GetSaveGamesDirectory();
	}

	unguard;
}

void XGameDirectory::GetMapsDirectory(void)
{
	guard(XGameDirectory::GetMapsDirectory);

	SetDirType(GD_Maps);
	directoryList.Empty();

	FString Mask = FString::Printf(TEXT("*.%s"), DxTextPtr(FURL::DefaultMapExt));

	for (INT PathIndex = 0; PathIndex < GSys->Paths.Num(); PathIndex++)
	{
		FString Path = GSys->Paths(PathIndex);
		TCHAR* MaskPos = appStrstr(DxTextPtr(Path), *Mask);
		if (MaskPos != NULL)
		{
			TCHAR SearchRoot[256];
			appStrcpy(SearchRoot, DxTextPtr(Path));

			TCHAR* SearchMaskPos = appStrstr(SearchRoot, *Mask);
			if (SearchMaskPos != NULL)
			{
				*SearchMaskPos = 0;
				appStrcat(SearchRoot, TEXT("\\"));
				appStrcat(SearchRoot, *Mask);

				TArray<FString> FoundFiles;
				FoundFiles = GFileManager->FindFiles(SearchRoot, TRUE, FALSE);

				for (INT FoundIndex = 0; FoundIndex < FoundFiles.Num(); FoundIndex++)
				{
					UBOOL bAlreadyListed = FALSE;
					for (INT ListedIndex = 0; ListedIndex < directoryList.Num(); ListedIndex++)
					{
						if (appStricmp(DxTextPtr(directoryList(ListedIndex)), DxTextPtr(FoundFiles(FoundIndex))) == 0)
						{
							bAlreadyListed = TRUE;
							break;
						}
					}

					if (bAlreadyListed == FALSE)
						new(directoryList) FString(FoundFiles(FoundIndex));
				}
			}
		}
	}

	unguard;
}

void XGameDirectory::GetSaveGamesDirectory(void)
{
	guard(XGameDirectory::GetSaveGamesDirectory);

	SetDirType(GD_SaveGames);
	directoryList.Empty();

	FString SearchPath = FString::Printf(
		TEXT("%s\\%s*"),
		DxTextPtr(GSys->SavePath),
		TEXT("Save"));

	TArray<FString> FoundDirectories;
	FoundDirectories = GFileManager->FindFiles(*SearchPath, FALSE, TRUE);

	for (INT FoundIndex = 0; FoundIndex < FoundDirectories.Num(); FoundIndex++)
	{
		new(directoryList) FString(FoundDirectories(FoundIndex));
	}

	unguard;
}

INT XGameDirectory::GetSaveFreeSpace(void)
{
	guard(XGameDirectory::GetSaveFreeSpace);

	FString DefaultDirectory = GFileManager->GetDefaultDirectory();

	TCHAR RootPathName[4];
	RootPathName[0] = TEXT('C');
	RootPathName[1] = TEXT(':');
	RootPathName[2] = TEXT('\\');
	RootPathName[3] = 0;

	if (DefaultDirectory.Len() > 0)
		RootPathName[0] = (*DefaultDirectory)[0];

	HMODULE Kernel32 = GetModuleHandleA("KERNEL32");

	// Original ASM calls GetProcAddress() with the result of GetModuleHandleA()
	// directly.  It does not add a separate Kernel32 != NULL guard before the
	// call.  Keep that strict ordering; a NULL hModule simply leads to a NULL
	// procedure pointer on Win32.
	typedef BOOL (WINAPI *TGetDiskFreeSpaceExAProc)(LPCSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
	TGetDiskFreeSpaceExAProc GetDiskFreeSpaceExAProc = (TGetDiskFreeSpaceExAProc)GetProcAddress(Kernel32, "GetDiskFreeSpaceExA");
	if (GetDiskFreeSpaceExAProc != NULL)
	{
		ANSICHAR RootPathAnsi[8];
		appMemzero(RootPathAnsi, sizeof(RootPathAnsi));
		RootPathAnsi[0] = (ANSICHAR)RootPathName[0];
		RootPathAnsi[1] = ':';
		RootPathAnsi[2] = '\\';
		RootPathAnsi[3] = 0;

		ULARGE_INTEGER FreeBytesAvailable;
		ULARGE_INTEGER TotalNumberOfBytes;
		ULARGE_INTEGER TotalNumberOfFreeBytes;
		FreeBytesAvailable.QuadPart = 0;
		TotalNumberOfBytes.QuadPart = 0;
		TotalNumberOfFreeBytes.QuadPart = 0;

		if (GetDiskFreeSpaceExAProc(RootPathAnsi, &FreeBytesAvailable, &TotalNumberOfBytes, &TotalNumberOfFreeBytes) != FALSE)
			return (INT)(FreeBytesAvailable.QuadPart / 1024);
	}

	DWORD SectorsPerCluster = 0;
	DWORD BytesPerSector = 0;
	DWORD NumberOfFreeClusters = 0;
	DWORD TotalNumberOfClusters = 0;

	if (GetDiskFreeSpace(RootPathName, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters) != FALSE)
	{
		QWORD FreeBytes = QWORD(SectorsPerCluster) * QWORD(BytesPerSector) * QWORD(NumberOfFreeClusters);
		return (INT)(FreeBytes / 1024);
	}

	return 0;
	unguard;
}

INT XGameDirectory::GetSaveDirectorySize(INT SaveIndex)
{
	guard(XGameDirectory::GetSaveDirectorySize);

	INT TotalKBytes = 0;
	FString DirectoryName = DxSaveDirectoryNameFromIndex(SaveIndex);
	FString SearchPath = FString::Printf(
		TEXT("%s\\%s\\*.*"),
		DxTextPtr(GSys->SavePath),
		*DirectoryName);

	TArray<FString> Files;
	Files = GFileManager->FindFiles(*SearchPath, TRUE, FALSE);

	for (INT FileIndex = 0; FileIndex < Files.Num(); FileIndex++)
	{
		FString FullFilename = FString::Printf(
			TEXT("%s\\%s\\%s"),
			DxTextPtr(GSys->SavePath),
			*DirectoryName,
			DxTextPtr(Files(FileIndex)));

		INT FileSize = GFileManager->FileSize(*FullFilename);
		TotalKBytes += FileSize / 1024;
	}

	return TotalKBytes;
	unguard;
}

INT XGameDirectory::GetNewSaveFileIndex(void)
{
	guard(XGameDirectory::GetNewSaveFileIndex);

	INT LargestIndex = 0;
	if (directoryList.Num() <= 0)
		return 1;

	for (INT FileIndex = 0; FileIndex < directoryList.Num(); FileIndex++)
	{
		TCHAR TempName[128];
		appStrcpy(TempName, DxTextPtr(directoryList(FileIndex)));

		TCHAR* SavePrefix = appStrstr(TempName, TEXT("Save"));
		if (SavePrefix != NULL)
		{
			// ASM: word ptr [SavePrefix+8] = 0; appAtoi(SavePrefix+10).
			// Keep the original odd parsing exactly: it skips the first digit after "Save".
			SavePrefix[4] = 0;
			INT ParsedIndex = appAtoi(SavePrefix + 5);
			if (ParsedIndex >= LargestIndex)
				LargestIndex = ParsedIndex;
		}
	}

	return LargestIndex + 1;
	unguard;
}

FString XGameDirectory::GenerateSaveFilename(INT SaveIndex)
{
	guard(XGameDirectory::GenerateSaveFilename);
	return FString::Printf(TEXT("%s%04d.%s"), TEXT("Save"), SaveIndex, DxDefaultSaveExt());
	unguard;
}

FString XGameDirectory::GenerateNewSaveFilename(INT NewIndex)
{
	guard(XGameDirectory::GenerateNewSaveFilename);

	GetSaveGamesDirectory();

	INT UseIndex = NewIndex;
	if (UseIndex == -1)
		UseIndex = GetNewSaveFileIndex();

	return GenerateSaveFilename(UseIndex);
	unguard;
}

INT XGameDirectory::GetDirCount(void)
{
	guard(XGameDirectory::GetDirCount);
	return directoryList.Num();
	unguard;
}

const TCHAR* XGameDirectory::GetDirFilename(INT FileIndex)
{
	guard(XGameDirectory::GetDirFilename);

	if (FileIndex >= directoryList.Num())
		return NULL;

	// Original only checks the upper bound. Avoid TArray::operator() here so a
	// negative FileIndex keeps the same raw-index behaviour instead of tripping
	// a source-level bounds assertion earlier than the DLL did.
	FString* DirectoryData = (FString*)directoryList.GetData();
	return DxTextPtr(DirectoryData[FileIndex]);
	unguard;
}

UDeusExSaveInfo* XGameDirectory::GetSaveInfo(INT FileIndex)
{
	guard(XGameDirectory::GetSaveInfo);

	UDeusExSaveInfo* SaveInfo = DxLoadDeusExSaveInfo(DxSaveInfoPathFromSaveIndex(FileIndex));
	DxAddLoadedSaveInfoPointer(this, SaveInfo);
	return SaveInfo;

	unguard;
}

UDeusExSaveInfo* XGameDirectory::GetSaveInfoFromDirectoryIndex(INT DirectoryIndex)
{
	guard(XGameDirectory::GetSaveInfoFromDirectoryIndex);

	UDeusExSaveInfo* SaveInfo = NULL;
	if (DirectoryIndex >= 0 && DirectoryIndex < directoryList.Num())
	{
		SaveInfo = DxLoadDeusExSaveInfo(DxSaveInfoPathFromDirectoryName(directoryList(DirectoryIndex)));
		DxAddLoadedSaveInfoPointer(this, SaveInfo);
	}

	return SaveInfo;
	unguard;
}

UDeusExSaveInfo* XGameDirectory::GetTempSaveInfo(void)
{
	guard(XGameDirectory::GetTempSaveInfo);

	if (tempSaveInfo == NULL)
		tempSaveInfo = new(UObject::GetTransientPackage(), NAME_None) UDeusExSaveInfo();

	return tempSaveInfo;
	unguard;
}

void XGameDirectory::DeleteSaveInfo(UDeusExSaveInfo* SaveInfo)
{
	guard(XGameDirectory::DeleteSaveInfo);

	if (SaveInfo == NULL)
		return;

	for (INT Index = 0; Index < loadedSaveInfoPointers.Num(); Index++)
	{
		if (loadedSaveInfoPointers(Index) == SaveInfo)
		{
			UObject::ResetLoaders(SaveInfo->GetOuter(), FALSE, TRUE);
			delete SaveInfo;
			loadedSaveInfoPointers.Remove(Index);
			return;
		}
	}

	unguard;
}

void XGameDirectory::PurgeAllSaveInfo(void)
{
	guard(XGameDirectory::PurgeAllSaveInfo);

	while (loadedSaveInfoPointers.Num() > 0)
		DeleteSaveInfo(loadedSaveInfoPointers(0));

	loadedSaveInfoPointers.Empty();
	unguard;
}

void XGameDirectory::SetDirType(EGameDirectoryTypes NewGameDirectoryType)
{
	guard(XGameDirectory::SetDirType);
	gameDirectoryType = NewGameDirectoryType;
	unguard;
}

void XGameDirectory::SetDirFilter(FString& NewFilter)
{
	guard(XGameDirectory::SetDirFilter);
	currentFilter = NewFilter;
	unguard;
}

void XGameDirectory::execGetGameDirectory(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetGameDirectory);
	P_FINISH;
	GetGameDirectory();
	unguardexec;
}

void XGameDirectory::execGetNewSaveFileIndex(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetNewSaveFileIndex);
	P_FINISH;
	*(INT*)Result = GetNewSaveFileIndex();
	unguardexec;
}

void XGameDirectory::execGenerateSaveFilename(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGenerateSaveFilename);
	P_GET_INT(SaveIndex);
	P_FINISH;
	*(FString*)Result = GenerateSaveFilename(SaveIndex);
	unguardexec;
}

void XGameDirectory::execGenerateNewSaveFilename(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGenerateNewSaveFilename);
	P_GET_INT_OPTX(NewIndex, -1);
	P_FINISH;
	*(FString*)Result = GenerateNewSaveFilename(NewIndex);
	unguardexec;
}

void XGameDirectory::execGetDirCount(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetDirCount);
	P_FINISH;
	*(INT*)Result = GetDirCount();
	unguardexec;
}

void XGameDirectory::execGetDirFilename(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetDirFilename);
	P_GET_INT(FileIndex);
	P_FINISH;

	// Original wrapper assigns the raw GetDirFilename() pointer into the result
	// string with no NULL fallback. If FileIndex is invalid, GetDirFilename()
	// returns NULL and the original path would dereference it while copying.
	*(FString*)Result = GetDirFilename(FileIndex);

	unguardexec;
}

void XGameDirectory::execSetDirType(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execSetDirType);
	P_GET_BYTE(NewType);
	P_FINISH;
	SetDirType(EGameDirectoryTypes(NewType));
	unguardexec;
}

void XGameDirectory::execSetDirFilter(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execSetDirFilter);
	P_GET_STR(NewFilter);
	P_FINISH;
	SetDirFilter(NewFilter);
	unguardexec;
}

void XGameDirectory::execGetSaveInfo(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetSaveInfo);
	P_GET_INT(FileIndex);
	P_FINISH;
	*(UDeusExSaveInfo**)Result = GetSaveInfo(FileIndex);
	unguardexec;
}

void XGameDirectory::execGetSaveInfoFromDirectoryIndex(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetSaveInfoFromDirectoryIndex);
	P_GET_INT(DirectoryIndex);
	P_FINISH;
	*(UDeusExSaveInfo**)Result = GetSaveInfoFromDirectoryIndex(DirectoryIndex);
	unguardexec;
}

void XGameDirectory::execGetTempSaveInfo(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetTempSaveInfo);
	P_FINISH;
	*(UDeusExSaveInfo**)Result = GetTempSaveInfo();
	unguardexec;
}

void XGameDirectory::execDeleteSaveInfo(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execDeleteSaveInfo);
	P_GET_OBJECT(UDeusExSaveInfo, SaveInfo);
	P_FINISH;
	DeleteSaveInfo(SaveInfo);
	unguardexec;
}

void XGameDirectory::execPurgeAllSaveInfo(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execPurgeAllSaveInfo);
	P_FINISH;
	PurgeAllSaveInfo();
	unguardexec;
}

void XGameDirectory::execGetSaveFreeSpace(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetSaveFreeSpace);
	P_FINISH;
	*(INT*)Result = GetSaveFreeSpace();
	unguardexec;
}

void XGameDirectory::execGetSaveDirectorySize(FFrame& Stack, RESULT_DECL)
{
	guard(XGameDirectory::execGetSaveDirectorySize);
	P_GET_INT(SaveIndex);
	P_FINISH;
	*(INT*)Result = GetSaveDirectorySize(SaveIndex);
	unguardexec;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
