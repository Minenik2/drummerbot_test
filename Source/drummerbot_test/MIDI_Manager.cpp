#include "MIDI_Manager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

// Sets default values
AMIDI_Manager::AMIDI_Manager()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMIDI_Manager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMIDI_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMIDI_Manager::LoadMIDI()
{
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

	// Simple parsing example (not full MIDI spec, just enough to demonstrate):
	// MIDI header should start with "MThd"
	if (FileData.Num() < 14 || !(FileData[0] == 'M' && FileData[1] == 'T' && FileData[2] == 'h' && FileData[3] == 'd'))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid MIDI file header"));
		return;
	}

	// Read header info
	int16 FormatType = (FileData[8] << 8) | FileData[9];
	int16 NumTracks = (FileData[10] << 8) | FileData[11];
	int16 Division = (FileData[12] << 8) | FileData[13];

	UE_LOG(LogTemp, Log, TEXT("MIDI Format: %d, Tracks: %d, Division: %d"), FormatType, NumTracks, Division);

	// Just as a test: store one fake event
	FMIDIEvent TestEvent;
	TestEvent.DeltaTime = 0;
	TestEvent.StatusByte = 0x90; // Note On, channel 1
	TestEvent.Data1 = 60;		 // Middle C
	TestEvent.Data2 = 100;		 // Velocity
	MIDIEvents.Add(TestEvent);

	UE_LOG(LogTemp, Log, TEXT("MIDI parsing stub complete. 1 test event added."));
}
