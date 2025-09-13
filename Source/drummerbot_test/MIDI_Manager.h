#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MIDI_Manager.generated.h"

USTRUCT(BlueprintType)
struct FMIDIEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float TimeSec; // When to play (seconds from start)

	UPROPERTY(BlueprintReadOnly)
	uint8 StatusByte;

	UPROPERTY(BlueprintReadOnly)
	uint8 Data1; // Note number

	UPROPERTY(BlueprintReadOnly)
	uint8 Data2; // Velocity
};

UCLASS()
class DRUMMERBOT_TEST_API AMIDI_Manager : public AActor
{
	GENERATED_BODY()

public:
	AMIDI_Manager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MIDI")
	FString MIDIFilePath;

	UPROPERTY(BlueprintReadOnly, Category = "MIDI")
	TArray<FMIDIEvent> MIDIEvents;

	UFUNCTION(BlueprintCallable, Category = "MIDI")
	void LoadMIDI();

	UFUNCTION(BlueprintCallable, Category = "MIDI")
	void StartPlayback();

	UFUNCTION(BlueprintCallable, Category = "MIDI")
	void StopPlayback();

	// Called from C++ when a Note On is reached
	UFUNCTION(BlueprintImplementableEvent, Category = "MIDI")
	void OnNoteOn(int32 Note, int32 Velocity);

	// Called from C++ when a Note Off is reached
	UFUNCTION(BlueprintImplementableEvent, Category = "MIDI")
	void OnNoteOff(int32 Note);

private:
	bool bIsPlaying;
	float PlaybackTime;
	int32 CurrentEventIndex;

	void ParseTrack(const TArray<uint8> &Data, int32 StartIndex, int32 TrackLength, int32 Division);
	int32 ReadVariableLength(const TArray<uint8> &Data, int32 &Index);
};
