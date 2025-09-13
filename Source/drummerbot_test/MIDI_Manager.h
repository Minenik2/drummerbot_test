// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MIDI_Manager.generated.h"

USTRUCT(BlueprintType)
struct FMIDIEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 DeltaTime;

	UPROPERTY(BlueprintReadOnly)
	uint8 StatusByte;

	UPROPERTY(BlueprintReadOnly)
	uint8 Data1;

	UPROPERTY(BlueprintReadOnly)
	uint8 Data2;
};

UCLASS()
class DRUMMERBOT_TEST_API AMIDI_Manager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMIDI_Manager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Path to the MIDI file (set in details panel in editor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MIDI")
	FString MIDIFilePath;

	// Parsed MIDI events
	UPROPERTY(BlueprintReadOnly, Category = "MIDI")
	TArray<FMIDIEvent> MIDIEvents;

	// Blueprint-callable function to load and parse MIDI
	UFUNCTION(BlueprintCallable, Category = "MIDI")
	void LoadMIDI();
};
