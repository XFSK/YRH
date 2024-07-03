
#include "OnlineGameMode.h"

#include "MyPlayer.h"
#include "OnlinePlayerController.h"
#include "OnlinePlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AOnlineGameMode::AOnlineGameMode()
{
	bDelayedStart = true;
}

void AOnlineGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	
}

void AOnlineGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AOnlineGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
	{
		AOnlinePlayerController* OnlinePlayerController = Cast<AOnlinePlayerController>(*It);
		if(OnlinePlayerController)
		{
			OnlinePlayerController->OnMatchStateSet(MatchState);
		}
		
	}
}

void AOnlineGameMode::PlayerEliminated(class AMyPlayer* ElimmedPlayer,class AOnlinePlayerController* VictimController,AOnlinePlayerController* AttackerController)
{
	AOnlinePlayerState* AttackerPlayerState = AttackerController ? Cast<AOnlinePlayerState>(AttackerController->PlayerState) : nullptr;
	AOnlinePlayerState* VictimPlayerState = VictimController ? Cast<AOnlinePlayerState>(VictimController->PlayerState) : nullptr;

	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if(ElimmedPlayer)
	{
		ElimmedPlayer->Elim();
	}
}

void AOnlineGameMode::RequestRespawn(AMyPlayer* ElimmedPlayer, AController* ElimmedController)
{
	if(ElimmedPlayer)
	{
		ElimmedPlayer->Reset();
		ElimmedPlayer->Destroy();
	}
	if(ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,APlayerStart::StaticClass(),PlayerStarts);
		int32 Selection = FMath::RandRange(0,PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController,PlayerStarts[Selection]);
	}
}


