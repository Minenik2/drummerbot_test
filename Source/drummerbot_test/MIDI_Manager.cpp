#include "MIDI_Manager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

AMIDI_Manager::AMIDI_Manager()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsPlaying = false;
	PlaybackTime = 0.f;
	CurrentEventIndex = 0;
}

void AMIDI_Manager::BeginPlay()
{
	Super::BeginPlay();
}

void AMIDI_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPlaying && MIDIEvents.Num() > 0)
	{
		PlaybackTime += DeltaTime;

		while (CurrentEventIndex < MIDIEvents.Num() &&
			   MIDIEvents[CurrentEventIndex].TimeSec <= PlaybackTime)
		{
			const FMIDIEvent &Event = MIDIEvents[CurrentEventIndex];
			uint8 Status = Event.StatusByte & 0xF0;

			if (Status == 0x90 && Event.Data2 > 0) // Note On
			{
				OnNoteOn(Event.Data1, Event.Data2);
			}
			else if (Status == 0x80 || (Status == 0x90 && Event.Data2 == 0)) // Note Off
			{
				OnNoteOff(Event.Data1);
			}

			CurrentEventIndex++;
		}
	}
}

void AMIDI_Manager::LoadMIDI()
{
	MIDIEvents.Empty();

	if (MIDIFilePath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MIDI file path is empty!"));
		return;
	}

	FString AbsolutePath = FPaths::ConvertRelativePathToFull(MIDIFilePath);
	TArray<uint8> FileData;

	if (!FFileHelper::LoadFileToArray(FileData, *AbsolutePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load MIDI file at %s"), *AbsolutePath);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("MIDI file loaded, size: %d bytes"), FileData.Num());

	// Check header
	if (FileData.Num() < 14 || !(FileData[0] == 'M' && FileData[1] == 'T' && FileData[2] == 'h' && FileData[3] == 'd'))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid MIDI file header"));
		return;
	}

	int16 FormatType = (FileData[8] << 8) | FileData[9];
	int16 NumTracks = (FileData[10] << 8) | FileData[11];
	int16 Division = (FileData[12] << 8) | FileData[13];

	UE_LOG(LogTemp, Log, TEXT("MIDI Format: %d, Tracks: %d, Division: %d"), FormatType, NumTracks, Division);

	int32 Index = 14;

	// Parse each track
	for (int t = 0; t < NumTracks; t++)
	{
		if (Index + 8 >= FileData.Num())
			break;

		if (!(FileData[Index] == 'M' && FileData[Index + 1] == 'T' && FileData[Index + 2] == 'r' && FileData[Index + 3] == 'k'))
		{
			UE_LOG(LogTemp, Error, TEXT("Track %d missing MTrk header"), t);
			return;
		}

		int32 TrackLength = (FileData[Index + 4] << 24) | (FileData[Index + 5] << 16) | (FileData[Index + 6] << 8) | FileData[Index + 7];
		Index += 8;

		ParseTrack(FileData, Index, TrackLength, Division);
		Index += TrackLength;
	}

	UE_LOG(LogTemp, Log, TEXT("Parsed %d MIDI events"), MIDIEvents.Num());
}

void AMIDI_Manager::ParseTrack(const TArray<uint8> &Data, int32 StartIndex, int32 TrackLength, int32 Division)
{
	int32 Index = StartIndex;
	int32 EndIndex = StartIndex + TrackLength;

	float Time = 0.f;
	float SecsPerTick = (60.f / 120.f) / Division; // Default 120 BPM

	uint8 RunningStatus = 0;

	while (Index < EndIndex)
	{
		int32 Delta = ReadVariableLength(Data, Index);
		Time += Delta * SecsPerTick;

		uint8 StatusByte = Data[Index];
		if (StatusByte < 0x80)
		{
			// Running status (reuse last status)
			StatusByte = RunningStatus;
		}
		else
		{
			Index++;
			RunningStatus = StatusByte;
		}

		uint8 EventType = StatusByte & 0xF0;

		if (EventType == 0x90 || EventType == 0x80) // Note On / Off
		{
			uint8 Note = Data[Index++];
			uint8 Velocity = Data[Index++];

			FMIDIEvent E;
			E.TimeSec = Time;
			E.StatusByte = StatusByte;
			E.Data1 = Note;
			E.Data2 = Velocity;
			MIDIEvents.Add(E);
		}
		else if (StatusByte == 0xFF) // Meta event
		{
			uint8 Type = Data[Index++];
			int32 Length = ReadVariableLength(Data, Index);
			if (Type == 0x51 && Length == 3) // Tempo
			{
				int32 MicroPerQuarter = (Data[Index] << 16) | (Data[Index + 1] << 8) | Data[Index + 2];
				float BPM = 60000000.f / MicroPerQuarter;
				SecsPerTick = (60.f / BPM) / Division;
				UE_LOG(LogTemp, Log, TEXT("Tempo change: %f BPM"), BPM);
			}
			Index += Length;
		}
		else
		{
			// Skip unsupported events
			if (Index < EndIndex)
				Index++;
		}
	}
}

int32 AMIDI_Manager::ReadVariableLength(const TArray<uint8> &Data, int32 &Index)
{
	int32 Value = 0;
	uint8 Byte;
	do
	{
		Byte = Data[Index++];
		Value = (Value << 7) | (Byte & 0x7F);
	} while (Byte & 0x80);
	return Value;
}

void AMIDI_Manager::StartPlayback()
{
	if (MIDIEvents.Num() == 0)
		return;
	bIsPlaying = true;
	PlaybackTime = 0.f;
	CurrentEventIndex = 0;
	UE_LOG(LogTemp, Log, TEXT("MIDI playback started"));
}

void AMIDI_Manager::StopPlayback()
{
	bIsPlaying = false;
	UE_LOG(LogTemp, Log, TEXT("MIDI playback stopped"));
}
