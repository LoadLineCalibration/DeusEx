/*=============================================================================
    ADeusExPlayer_MiscNatives_Reconstructed.cpp
    DeusEx.dll reconstruction pass 13.

    Pass13 changes:
    - Keeps execSetBoolFlagFromString() faithful: direct flagBase dereference.
    - Restores ADeusExPlayer constructor side effects from ASM: one-time
      version log and the original hax0r cheat-enable path.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

/*-----------------------------------------------------------------------------
    Construction / version / destruction.
-----------------------------------------------------------------------------*/

ADeusExPlayer::ADeusExPlayer()
{
    guard(ADeusExPlayer::ADeusExPlayer);

    static UBOOL bLoggedVersion = TRUE;

    if (bLoggedVersion == TRUE)
    {
        bLoggedVersion = FALSE;
        GLog->Logf(NAME_Init, TEXT("*** DEUS EX VERSION %s ***"), GetDeusExVersion());
    }

    if (ParseParam(appCmdLine(), TEXT("hax0r")) == TRUE)
    {
        if (Level == NULL || Level->NetMode == NM_Standalone)
        {
            bCheatsEnabled = TRUE;
            GLog->Logf(NAME_Init, TEXT("*** Cheats Enabled!  U R l337, d00D!  U r0X0r!"));
        }
    }

    unguard;
}

void ADeusExPlayer::Destroy(void)
{
    guard(ADeusExPlayer::Destroy);

    ConHistory = NULL;
    FirstLog = NULL;
    LastLog = NULL;

    Super::Destroy();

    unguard;
}

const TCHAR* ADeusExPlayer::GetDeusExVersion(void)
{
    guard(ADeusExPlayer::GetDeusExVersion);

    static TCHAR VersionString[130];
    appSprintf(VersionString, TEXT("%s %s"), appFromAnsi("Mon Mar 19 12:06:14 2001"), TEXT("v1.112fm"));
	
    return VersionString;

    unguard;
}

void ADeusExPlayer::execGetDeusExVersion(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execGetDeusExVersion);

    P_FINISH;
    *(FString*)Result = GetDeusExVersion();

    unguardexec;
}

/*-----------------------------------------------------------------------------
    Conversation binding.
-----------------------------------------------------------------------------*/

void ADeusExPlayer::execConBindEvents(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execConBindEvents);

    P_FINISH;
    ConBindEvents();

    unguardexec;
}

void ADeusExPlayer::ConBindEvents(void)
{
    guard(ADeusExPlayer::ConBindEvents);

    ADeusExLevelInfo* DeusExLevelInfo = NULL;

    if (XLevel != NULL)
    {
        for (INT iActor = 0; iActor < XLevel->Actors.Num(); iActor++)
        {
            DeusExLevelInfo = Cast<ADeusExLevelInfo>(XLevel->Actors(iActor));
            if (DeusExLevelInfo != NULL)
                break;
        }
    }

    if (DeusExLevelInfo == NULL)
    {
        debugf(TEXT("DeusExLevelInfo object missing!  Unable to bind Conversations!"));
        return;
    }

    if (DeusExLevelInfo->missionNumber < 0)
    {
        return;
    }

    TCHAR ConversationPackage[128];
    TCHAR ConversationListName[64];
    TCHAR ConversationObjectName[128];

    const TCHAR* SourcePackage = *DeusExLevelInfo->ConversationPackage;
    if (appStricmp(SourcePackage, TEXT("DeusExConversations")) == 0)
        appStrcpy(ConversationPackage, TEXT("DeusExCon"));
    else
        appStrcpy(ConversationPackage, SourcePackage);

    appSprintf(ConversationListName, TEXT("ConList_Mission%02d"), DeusExLevelInfo->missionNumber);
    appSprintf(ConversationObjectName, TEXT("%sText.%s"), ConversationPackage, ConversationListName);

    DConversationList* ConversationList = Cast<DConversationList>(StaticLoadObject(DConversationList::StaticClass(), NULL, ConversationObjectName, NULL, LOAD_NoWarn, NULL));
    if (ConversationList != NULL)
        ConversationList->BindConversations(this);

    unguard;
}

/*-----------------------------------------------------------------------------
    Flags.
-----------------------------------------------------------------------------*/

void ADeusExPlayer::execSetBoolFlagFromString(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execSetBoolFlagFromString);

    P_GET_STR(FlagNameString);
    P_GET_UBOOL(bFlagValue);
    P_FINISH;

    FName FlagName(*FlagNameString, FNAME_Add);

    // Original ASM directly dereferences flagBase at ADeusExPlayer+0x890.
    // Do not add a NULL guard in the faithful reconstruction.
    flagBase->SetBool(FlagName, bFlagValue, TRUE, -1);

    *(FName*)Result = FlagName;

    unguardexec;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
