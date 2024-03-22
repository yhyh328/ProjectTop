// gachaGameMode.cpp

#include "gachaGameMode.h"
#include "gachaCharacter.h"
#include "Public/Item.h"
#include "Engine/DataTable.h"
#include <algorithm> // std::find_if 사용
#include "Engine/World.h"
#include "Runtime/LevelSequence/Public/LevelSequenceActor.h"
#include "Runtime/LevelSequence/Public/LevelSequencePlayer.h"
#include "Runtime/LevelSequence/Public/LevelSequence.h"
#include "UObject/ConstructorHelpers.h"

AgachaGameMode::AgachaGameMode()
{
	// set default pawn class to our Blueprinted character
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTableAsset(TEXT("/Game/ThirdPerson/DrawTable"));
    DrawResultTable = DataTableAsset.Object;
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AgachaGameMode::PlaySequence(const FString& SequencePath)
{
    UObject* LevelSequenceObject = LoadObject<ULevelSequence>(nullptr, *SequencePath);
    if (!LevelSequenceObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("LevelSequence could not be loaded: %s"), *SequencePath);
        return;
    }

    ULevelSequence* LevelSequence = Cast<ULevelSequence>(LevelSequenceObject);
    if (!LevelSequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to cast to ULevelSequence: %s"), *SequencePath);
        return;
    }


    UWorld* World = GetWorld();
    if (!World) return;

    // 레벨 시퀀스 액터와 플레이어 생성
    FMovieSceneSequencePlaybackSettings Settings;
    ALevelSequenceActor* LevelSequenceActor;
    ULevelSequencePlayer* LevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(World, LevelSequence, Settings, LevelSequenceActor);

    if (LevelSequencePlayer)
    {
        LevelSequencePlayer->Play();
    }
}

FItem AgachaGameMode::DrawItemTable(const UDataTable* ItemDataTable)
{
    // 1. 확률에 따라 아이템의 등급 결정
    int32 RandomNum = FMath::RandRange(0, 99);
    int32 Grade = (RandomNum < 60) ? 1 :
        (RandomNum < 85) ? 2 :
        (RandomNum < 97) ? 3 : 4;

    // 2. 결정된 등급의 아이템들 중에서 무작위로 하나 선택
    TArray<FItem*> ItemsOfGrade;
    FString ContextString;
    for (const auto& Row : ItemDataTable->GetRowMap())
    {
        const FItem* Item = (FItem*)Row.Value;
        if (Item && Item->ItemGrade == Grade)
        {
            ItemsOfGrade.Add(const_cast<FItem*>(Item));
        }
    }

    FItem SelectedItem;
    if (!ItemsOfGrade.IsEmpty())
    {
        int32 RandomIndex = FMath::RandRange(0, ItemsOfGrade.Num() - 1);
        SelectedItem = *ItemsOfGrade[RandomIndex]; // 안전한 접근을 위해 IsValidIndex 검사 생략
    }
    else
    {
        // 만약 해당 등급의 아이템이 없으면, 빈 아이템 반환
        return FItem();
    }

    // 3. 아이템 등급에 따라 시퀀스 재생
    switch (SelectedItem.ItemGrade)
    {
    case 1:
        PlaySequence("/Game/ThirdPerson/common");
        break;
    case 2:
        PlaySequence("/Game/ThirdPerson/rare");
        break;
    case 3:
        PlaySequence("/Game/ThirdPerson/epic");
        break;
    case 4:
        PlaySequence("/Game/ThirdPerson/legend");
        break;
    default:
        break;
    }

    return SelectedItem;
}


void AgachaGameMode::DrawResult()
{
    int32 Result = GetRandomResult();

    FString ResultString;
    if (Result >= 0 && Result < 60) // 0~59 ������ ����� ���� ó��
    {
        ResultString = TEXT("60% N grade");
    }
    else if (Result >= 60 && Result < 85) // 60~84 ������ ��� (25% Ȯ��)�� ���� ó��
    {
        ResultString = TEXT("25% R grade");
    }
    else if (Result >= 85 && Result < 97) // 85~96 ������ ��� (12% Ȯ��)�� ���� ó��
    {
        ResultString = TEXT("12% SR grade");
    }
    else if (Result >= 97 && Result < 100) // 97~99 ������ ��� (3% Ȯ��)�� ���� ó��
    {
        ResultString = TEXT("3% UR grade");
    }

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, *ResultString);
}


int32 AgachaGameMode::GetRandomResult() const
{
    return FMath::RandRange(0, 99); // 0~99 ���� ���� �� ����
}