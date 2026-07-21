# Deus Ex DeusEx.dll — Reconstructed Source

This repository contains an independently reconstructed source implementation of the original **Deus Ex `DeusEx.dll`**, produced through AI-assisted reverse engineering.

`DeusEx.dll` contains a significant part of the native game-specific implementation used by Deus Ex on top of Unreal Engine 1.

The project is intended for **preservation, research, modding, compatibility work, debugging, engine study, and further development of Deus Ex**.

This is **not** an official Ion Storm, Eidos Interactive, Epic Games, or Square Enix source release, and this project is not affiliated with or endorsed by any of those companies.

## Reconstruction

The source code was reconstructed by analyzing the original retail `DeusEx.dll` binary and the surrounding Deus Ex / Unreal Engine 1 environment.

The reconstruction process used, among other things:

* decompiler output;
* disassembly / assembly code;
* original and publicly available SDK headers;
* exported Unreal class headers;
* UnrealScript classes;
* native function declarations;
* Unreal Engine 1 object and reflection information;
* runtime behavior of the original game;
* comparison against the original retail `DeusEx.dll`.

A substantial portion of the reconstruction was performed with AI assistance.

The resulting code was then manually inspected, compiled, tested, and corrected where differences from the original binary behavior were discovered.

**No leaked or proprietary original `DeusEx.dll` source code was used in this reconstruction.**

This source should therefore not be considered the original Ion Storm implementation. It is an independently produced reconstruction intended to reproduce the functionality and behavior of the original binary as accurately as practical.

## Status

The reconstructed source is a work in progress.

Large portions of the original native implementation have been recovered and made buildable, but reconstruction accuracy may vary between individual classes and functions.

Some functions may still contain:

* imperfect reconstruction;
* implementation differences;
* incorrect edge-case behavior;
* missing fidelity details;
* assumptions derived from decompiler output;
* behavior that requires additional comparison against the original binary.

Testing and comparison against the original retail `DeusEx.dll` remain an important part of the project.

Bug reports, additional reverse-engineering findings, behavioral comparisons, and reconstruction improvements are welcome.

## Purpose

The immediate goal of this repository is to recover and document the native Deus Ex game code in a form that can be understood, compiled, tested, modified, and improved.

Possible uses include:

* studying the internal implementation of Deus Ex;
* fixing bugs in native game code;
* improving compatibility with modern systems;
* supporting Unreal Engine 1 and Deus Ex modding;
* documenting previously undocumented native behavior;
* comparing reconstructed code against the original binaries;
* removing historical limitations where practical;
* providing a foundation for future engine and game modernization work.

## Original game requirement

This repository does not provide the original Deus Ex game.

A legally obtained copy of Deus Ex is required for normal use, testing, and comparison against the original implementation.

Original maps, textures, sounds, music, packages, and other proprietary game assets are not intended to be redistributed as part of this reconstruction project.

## Native SDK dependencies

Building the reconstructed source requires headers, libraries, and other development files compatible with the original Deus Ex / Unreal Engine 1 native environment.

These dependencies are separate from the reconstructed `DeusEx.dll` implementation itself.

Refer to the repository build configuration and project files for the currently required SDK layout.

## AI-assisted reverse engineering

AI tools were used extensively during reconstruction to help analyze:

* decompiler output;
* assembly code;
* calling conventions;
* Unreal Engine object structures;
* relationships between native and UnrealScript code;
* possible high-level equivalents of reconstructed machine code.

AI-generated reconstruction was not treated as authoritative.

The resulting code has been subject to manual review, compilation, runtime testing, binary comparison, and iterative correction.

Where reconstructed behavior differs from the original game, the original retail binary is considered the behavioral reference.

## Preservation

A major goal of this project is to prevent technical knowledge about Deus Ex from being lost or remaining confined to private source trees.

The reconstructed source is intentionally made publicly available so that other developers and researchers can:

* study it;
* verify it;
* find reconstruction mistakes;
* improve it;
* build upon it;
* preserve it;
* continue development even if the original reconstruction work eventually stops.

Public availability is an important part of the preservation effort.

A closed implementation can disappear with its original author. A publicly documented and version-controlled reconstruction can continue to be examined and improved by the community.

## Long-term direction

This repository is part of a broader effort to reconstruct the native components of the original Deus Ex release.

A possible long-term objective is to recover enough of the original native codebase to allow deeper modernization work, including:

* improved compatibility with modern operating systems;
* removal of legacy engine limitations;
* native bug fixes;
* modern compiler support;
* architecture cleanup;
* possible 64-bit ports;
* new rendering or platform capabilities;
* continued development of Deus Ex beyond the limitations of the original binaries.

These are long-term possibilities rather than guarantees.

The immediate priority remains accurate reconstruction and preservation of the original native implementation.

## Contributions

Contributions that improve reconstruction accuracy, compatibility, documentation, or behavioral fidelity are welcome.

When changing reconstructed code, comparison against the original retail binary and original game behavior is strongly encouraged.

Where possible, reconstruction fixes should document why the previous implementation was incorrect and how the corrected behavior was determined.

## Disclaimer

This source is provided **as-is**, without warranty of any kind.

Reverse engineering and reconstructed implementations may be subject to different laws and license terms depending on jurisdiction and circumstances.

Users and contributors are responsible for ensuring that their use of this project complies with applicable laws and agreements.

Again, this repository contains an **independently reconstructed implementation**, not the original proprietary Ion Storm source code.
