#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeGenerator.generated.h"

UENUM(BlueprintType)
enum class EMazeGenerationAlgorithmType : uint8
{
	WithStack,
	WithRecursion
};

UCLASS()
class MAZE_API AMazeGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMazeGenerator();

protected:
	virtual void BeginPlay() override;

	class FMazeData
	{
	public:
		FMazeData(AMazeGenerator& MazeGen);
		void Init();
		int32 Get(const int32 X, const int32 Y) const;
		void Set(const int32 X, const int32 Y, const int32 Value);
		bool IsValid(const int32 X, const int32 Y) const;
		void Open(const int32 X, const int32 Y);
		bool IsOpen(const int32 X, const int32 Y) const;
		bool IsClosed(const int32 X, const int32 Y) const;

	private:
		AMazeGenerator& MazeGen;
		TArray<int32> Data;
	};

	FMazeData Maze = FMazeData(*this);

	class FWorldDirections
	{
	public:
		void Shuffle();
		TTuple<int32, int32> operator[](const size_t& Index);
		int32 Num() const;

	private:
		TArray<TTuple<int32, int32>> Data = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
	};

	FWorldDirections Directions;

	void GenerateMaze();
	void GenerateMazeWithStack();
	void GenerateMazeWithRecursion(const int32 X, const int32 Y);

	void RemoveDeadEndsInside();
	void RemoveDeadEndsOnEdges();
	void RemoveDeadEndsOnVerticalEdges(const int32 Y);
	void RemoveDeadEndsOnHorizontalEdges(const int32 X);
	void RemoveDeadEndsAtCorners();

	void PlacePieces() const;
	void PlacePiece(const int32 X, const int32 Y, const float& Yaw, const TSubclassOf<AActor>& Piece) const;
	bool IsPatternMatching(const int32 X, const int32 Y, const TArray<int32>& Pattern) const;

	// Straights
	TArray<int32> HorizontalStraightPattern = {
		5, 1, 5,
		0, 0, 0,
		5, 1, 5
	};

	TArray<int32> VerticalStraightPattern = {
		5, 0, 5,
		1, 0, 1,
		5, 0, 5
	};

	// T Junctions
	TArray<int32> TJunctionUpPattern = {
		1, 0, 1,
		0, 0, 0,
		5, 1, 5
	};

	TArray<int32> TJunctionDownPattern = {
		5, 1, 5,
		0, 0, 0,
		1, 0, 1
	};

	TArray<int32> TJunctionLeftPattern = {
		1, 0, 5,
		0, 0, 1,
		1, 0, 5
	};

	TArray<int32> TJunctionRightPattern = {
		5, 0, 1,
		1, 0, 0,
		5, 0, 1
	};

	// Crossroad
	TArray<int32> CrossroadPattern = {
		1, 0, 1,
		0, 0, 0,
		1, 0, 1
	};

	// Turns
	TArray<int32> TurnLeftUpPattern = {
		1, 0, 5,
		0, 0, 1,
		5, 1, 5
	};

	TArray<int32> TurnLeftDownPattern = {
		5, 1, 5,
		0, 0, 1,
		1, 0, 5
	};

	TArray<int32> TurnRightUpPattern = {
		5, 0, 1,
		1, 0, 0,
		5, 1, 5
	};

	TArray<int32> TurnRightDownPattern = {
		5, 1, 5,
		1, 0, 0,
		5, 0, 1
	};

	// Dead ends
	TArray<int32> DeadEndUpPattern = {
		5, 0, 5,
		1, 0, 1,
		5, 1, 5
	};

	TArray<int32> DeadEndDownPattern = {
		5, 1, 5,
		1, 0, 1,
		5, 0, 5
	};

	TArray<int32> DeadEndLeftPattern = {
		5, 1, 5,
		0, 0, 1,
		5, 1, 5
	};

	TArray<int32> DeadEndRightPattern = {
		5, 1, 5,
		1, 0, 0,
		5, 1, 5
	};

public:
	UPROPERTY(EditAnywhere)
	EMazeGenerationAlgorithmType Algorithm = EMazeGenerationAlgorithmType::WithRecursion;

	UPROPERTY(EditAnywhere)
	bool NoDeadEnds = false;

	UPROPERTY(EditAnywhere)
	int32 StartX = 5;

	UPROPERTY(EditAnywhere)
	int32 StartY = 5;

	UPROPERTY(EditAnywhere)
	int32 Width = 50;

	UPROPERTY(EditAnywhere)
	int32 Height = 100;

	UPROPERTY(EditAnywhere)
	int32 Scale = 300;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> StraightPiece;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> TJunctionPiece;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> CrossroadPiece;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> TurnPiece;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DeadEndPiece;
};
