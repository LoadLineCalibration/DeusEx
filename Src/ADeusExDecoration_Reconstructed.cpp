/*=============================================================================
	ADeusExDecoration_Reconstructed_Reconstructed.cpp
	DeusEx.dll reconstruction pass 13.

	ADeusExDecoration only contributes the conversation binding native in this DLL.
	The implementation is the same conversation-list lookup shape used by
	ADeusExPlayer and AScriptedPawn, with original string literals kept in-place.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

void ADeusExDecoration::execConBindEvents(FFrame& Stack, RESULT_DECL)
{
	guard(ADeusExDecoration::execConBindEvents);
	P_FINISH;
	ConBindEvents();
	unguardexec;
}

void ADeusExDecoration::ConBindEvents(void)
{
	guard(ADeusExDecoration::ConBindEvents);

	ADeusExLevelInfo* DeusExLevelInfo = NULL;

	for (INT iActor = 0; iActor < XLevel->Actors.Num(); iActor++)
	{
		DeusExLevelInfo = Cast<ADeusExLevelInfo>(XLevel->Actors(iActor));
		if (DeusExLevelInfo != NULL)
			break;
	}

	if (DeusExLevelInfo != NULL && DeusExLevelInfo->missionNumber >= 0)
	{
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

		DConversationList* ConversationList = Cast<DConversationList>(StaticLoadObject(
			DConversationList::StaticClass(),
			NULL,
			ConversationObjectName,
			NULL,
			LOAD_NoWarn,
			NULL));

		if (ConversationList != NULL)
			ConversationList->BindConversations(this);
	}

	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
