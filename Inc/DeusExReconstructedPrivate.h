/*=============================================================================
	DeusExReconstructedPrivate.h
	VC98 compile-trial umbrella for reconstructed DeusEx.dll.

	Do not put gameplay logic here.  This file only supplies cross-package type
	visibility that original DeusEx source got from the old build environment.
=============================================================================*/
#ifndef DEUSEX_RECONSTRUCTED_PRIVATE_H
#define DEUSEX_RECONSTRUCTED_PRIVATE_H

#ifndef DEUSEX_API
#define DEUSEX_API DLL_EXPORT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// DeusEx.h itself has some includes before its own include guard.  Do not
// include it through /FI and then include it again from every .cpp; that causes
// the generated classes to be parsed twice.  Every reconstructed .cpp includes
// this umbrella directly, so original headers are still left untouched.
#include "DeusEx.h"

// Native/noexport helper classes that are not declared by DeusExClasses.h.
#include "DeusExGameEngine.h"
#include "ExtGameDirectory.h"
#include "UDumpLocation.h"
#include "ULaserIterator.h"

// Cross-package conversation/history classes.
#include "ConSys.h"

#endif // DEUSEX_RECONSTRUCTED_PRIVATE_H
