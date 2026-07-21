/*=============================================================================
    ADeusExPlayer_SaveNatives_Reconstructed_Reconstructed.cpp
    DeusEx.dll reconstruction pass 13.

    Pass13 focus:
    - Align ADeusExPlayer save/object native wrappers with ASM.
    - Remove reconstruction-side safety guards where original DLL directly
      dereferences GetLevel()->Engine or flagBase.
    - Keep object creation outers exactly as observed in ASM:
      XGameDirectory -> GetTransientPackage(), all other helper objects -> GetOuter().
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

/*-----------------------------------------------------------------------------
    Save / directory / dump helper natives.
-----------------------------------------------------------------------------*/

void ADeusExPlayer::execSaveGame(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execSaveGame);

    P_GET_INT(Position);
    P_GET_STR_OPTX(SaveDesc, TEXT(""));
    P_FINISH;

    // Original ASM directly calls GetLevel(), then uses [Level+0x40] as Engine.
    // No NULL guard exists in the DLL wrapper.
    DDeusExGameEngine* DeusExEngine = (DDeusExGameEngine*)GetLevel()->Engine;
    DeusExEngine->SaveGame(Position, SaveDesc);

    unguardexec;
}

void ADeusExPlayer::execDeleteSaveGameFiles(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execDeleteSaveGameFiles);

    P_GET_STR_OPTX(SaveDirectory, TEXT(""));
    P_FINISH;

    // Original ASM directly calls GetLevel(), then uses [Level+0x40] as Engine.
    DDeusExGameEngine* DeusExEngine = (DDeusExGameEngine*)GetLevel()->Engine;
    DeusExEngine->DeleteSaveGameFiles(SaveDirectory);

    unguardexec;
}

void ADeusExPlayer::execCreateGameDirectoryObject(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execCreateGameDirectoryObject);

    P_FINISH;
    *(XGameDirectory**)Result = new(GetTransientPackage(), NAME_None) XGameDirectory();

    unguardexec;
}

void ADeusExPlayer::execCreateDumpLocationObject(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execCreateDumpLocationObject);

    P_FINISH;
    *(UDumpLocation**)Result = new(GetOuter(), NAME_None) UDumpLocation();

    unguardexec;
}

void ADeusExPlayer::execCreateDataVaultImageNoteObject(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execCreateDataVaultImageNoteObject);

    P_FINISH;
    *(UDataVaultImageNote**)Result = new(GetOuter(), NAME_None) UDataVaultImageNote();

    unguardexec;
}

void ADeusExPlayer::execCreateLogObject(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execCreateLogObject);

    P_FINISH;
    *(UDeusExLog**)Result = new(GetOuter(), NAME_None) UDeusExLog();

    unguardexec;
}

void ADeusExPlayer::execCreateHistoryObject(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execCreateHistoryObject);

    P_FINISH;
    *(DConHistory**)Result = new(GetOuter(), NAME_None) DConHistory();

    unguardexec;
}

void ADeusExPlayer::execCreateHistoryEvent(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execCreateHistoryEvent);

    P_FINISH;
    *(DConHistoryEvent**)Result = new(GetOuter(), NAME_None) DConHistoryEvent();

    unguardexec;
}

void ADeusExPlayer::execUnloadTexture(FFrame& Stack, RESULT_DECL)
{
    guard(ADeusExPlayer::execUnloadTexture);

    P_GET_OBJECT(UTexture, Texture);
    P_FINISH;

    if (Texture != NULL)
    {
        for (INT iMip = 0; iMip < Texture->Mips.Num(); iMip++)
        {
            //Texture->Mips(iMip).DataArray.Empty();
            Texture->Mips(iMip).DataArray.Unload();
        }
    }

    unguardexec;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
