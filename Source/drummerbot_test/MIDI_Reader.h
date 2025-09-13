// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FMidiEvent
{
    float TimeSeconds;   // When to play, relative to start
    int32 Note;          // MIDI note number
    bool bIsNoteOn;      // true = Note On, false = Note Off
    int32 Velocity;      // Velocity (0–127)
};

/**
 * 
 */
class DRUMMERBOT_TEST_API MIDI_Reader
{
public:
    MIDI_Reader();
    ~MIDI_Reader();

    // Load and parse a MIDI file
    bool LoadMIDIFile(const FString& FilePath);

    // Access parsed events
    const TArray<FMidiEvent>& GetEvents() const { return Events; }

private:
    TArray<FMidiEvent> Events;

    // Helpers
    uint32 ReadUInt32BE(const uint8* Data);
    uint16 ReadUInt16BE(const uint8* Data);
    int32 ReadVariableLength(const TArray<uint8>& Data, int32& Offset);
};
