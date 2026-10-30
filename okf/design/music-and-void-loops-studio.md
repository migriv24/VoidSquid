---
type: Design
title: Music and Void Loops Studio
description: Whether Void Loops Studio can carry Void Squid's music now (no - it has no realtime engine yet and is in a feature freeze), what it can carry soon (game music as MIDI sequences in its node graph), and the concrete bridge found - Donkey Kong 64's music is the N64's compressed MIDI, convertible to the Standard MIDI Files VLS already reads.
tags: [status:planned, audience:dev, confidence:asserted]
timestamp: 2026-10-13T00:00:00Z
---

# Music and Void Loops Studio

The author, 2026-10-13: *"lets consider giving music to Void Loops Studio, and implementing stuff
from VLS into void squid. while void loops studio isn't nearly done at all, it has a decent amount of
progress… do some research into it to see if its suitable."*

## What Void Loops Studio is (read 2026-10-13)

A node-based DAW on Void Core: the project is a graph (sequencers, oscillators, samples, VSTs,
effects, an output), a Void Core mantle; the audio is computed by a separate engine that never
reads the model (its *realtime boundary*, the same law as Void Unity's two clocks).

- **Python prototype** (`voidloops/`, about 2,950 lines): v1 shipped 2026-07-08 (typed node graph,
  real VSTs through RemoteVstPlugin, looping sequences, a MIDI reader, an offline renderer). Now the
  reference implementation and its 16 test files the specification.
- **Native C++** (`native/`, about 850 lines, on Void Maiz): milestone 1 of 6, the shell. The
  streaming engine (miniaudio-class) is milestone 4.
- **Feature freeze since 2026-10-04**: catching up on Void Core 0.2.14, Void Maiz and LAN
  collaboration before new features.

## Suitable now? No, for playing; yes, for describing

| what Void Squid needs | VLS today |
|---|---|
| play a game's music in the runtime, looping, reacting | no realtime engine (milestone 4); the runtime keeps raylib's streams |
| describe music as structure (tracks, loops, stems, tempo, instruments) | yes: its model is exactly that, on Void Core |
| bring a game's sequenced music in as notes | yes: a `midi-in` node reads Standard MIDI Files |
| reactive game audio (a zone entered, a boss low) | designed in principle: model-rate events across its realtime boundary (Void Jelly's 2026-10-03 note) |

## The concrete bridge

Donkey Kong 64's pointer table 0 holds its music; every file starts with a 0x44-byte header of 16
track offsets and a division. That matches, as recalled, the N64 audio library's compressed MIDI
sequence (ALCSeq); converting it to a Standard MIDI File is a known, small job. Then every Donkey Kong
64 song is a `midi-in` node in a VLS patch, and the Feast's music could name a VLS project instead
of a WAV. Super Mario 64 DS and Mario Kart DS use the DS's SSEQ sequences, the same idea. Galaxy's
music is streamed audio (AST): it stays a stream.

## Step 1, built (2026-10-14)

`tools/dk64_music.py` (the exercise): 174 of Donkey Kong 64's sequences (table 0's entries 1–174) written as Standard MIDI
Files (format 1, division 384, note-offs from durations, loops as `loopStart`/`loopEnd` markers);
Void Loops Studio's own `read_midi_notes` opens every one; pitches centre on octaves 3–6, the median
note a third of a beat. Song 60 stops at an escape byte the format, as read, does not explain.
The N64's own sound banks (the instruments) are not converted: a MIDI file says which program, not
what it sounds like.

## The lean

1. Now: nothing linked. Squid writes game sequences as MIDI files into the person's workbench
   (DK64 done), and a music turkey says which.
2. When VLS's native compile pipeline exists (its milestone 3): a music turkey may point at a VLS
   project; the studio opens it in VLS (a process, as Play opens the runtime).
3. When VLS's streaming engine exists (milestone 4): the runtime sends it model-rate events, and the
   music reacts.

Asked of Void Loops Studio: `MESSAGE_FOR_VOIDLOOPSSTUDIO_voidsquid-game-music-is-midi-and-your-model-fits-2026-10-13.md`.
