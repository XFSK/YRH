
#include "OnlineGameMode.h"

#include "MyPlayer.h"
#include "OnlinePlayerController.h"
#include "OnlinePlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

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
