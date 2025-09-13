// Fill out your copyright notice in the Description page of Project Settings.

#include "MIDI_Reader.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

MIDI_Reader::MIDI_Reader() {}
MIDI_Reader::~MIDI_Reader() {}

uint32 MIDI_Reader::ReadUInt32BE(const uint8 *Data)
{
    return (Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3];
}

uint16 MIDI_Reader::ReadUInt16BE(const uint8 *Data)
{
    return (Data[0] << 8) | Data[1];
}

int32 MIDI_Reader::ReadVariableLength(const TArray<uint8> &Data, int32 &Offset)
{
    int32 Value = 0;
    uint8 Byte;
    do
    {
        Byte = Data[Offset++];
        Value = (Value << 7) | (Byte & 0x7F);
    } while (Byte & 0x80);
    return Value;
}

bool MIDI_Reader::LoadMIDIFile(const FString &FilePath)
{
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load MIDI file: %s"), *FilePath);
        return false;
    }

    int32 Offset = 0;

    // Parse header chunk
    if (FMemory::Memcmp(&FileData[Offset], "MThd", 4) != 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid MIDI header"));
        return false;
    }
    Offset += 4;

    uint32 HeaderLength = ReadUInt32BE(&FileData[Offset]);
    Offset += 4;
    uint16 Format = ReadUInt16BE(&FileData[Offset]);
    Offset += 2;
    uint16 NumTracks = ReadUInt16BE(&FileData[Offset]);
    Offset += 2;
    uint16 Division = ReadUInt16BE(&FileData[Offset]);
    Offset += 2;

    UE_LOG(LogTemp, Log, TEXT("MIDI Header: Format=%d, Tracks=%d, Division=%d"), Format, NumTracks, Division);

    // Skip any extra header data
    Offset += HeaderLength - 6;

    // Only handle Format 0 for now
    if (Format != 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Only Format 0 supported right now"));
        return false;
    }

    // Parse track chunk
    if (FMemory::Memcmp(&FileData[Offset], "MTrk", 4) != 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid Track header"));
        return false;
    }
    Offset += 4;

    uint32 TrackLength = ReadUInt32BE(&FileData[Offset]);
    Offset += 4;
    int32 TrackEnd = Offset + TrackLength;

    int32 TicksElapsed = 0;
    float Tempo = 500000.0f; // default 120 bpm (500,000 µs per quarter note)

    while (Offset < TrackEnd)
    {
        // Delta time
        int32 DeltaTicks = ReadVariableLength(FileData, Offset);
        TicksElapsed += DeltaTicks;

        uint8 Status = FileData[Offset++];

        if ((Status & 0xF0) == 0x90) // Note On
        {
            uint8 Note = FileData[Offset++];
            uint8 Velocity = FileData[Offset++];

            if (Velocity > 0)
            {
                float Seconds = (TicksElapsed * Tempo) / (Division * 1000000.0f);
                FMidiEvent Event{Seconds, Note, true, Velocity};
                Events.Add(Event);

                UE_LOG(LogTemp, Log, TEXT("Note On: %d at %f sec"), Note, Seconds);
            }
            else
            {
                // Velocity 0 = Note Off
                float Seconds = (TicksElapsed * Tempo) / (Division * 1000000.0f);
                FMidiEvent Event{Seconds, Note, false, 0};
                Events.Add(Event);

                UE_LOG(LogTemp, Log, TEXT("Note Off: %d at %f sec"), Note, Seconds);
            }
        }
        else if ((Status & 0xF0) == 0x80) // Note Off
        {
            uint8 Note = FileData[Offset++];
            uint8 Velocity = FileData[Offset++];

            float Seconds = (TicksElapsed * Tempo) / (Division * 1000000.0f);
            FMidiEvent Event{Seconds, Note, false, Velocity};
            Events.Add(Event);

            UE_LOG(LogTemp, Log, TEXT("Note Off: %d at %f sec"), Note, Seconds);
        }
        else if (Status == 0xFF) // Meta event
        {
            uint8 MetaType = FileData[Offset++];
            int32 Length = ReadVariableLength(FileData, Offset);

            if (MetaType == 0x51 && Length == 3)
            {
                // Set Tempo
                uint32 MicroPerQuarter = (FileData[Offset] << 16) | (FileData[Offset + 1] << 8) | FileData[Offset + 2];
                Tempo = (float)MicroPerQuarter;
                UE_LOG(LogTemp, Log, TEXT("Tempo change: %f µs/qn"), Tempo);
            }

            Offset += Length;
        }
        else
        {
            // Skip unknown events (just read their data length appropriately)
            UE_LOG(LogTemp, Warning, TEXT("Unhandled MIDI event: 0x%02X"), Status);
            // For simplicity, assume 2 data bytes
            Offset += 2;
        }
    }

    return true;
}
