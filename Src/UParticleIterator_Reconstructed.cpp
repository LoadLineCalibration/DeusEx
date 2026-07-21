/*=============================================================================
    UParticleIterator_Reconstructed.cpp
    DeusEx.dll reconstruction pass 13.

    Pass13 change:
    - execUpdateParticles() now matches the original tail more closely: after
      scanning all particles the proxy actor is dereferenced directly and its
      bHidden flag is set/cleared.  The earlier reconstruction-side NULL guard
      was safer, but not original.
=============================================================================

	Pass18 packaging note:
	- This is the canonical full-content source snapshot for moving to the next
	  dialogue. Previous pass suffixes were removed from filenames only; runtime
	  code remains based on the latest strict passes.
=============================================================================*/

#include "DeusExReconstructedPrivate.h"

/*-----------------------------------------------------------------------------
    UParticleIterator.
-----------------------------------------------------------------------------*/

void UParticleIterator::DeleteParticle(INT particleIndex)
{
    guard(UParticleIterator::DeleteParticle);

    if (particleIndex >= 0 && particleIndex < MAX_PARTICLES)
    {
        if (Particles[particleIndex].bActive == TRUE)
        {
            Particles[particleIndex].bActive = FALSE;
            nextFreeParticle = particleIndex;
        }
    }

    unguard;
}

AActor* UParticleIterator::CurrentItem()
{
    guard(UParticleIterator::CurrentItem);

    if (proxy == NULL)
        return Super::CurrentItem();

    if (Particles[Index].bActive == FALSE)
        return Super::CurrentItem();

    proxy->DrawScale = Particles[Index].DrawScale;
    proxy->ScaleGlow = Particles[Index].ScaleGlow;

    if (proxy->XLevel->FarMoveActor(proxy, Particles[Index].Location, FALSE, FALSE) == FALSE)
    {
        DeleteParticle(Index);
        return Super::CurrentItem();
    }

    return proxy;

    unguard;
}

void UParticleIterator::execUpdateParticles(FFrame& Stack, RESULT_DECL)
{
    guard(UParticleIterator::execUpdateParticles);

    P_GET_FLOAT(deltaSeconds);
    P_FINISH;

    UBOOL bAllParticlesDead = TRUE;

    for (INT iParticle = 0; iParticle < MAX_PARTICLES; iParticle++)
    {
        FsParticle& Particle = Particles[iParticle];

        if (Particle.bActive == TRUE)
        {
            if (Particle.LifeSpan > 0.0f)
            {
                Particle.LifeSpan -= deltaSeconds;
                if (Particle.LifeSpan <= 0.0f)
                    Particle.LifeSpan = -1.0f;
            }

            if (Particle.LifeSpan >= 0.0f)
            {
                bAllParticlesDead = FALSE;

                Particle.Velocity.X = Particle.initVel.X + 2.0f - appFrand() * 5.0f;
                Particle.Velocity.Y = Particle.initVel.Y + 2.0f - appFrand() * 5.0f;

                if (bOwnerUsesGravity == TRUE)
                    Particle.Velocity.Z += deltaSeconds * OwnerZoneGravity * 0.2f;
                else
                    Particle.Velocity.Z = Particle.initVel.Z + (appFrand() * 0.2f + 0.9f) * ((OwnerLifeSpan - Particle.LifeSpan) * OwnerRiseRate);

                if (bOwnerScales == TRUE)
                {
                    Particle.DrawScale = (OwnerLifeSpan - Particle.LifeSpan + 1.0f) * OwnerDrawScale;

                    if (Particle.DrawScale < 0.01f)
                        Particle.DrawScale = 0.01f;
                    if (Particle.DrawScale > 3.0f)
                        Particle.DrawScale = 3.0f;
                }
                else
                {
                    Particle.DrawScale = OwnerDrawScale;
                }

                if (bOwnerFades == TRUE)
                    Particle.ScaleGlow = Particle.LifeSpan / OwnerLifeSpan;
                else
                    Particle.ScaleGlow = 1.0f;

                Particle.Location.X += deltaSeconds * Particle.Velocity.X;
                Particle.Location.Y += deltaSeconds * Particle.Velocity.Y;
                Particle.Location.Z += deltaSeconds * Particle.Velocity.Z;
            }
            else
            {
                DeleteParticle(iParticle);
            }
        }
    }

    // Original ASM directly dereferences proxy at UParticleIterator+0xD38 here.
    // CurrentItem() has a proxy guard, but UpdateParticles() does not.
    if (bAllParticlesDead == TRUE)
        proxy->bHidden = TRUE;
    else
        proxy->bHidden = FALSE;

    unguardexec;
}

/*-----------------------------------------------------------------------------
    The End.
-----------------------------------------------------------------------------*/
