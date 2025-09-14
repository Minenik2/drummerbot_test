# Drummerbot in Unreal engine 5.6
A prototype of a drummer bot in unreal engine. Currently very barebone functionality.

Drum models are made by me in blender and imported into unreal as FBX.

## Features
- Pressing ASDF buttons to trigger kick/snare/cymbal/hi-hat
- When the drum is triggered it plays an animation scaling itself up and down as feedback
  - It also plays it's corresponding .wav sound

## Currently working on
A midi feature to be able to import and play midi files

## Using
- Unreal engine 5.6
  - [Enhanced Input for keyboard implementation](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)
  - [MIDI Device Support for midi implementation](https://dev.epicgames.com/documentation/en-us/unreal-engine/midi-in-unreal-engine)


## MIDI_Manager.cpp explained
(This explanation was run true chatgpt to make it sound more coherent)

This class is essentially a file reader and playback manager for .mid (MIDI) files. It leverages the Unreal Engine function FFileHelper::LoadFileToArray() to read an entire file into a byte array (`TArray<uint8>`).

MIDI files are binary files, and their contents are structured as a series of bytes. In hex view, each pair of characters (e.g., 4D, 54, 68) represents one byte (8 bits). The MIDI protocol defines how these bytes must be interpreted by file readers and sequencers.

![image](https://miro.medium.com/v2/resize:fit:1280/1*4KjU9nsZjejIuhn0I3AEPg.gif)

### MIDI File Structure
A standard MIDI file has three main sections:
1. File Header (MThd)
    - Always 14 bytes long.
    - Identifies the file as a MIDI file.
    - Contains information such as format type, number of tracks, and the time division (ticks per quarter note).
2. Track Chunks (MTrk)
    - Each track has a header (MTrk) followed by a 32-bit integer indicating the track length (in bytes).
    - Contains a sequence of MIDI events (note on, note off, tempo changes, etc.), each preceded by a delta-time (timestamp).
3. Track Trailer
    - Marks the end of track events.

### MIDI Events
- Delta-Time
  - Stored as a variable-length quantity (VLQ).
  - Indicates the time (in ticks) between events.
- Note On (0x90)
  - Tells the synthesizer to start playing a note. (In our case it will emit the OnNoteOn with both the played note integer and velocity to the class, this can then be further expanded in BP_MIDI_Manager via blueprints)
  - Format: 0x90 | channel, note, velocity. (Simply it will emit the saved pressed node ID between 0 and 127 and velocity also between 0 and 127)
  - If velocity = 0, it is interpreted as a Note Off.
- Note Off (0x80)
  - Tells the synthesizer to stop playing a note.
  - Format: 0x80 | channel, note, velocity.
- Meta Events (0xFF)
  - Provide non-MIDI data, such as tempo changes.
  - Tempo event (0xFF 0x51) specifies microseconds per quarter note.
- Running Status
  - Optimization where repeated events omit the status byte; the last one is reused.

### Code Breakdown
- `LoadMIDI()`
  - Loads file into memory.
  - Validates MThd header.
  - Reads format type, number of tracks, and division (ticks per quarter note).
  - Iterates through each track and calls ParseTrack().
- `ParseTrack()`
  - Walks through all bytes in a track.
  - Reads delta-times using ReadVariableLength().
  - Updates absolute playback time (Time += Delta * SecsPerTick).
  - Parses MIDI events:
    - **Note On / Note Off** events are converted into `FMIDIEvent` structs and stored in `MIDIEvents`.  
    - **Tempo changes** recalculate `SecsPerTick`.  
    - Unsupported events are skipped.
  - Handles **running status** if the status byte is omitted.  
- `ReadVariableLength()`
  - Reads variable-length delta-time values.  
  - Each byte uses the highest bit as a “continue” flag.  
- Playback Methods
  - `StartPlayback()` resets state and begins playback.  
  - `Tick()` increments playback time and dispatches events (`OnNoteOn`, `OnNoteOff`) when their timestamps are reached.  
  - `StopPlayback()` stops playback.  

### Key Clarifications
- `0x90` with **velocity > 0** = **Note On**  
- `0x90` with **velocity = 0** = **Note Off** (optimization used by many MIDI files).  
- **Tempo (`0xFF 0x51`)** events adjust playback speed dynamically.  
- All parsing is based on **delta-times**, not absolute times — events are cumulative.  
- The implementation assumes a default tempo of **120 BPM** until a tempo meta-event is encountered.  
