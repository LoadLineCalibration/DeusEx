/*=============================================================================
    AScriptedPawn_Reconstructed.cpp
    DeusEx.dll reconstruction pass 25.

    Pass25 closes the large AScriptedPawn::Tick audit.  Tick() is aligned against
    the original ASM control flow: gating, bDisappear, PrePivot interpolation,
    visible-only work, timer decay, cloak/fire script calls, facing-target reset,
    and bleeding cadence.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

/*-----------------------------------------------------------------------------
    Conversation binding.
-----------------------------------------------------------------------------*/

void AScriptedPawn::execConBindEvents(FFrame& Stack, RESULT_DECL)
{
    guard(AScriptedPawn::execConBindEvents);

    P_FINISH;
    ConBindEvents();

    unguardexec;
}

void AScriptedPawn::ConBindEvents(void)
{
    guard(AScriptedPawn::ConBindEvents);

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
    Alliance helpers.
-----------------------------------------------------------------------------*/

void AScriptedPawn::execIsValidEnemy(FFrame& Stack, RESULT_DECL)
{
    guard(AScriptedPawn::execIsValidEnemy);

    P_GET_OBJECT(APawn, TestPawn);
    P_GET_UBOOL_OPTX(bCheckAlliance, TRUE);
    P_FINISH;

    *(UBOOL*)Result = IsValidEnemy(TestPawn, bCheckAlliance);

    unguardexec;
}

void AScriptedPawn::execGetPawnAllianceType(FFrame& Stack, RESULT_DECL)
{
    guard(AScriptedPawn::execGetPawnAllianceType);

    P_GET_OBJECT(APawn, QueryPawn);
    P_FINISH;

    *(BYTE*)Result = GetPawnAllianceType(QueryPawn);

    unguardexec;
}

void AScriptedPawn::execGetAllianceType(FFrame& Stack, RESULT_DECL)
{
    guard(AScriptedPawn::execGetAllianceType);

    P_GET_NAME(AllianceName);
    P_FINISH;

    *(BYTE*)Result = GetAllianceType(AllianceName);

    unguardexec;
}

UBOOL AScriptedPawn::IsValidEnemy(APawn* testPawn, UBOOL bCheckAlliance)
{
    guard(AScriptedPawn::IsValidEnemy);

    if (testPawn == NULL)
        return FALSE;

    if (testPawn == this)
        return FALSE;

    if (testPawn->bDeleteMe == TRUE)
        return FALSE;

    if (testPawn->bDetectable == FALSE)
        return FALSE;

    if (testPawn->Health <= 0)
        return FALSE;

    if (bCheckAlliance == TRUE)
    {
        if (GetPawnAllianceType(testPawn) != ALLIANCE_Hostile)
            return FALSE;
    }

    return TRUE;

    unguard;
}

EAllianceType AScriptedPawn::GetAllianceType(FName allianceName)
{
    guard(AScriptedPawn::GetAllianceType);

    EAllianceType Result = ALLIANCE_Neutral;

    if (allianceName != NAME_None)
    {
        for (INT iAlliance = 0; iAlliance < 16; iAlliance++)
        {
            FAllianceInfoEx& Info = AlliancesEx[iAlliance];
            if (Info.AllianceName == allianceName)
            {
                if (Info.AllianceLevel < 0.0f || Info.AgitationLevel >= 1.0f)
                    Result = ALLIANCE_Hostile;
                else if (Info.AllianceLevel > 0.0f)
                    Result = ALLIANCE_Friendly;
                else
                    Result = ALLIANCE_Neutral;
                break;
            }
        }
    }

    if (bLikesNeutral == TRUE && Result == ALLIANCE_Neutral)
        Result = ALLIANCE_Friendly;

    if (bReverseAlliances == TRUE)
    {
        if (Result == ALLIANCE_Friendly)
            Result = ALLIANCE_Hostile;
        else if (Result == ALLIANCE_Hostile)
            Result = ALLIANCE_Friendly;
    }

    return Result;

    unguard;
}

EAllianceType AScriptedPawn::GetPawnAllianceType(APawn* queryPawn)
{
    guard(AScriptedPawn::GetPawnAllianceType);

    EAllianceType Result = ALLIANCE_Neutral;
    EAllianceType PawnToMe = ALLIANCE_Neutral;

    if (queryPawn != NULL)
    {
        AScriptedPawn* ScriptedQueryPawn = Cast<AScriptedPawn>(queryPawn);
        if (ScriptedQueryPawn != NULL)
            PawnToMe = ScriptedQueryPawn->GetAllianceType(Alliance);

        Result = GetAllianceType(queryPawn->Alliance);

        if (PawnToMe == ALLIANCE_Hostile)
            Result = ALLIANCE_Hostile;
    }

    return Result;

    unguard;
}

/*-----------------------------------------------------------------------------
    Carcass memory.
-----------------------------------------------------------------------------*/

void AScriptedPawn::execHaveSeenCarcass(FFrame& Stack, RESULT_DECL)
{
    guard(AScriptedPawn::execHaveSeenCarcass);

    P_GET_NAME(CarcassName);
    P_FINISH;

    *(UBOOL*)Result = HaveSeenCarcass(CarcassName);

    unguardexec;
}

void AScriptedPawn::execAddCarcass(FFrame& Stack, RESULT_DECL)
{
    guard(AScriptedPawn::execAddCarcass);

    P_GET_NAME(CarcassName);
    P_FINISH;

    AddCarcass(CarcassName);

    unguardexec;
}

UBOOL AScriptedPawn::HaveSeenCarcass(FName carcassName)
{
    guard(AScriptedPawn::HaveSeenCarcass);

    for (INT iCarcass = 0; iCarcass < NumCarcasses; iCarcass++)
    {
        if (Carcasses[iCarcass] == carcassName)
            return TRUE;
    }

    return FALSE;

    unguard;
}

void AScriptedPawn::AddCarcass(FName carcassName)
{
    guard(AScriptedPawn::AddCarcass);

    if (NumCarcasses < 4 && HaveSeenCarcass(carcassName) == FALSE)
    {
        Carcasses[NumCarcasses] = carcassName;
        NumCarcasses++;
    }

    unguard;
}

/*-----------------------------------------------------------------------------
    Agitation / fear decay.
-----------------------------------------------------------------------------*/

void AScriptedPawn::UpdateAgitation(FLOAT deltaSeconds)
{
    guard(AScriptedPawn::UpdateAgitation);

    if (AgitationCheckTimer > 0.0f)
    {
        AgitationCheckTimer -= deltaSeconds;
        if (AgitationCheckTimer < 0.0f)
            AgitationCheckTimer = 0.0f;
    }

    FLOAT DecayAmount = 0.0f;

    if (AgitationTimer <= 0.0f)
    {
        DecayAmount = deltaSeconds * AgitationDecayRate;
    }
    else if (AgitationTimer >= deltaSeconds)
    {
        AgitationTimer -= deltaSeconds;
    }
    else
    {
        FLOAT RemainingFraction = AgitationTimer / deltaSeconds;
        AgitationTimer = 0.0f;
        DecayAmount = (1.0f - RemainingFraction) * (deltaSeconds * AgitationDecayRate);
    }

    if (bAlliancesChanged == TRUE && DecayAmount > 0.0f)
    {
        bAlliancesChanged = FALSE;

        for (INT iAlliance = 15; iAlliance >= 0; iAlliance--)
        {
            FAllianceInfoEx& Info = AlliancesEx[iAlliance];

            if (Info.AllianceName != NAME_None && Info.bPermanent == FALSE && Info.AgitationLevel > 0.0f)
            {
                bAlliancesChanged = TRUE;
                Info.AgitationLevel -= DecayAmount;

                if (Info.AgitationLevel < 0.0f)
                    Info.AgitationLevel = 0.0f;
            }
        }
    }

    unguard;
}

void AScriptedPawn::UpdateFear(FLOAT deltaSeconds)
{
    guard(AScriptedPawn::UpdateFear);

    FLOAT DecayAmount = 0.0f;

    if (FearTimer <= 0.0f)
    {
        DecayAmount = deltaSeconds * FearDecayRate;
    }
    else if (FearTimer >= deltaSeconds)
    {
        FearTimer -= deltaSeconds;
        return;
    }
    else
    {
        FLOAT RemainingFraction = FearTimer / deltaSeconds;
        FearTimer = 0.0f;
        DecayAmount = (1.0f - RemainingFraction) * (deltaSeconds * FearDecayRate);
    }

    if (DecayAmount > 0.0f && FearLevel > 0.0f)
    {
        FearLevel -= DecayAmount;
        if (FearLevel < 0.0f)
            FearLevel = 0.0f;
    }

    unguard;
}

/*-----------------------------------------------------------------------------
    Tick.
-----------------------------------------------------------------------------*/

static void DeusExDecreaseTimer(FLOAT& Timer, FLOAT deltaSeconds)
{
    if (Timer > 0.0f)
    {
        Timer -= deltaSeconds;
        if (Timer < 0.0f)
            Timer = 0.0f;
    }
}

UBOOL AScriptedPawn::Tick(FLOAT deltaSeconds, ELevelTick tickType)
{
    guard(AScriptedPawn::Tick);

    UBOOL bRunDeusExTick = FALSE;

    if (RemoteRole != ROLE_AutonomousProxy && Role >= ROLE_SimulatedProxy && tickType != LEVELTICK_ViewportsOnly)
        bRunDeusExTick = TRUE;

    if (bAlwaysTick == TRUE)
        bRunDeusExTick = TRUE;

    if (bRunDeusExTick == TRUE)
    {
        FLOAT SecondsSinceLastRender = XLevel->TimeSeconds - LastRenderTime;
        if (SecondsSinceLastRender < 0.0f)
            SecondsSinceLastRender = 0.0f;

        if (bDisappear == TRUE)
        {
            if (InStasis() == TRUE || SecondsSinceLastRender > 5.0f)
            {
                XLevel->DestroyActor(this, FALSE);
                return TRUE;
            }
        }

        if (PrePivotTime > 0.0f)
        {
            if (deltaSeconds >= PrePivotTime)
            {
                PrePivot = DesiredPrePivot;
                PrePivotTime = 0.0f;
            }
            else
            {
                const FLOAT Alpha = deltaSeconds / PrePivotTime;
                PrePivot += (DesiredPrePivot - PrePivot) * Alpha;
                PrePivotTime -= deltaSeconds;
            }
        }

        UBOOL bTickVisibleWork = TRUE;
        if (bTickVisibleOnly == TRUE)
            bTickVisibleWork = DistanceFromPlayer <= 1200.0f;

        UpdateAgitation(deltaSeconds);
        UpdateFear(deltaSeconds);

        // Original ASM always subtracts DeltaSeconds from AlarmTimer first,
        // even if it was already negative, and only then clamps to zero.
        AlarmTimer -= deltaSeconds;
        if (AlarmTimer < 0.0f)
            AlarmTimer = 0.0f;

        if (Weapon != NULL)
            WeaponTimer += deltaSeconds;
        else if (WeaponTimer != 0.0f)
            WeaponTimer = 0.0f;

        DeusExDecreaseTimer(FireTimer, deltaSeconds);
        DeusExDecreaseTimer(SpecialTimer, deltaSeconds);

        if (ReloadTimer > 0.0f && Weapon != NULL)
            ReloadTimer -= deltaSeconds;
        else
            ReloadTimer = 0.0f;

        DeusExDecreaseTimer(AvoidWallTimer, deltaSeconds);
        DeusExDecreaseTimer(AvoidBumpTimer, deltaSeconds);
        DeusExDecreaseTimer(ObstacleTimer, deltaSeconds);
        DeusExDecreaseTimer(CloakEMPTimer, deltaSeconds);
        DeusExDecreaseTimer(TakeHitTimer, deltaSeconds);
        DeusExDecreaseTimer(CarcassCheckTimer, deltaSeconds);

        if (PotentialEnemyTimer > 0.0f)
        {
            PotentialEnemyTimer -= deltaSeconds;
            if (PotentialEnemyTimer <= 0.0f)
            {
                PotentialEnemyTimer = 0.0f;
                PotentialEnemyAlliance = NAME_None;
            }
        }

        DeusExDecreaseTimer(BeamCheckTimer, deltaSeconds);
        DeusExDecreaseTimer(FutzTimer, deltaSeconds);
        DeusExDecreaseTimer(PlayerAgitationTimer, deltaSeconds);

        if (DistressTimer >= 0.0f)
        {
            DistressTimer += deltaSeconds;
            if (DistressTimer > FearSustainTime)
                DistressTimer = -1.0f;
        }

        if (bHasCloak == TRUE)
        {
            // UnrealScript bool parameters are carried as bitfields in generated
            // native parameter structs.  The original ASM writes only the low
            // bit of this local parameter block before ProcessEvent().
            struct FEnableCloakParms
            {
                BITFIELD bEnable:1 GCC_PACK(4);
            } Parms;

            Parms.bEnable = FALSE;
            if (Health <= CloakThreshold)
                Parms.bEnable = TRUE;

            ProcessEvent(FindFunctionChecked(TEXT("EnableCloak")), &Parms);
        }

        if (bFacingTarget == TRUE)
        {
            if ((Acceleration == FVector(0.0f, 0.0f, 0.0f)) || Physics != PHYS_Walking || TurnDirection == 0)
            {
                bFacingTarget = FALSE;

                if (TurnDirection != 0)
                    MoveTimer -= 4.0f;

                ActorAvoiding = NULL;
                NextDirection = 0;
                TurnDirection = 0;
                bClearedObstacle = TRUE;
                ObstacleTimer = 0.0f;
            }
        }

        if (bOnFire == TRUE)
        {
            burnTimer += deltaSeconds;
            if (burnTimer >= BurnPeriod)
                ProcessEvent(FindFunctionChecked(TEXT("ExtinguishFire")), NULL);
        }

        if (bTickVisibleWork == TRUE)
        {
            if (BleedRate > 0.0f && bCanBleed == TRUE)
            {
                FLOAT EffectiveBleedRate = BleedRate;
                if (EffectiveBleedRate < 0.0f)
                    EffectiveBleedRate = 0.0f;
                if (EffectiveBleedRate > 1.0f)
                    EffectiveBleedRate = 1.0f;

                FLOAT BloodIntervalBase = 1.0f - EffectiveBleedRate + 0.1f;
                FLOAT Speed = appSqrt(Velocity.X * Velocity.X + Velocity.Y * Velocity.Y + Velocity.Z * Velocity.Z);
                FLOAT SpeedFactor = Speed * 0.001953125f;

                if (SpeedFactor < 0.05f)
                    SpeedFactor = 0.05f;
                if (SpeedFactor > 1.0f)
                    SpeedFactor = 1.0f;

                FLOAT BloodInterval = BloodIntervalBase / SpeedFactor;

                DropCounter += deltaSeconds;
                while (DropCounter >= BloodInterval)
                {
                    ProcessEvent(FindFunctionChecked(TEXT("SpurtBlood")), NULL);
                    DropCounter -= BloodInterval;
                }

                BleedRate -= deltaSeconds / ClotPeriod;
            }

            if (BleedRate <= 0.0f)
            {
                DropCounter = 0.0f;
                BleedRate = 0.0f;
            }
        }
    }

    return AActor::Tick(deltaSeconds, tickType);

    unguard;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
