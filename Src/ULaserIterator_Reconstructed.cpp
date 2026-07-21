/*=============================================================================
    ULaserIterator_Reconstructed.cpp
    DeusEx.dll reconstruction pass 26.

    ASM-aligned against:
    DeusEx_dll_proc_00458_qCurrentItem_ULaserIterator_UAEPAVAActor_XZ.asm

    Pass26 changes:
    - MoveActor() helper now constructs FCheckResult(1.0f), matching the ASM
      constructor layout: Next=NULL, Actor=NULL, Location/Normal zero,
      Primitive=NULL, Time=1.0, Item=INDEX_NONE.  The old appMemzero+Time=1
      left Item as 0, which was not original.
    - savedLoc lottery now compares savedLoc against Outer actor Location, not
      proxy Location.  ASM reads this+0x18 and compares against Actor.Location.
    - savedLoc probability division no longer has a MaxItems zero guard; the
      original x87 code divides nextItem by MaxItems directly.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

/*-----------------------------------------------------------------------------
    Helpers.
-----------------------------------------------------------------------------*/

static void DeusExLaserMoveProxy(AActor* Proxy, const FVector& NewLocation)
{
    // Original ASM reaches proxy->XLevel directly after the proxy NULL check
    // at the top of CurrentItem().
    Proxy->XLevel->FarMoveActor(Proxy, NewLocation, FALSE, FALSE);
}

static void DeusExLaserRotateProxy(AActor* Proxy, const FRotator& NewRotation)
{
    // Original ASM builds the same layout as FCheckResult(1.0f): two NULL
    // iterator/actor pointers, zero vectors, NULL primitive, Time=1.0,
    // Item=INDEX_NONE.  Do not replace this with appMemzero+Time=1.
    FCheckResult Hit(1.0f);
    Proxy->XLevel->MoveActor(Proxy, FVector(0.0f, 0.0f, 0.0f), NewRotation, Hit, FALSE, FALSE, FALSE, FALSE);
}

/*-----------------------------------------------------------------------------
    ULaserIterator.
-----------------------------------------------------------------------------*/

AActor* ULaserIterator::CurrentItem()
{
    guard(ULaserIterator::CurrentItem);

    if (proxy == NULL)
        return Super::CurrentItem();

    INT SegmentLimit = 0;
    INT BeamIndex = 0;
    UBOOL bFoundBeam = FALSE;

    for (INT iBeam = 0; iBeam < MAX_BEAMS; iBeam++)
    {
        if (Beams[iBeam].bActive == TRUE)
        {
            SegmentLimit += Beams[iBeam].numSegments;
            if (Index < SegmentLimit)
            {
                BeamIndex = iBeam;
                bFoundBeam = TRUE;
                break;
            }
        }
    }

    // Original ASM initializes BeamIndex to zero and falls through if the scan
    // fails.  That should only happen if MaxItems/Index are already inconsistent.
    if (bFoundBeam == FALSE)
        BeamIndex = 0;

    sBeam& Beam = Beams[BeamIndex];

    FLOAT SegmentAlpha = 1.0f - (FLOAT)(SegmentLimit - Index) / (FLOAT)Beam.numSegments;
    if (SegmentAlpha < 0.0f)
        SegmentAlpha = 0.0f;
    if (SegmentAlpha > 1.0f)
        SegmentAlpha = 1.0f;

    FCoords BeamCoords = GMath.UnitCoords / Beam.Rotation;
    FVector BeamEnd = Beam.Location + BeamCoords.XAxis * Beam.Length;
    FVector SegmentLocation = Beam.Location + (BeamEnd - Beam.Location) * SegmentAlpha;
    FRotator SegmentRotation = Beam.Rotation;

    if (bRandomBeam == TRUE)
    {
        FVector RandVector;

        do
        {
            RandVector.X = appFrand() * 2.0f - 1.0f;
            RandVector.Y = appFrand() * 2.0f - 1.0f;
            RandVector.Z = appFrand() * 2.0f - 1.0f;
        }
        while ((RandVector | RandVector) > 1.0f);

        FVector RandDir = RandVector.UnsafeNormal();
        FVector RandomLocation = SegmentLocation + prevRand + RandDir;
        FVector ReflectedLocation = RandomLocation + (RandomLocation - prevLoc);

        SegmentRotation = (prevLoc - ReflectedLocation).Rotation();

        prevLoc = ReflectedLocation;
        prevRand = RandDir;
        SegmentLocation = RandomLocation;
    }

    DeusExLaserMoveProxy(proxy, SegmentLocation);
    DeusExLaserRotateProxy(proxy, SegmentRotation);

    // ASM compares savedLoc with this->Outer interpreted as an Actor Location
    // (this+0x18 -> Actor.Location), not with the moving proxy actor.
    AActor* OwnerActor = (AActor*)GetOuter();
    if (savedLoc == OwnerActor->Location)
    {
        FLOAT SaveChance = (FLOAT)nextItem / (FLOAT)MaxItems;

        if (SaveChance > appFrand())
        {
            savedLoc = SegmentLocation;
            savedRot = SegmentRotation;
        }
    }

    nextItem++;
    if (nextItem == MaxItems)
    {
        DeusExLaserMoveProxy(proxy, savedLoc);
        DeusExLaserRotateProxy(proxy, savedRot);
    }

    return proxy;

    unguard;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
