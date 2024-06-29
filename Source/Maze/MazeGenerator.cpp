#include "MazeGenerator.h"
#include "Math/Vector.h"

AMazeGenerator::FMazeData::FMazeData(AMazeGenerator& MazeGen): MazeGen{MazeGen}
{
}

void AMazeGenerator::FMazeData::Init()
{
	Data.Init(1, MazeGen.Width * MazeGen.Height);
}

int32 AMazeGenerator::FMazeData::Get(const int32 X, const int32 Y) const
{
	return Data[X + Y * MazeGen.Width];
}

void AMazeGenerator::FMazeData::Set(const int32 X, const int32 Y, const int32 Value)
{
	Data[X + Y * MazeGen.Width] = Value;
}

void AMazeGenerator::FMazeData::Open(const int32 X, const int32 Y)
{
	Set(X, Y, 0);
}

bool AMazeGenerator::FMazeData::IsOpen(const int32 X, const int32 Y) const
{
	return Data[X + Y * MazeGen.Width] == 0;
}

bool AMazeGenerator::FMazeData::IsClosed(const int32 X, const int32 Y) const
{
	return Data[X + Y * MazeGen.Width] == 1;
}

bool AMazeGenerator::FMazeData::IsValid(const int32 X, const int32 Y) const
{
	return X >= 0 && X < MazeGen.Width - 1 && Y >= 0 && Y < MazeGen.Height - 1;
}

void AMazeGenerator::FWorldDirections::Shuffle()
{
	const int32 LastIndex = Data.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		const int32 Index = FMath::RandRange(0, LastIndex);
		if (i != Index)
		{
			Data.Swap(i, Index);
		}
	}
}

TTuple<int32, int32> AMazeGenerator::FWorldDirections::operator[](const size_t& Index)
{
	return Data[Index];
}

int32 AMazeGenerator::FWorldDirections::Num() const
{
	return Data.Num();
}

AMazeGenerator::AMazeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AMazeGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateMaze();
	PlacePieces();
}

void AMazeGenerator::GenerateMaze()
{
	Maze.Init();
	Maze.Open(StartX, StartY);

	switch (Algorithm)
	{
	case EMazeGenerationAlgorithmType::WithStack:
		GenerateMazeWithStack();
		break;
	case EMazeGenerationAlgorithmType::WithRecursion:
		GenerateMazeWithRecursion(StartX, StartY);
		break;
	}

	if (NoDeadEnds)
	{
		RemoveDeadEndsInside();
		RemoveDeadEndsOnEdges();
		RemoveDeadEndsAtCorners();
	}
}

void AMazeGenerator::GenerateMazeWithStack()
{
	TArray<TTuple<int32, int32>> Stack;
	Stack.Push({StartX, StartY});

	while (!Stack.IsEmpty())
	{
		const int32 CurrentX = Stack.Top().Key;
		const int32 CurrentY = Stack.Top().Value;
		Stack.Pop();

		Directions.Shuffle();

		for (size_t i = 0; i < 4; ++i)
		{
			int32 NextX = CurrentX + Directions[i].Key * 2;
			int32 NextY = CurrentY + Directions[i].Value * 2;

			if (!Maze.IsValid(NextX, NextY))
				continue;

			if (!Maze.IsOpen(NextX, NextY))
			{
				Maze.Open((NextX + CurrentX) / 2, (NextY + CurrentY) / 2);
				Maze.Open(NextX, NextY);
				Stack.Push({NextX, NextY});
			}
		}
	}
}

void AMazeGenerator::GenerateMazeWithRecursion(const int32 X, const int32 Y)
{
	Directions.Shuffle();

	for (int32 i = 0; i < Directions.Num(); i++)
	{
		if (Directions[i] == TTuple<int32, int32>{1, 0}) // Up
		{
			if (X + 2 >= Width - 1 || Maze.IsOpen(X + 2, Y))
				continue;

			Maze.Open(X + 2, Y);
			Maze.Open(X + 1, Y);
			GenerateMazeWithRecursion(X + 2, Y);
		}
		else if (Directions[i] == TTuple<int32, int32>{0, -1}) // Right
		{
			if (Y + 2 >= Height - 1 || Maze.IsOpen(X, Y + 2))
				continue;

			Maze.Open(X, Y + 2);
			Maze.Open(X, Y + 1);
			GenerateMazeWithRecursion(X, Y + 2);
		}
		else if (Directions[i] == TTuple<int32, int32>{-1, 0}) // Down
		{
			if (X - 2 <= 0 || Maze.IsOpen(X - 2, Y))
				continue;

			Maze.Open(X - 2, Y);
			Maze.Open(X - 1, Y);
			GenerateMazeWithRecursion(X - 2, Y);
		}
		else if (Directions[i] == TTuple<int32, int32>{0, 1}) // Left
		{
			if (Y - 2 <= 0 || Maze.IsOpen(X, Y - 2))
				continue;

			Maze.Open(X, Y - 2);
			Maze.Open(X, Y - 1);
			GenerateMazeWithRecursion(X, Y - 2);
		}
	}
}

void AMazeGenerator::RemoveDeadEndsInside()
{
	for (int32 x = 2; x < Width - 3; x++)
	{
		for (int32 y = 2; y < Height - 3; y++)
		{
			int NextX = x;
			int NextY = y;
			if (IsPatternMatching(x, y, DeadEndUpPattern))
			{
				NextY--;
				while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
				{
					Maze.Open(NextX, NextY);
					NextY--;
				}
			}
			else if (IsPatternMatching(x, y, DeadEndDownPattern))
			{
				NextY++;
				while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
				{
					Maze.Open(NextX, NextY);
					NextY++;
				}
			}
			else if (IsPatternMatching(x, y, DeadEndLeftPattern))
			{
				NextX++;
				while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
				{
					Maze.Open(NextX, NextY);
					NextX++;
				}
			}
			else if (IsPatternMatching(x, y, DeadEndRightPattern))
			{
				NextX--;
				while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
				{
					Maze.Open(NextX, NextY);
					NextX--;
				}
			}
		}
	}
}

void AMazeGenerator::RemoveDeadEndsOnEdges()
{
	// Left Edge
	RemoveDeadEndsOnVerticalEdges(1);

	// Bottom Edge
	RemoveDeadEndsOnHorizontalEdges(1);

	// Right Edge
	RemoveDeadEndsOnVerticalEdges(Height - 3);

	// Top Edge
	RemoveDeadEndsOnHorizontalEdges(Width - 3);
}

void AMazeGenerator::RemoveDeadEndsOnVerticalEdges(const int32 Y)
{
	for (int32 x = 2; x < Width - 3; x++)
	{
		int32 NextX = x;
		if (IsPatternMatching(x, Y, DeadEndUpPattern))
		{
			NextX--;
			while (Maze.IsValid(NextX, Y) && Maze.IsClosed(NextX, Y))
			{
				Maze.Open(NextX, Y);
				NextX--;
			}
		}
		else if (IsPatternMatching(x, Y, DeadEndDownPattern))
		{
			NextX++;
			while (Maze.IsValid(NextX, Y) && Maze.IsClosed(NextX, Y))
			{
				Maze.Open(NextX, Y);
				NextX++;
			}
		}
		else if (IsPatternMatching(x, Y, DeadEndLeftPattern))
		{
			NextX++;
			while (Maze.IsValid(NextX, Y) && Maze.IsClosed(NextX, Y))
			{
				Maze.Open(NextX, Y);
				NextX++;
			}
		}
		else if (IsPatternMatching(x, Y, DeadEndRightPattern))
		{
			NextX--;
			while (Maze.IsValid(NextX, Y) && Maze.IsClosed(NextX, Y))
			{
				Maze.Open(NextX, Y);
				NextX--;
			}
		}
	}
}

void AMazeGenerator::RemoveDeadEndsOnHorizontalEdges(const int32 X)
{
	for (int32 y = 2; y < Height - 3; y++)
	{
		int32 NextY = y;
		if (IsPatternMatching(X, y, DeadEndUpPattern))
		{
			NextY--;
			while (Maze.IsValid(X, NextY) && Maze.IsClosed(X, NextY))
			{
				Maze.Open(X, NextY);
				NextY--;
			}
		}
		else if (IsPatternMatching(X, y, DeadEndDownPattern))
		{
			NextY++;
			while (Maze.IsValid(X, NextY) && Maze.IsClosed(X, NextY))
			{
				Maze.Open(X, NextY);
				NextY++;
			}
		}
		else if (IsPatternMatching(X, y, DeadEndLeftPattern))
		{
			NextY++;
			while (Maze.IsValid(X, NextY) && Maze.IsClosed(X, NextY))
			{
				Maze.Open(X, NextY);
				NextY++;
			}
		}
		else if (IsPatternMatching(X, y, DeadEndRightPattern))
		{
			NextY--;
			while (Maze.IsValid(X, NextY) && Maze.IsClosed(X, NextY))
			{
				Maze.Open(X, NextY);
				NextY--;
			}
		}
	}
}

void AMazeGenerator::RemoveDeadEndsAtCorners()
{
	RemoveDeadEndAtBottomLeft();
	RemoveDeadEndAtTopLeft();
	RemoveDeadEndAtBottomRight();
	RemoveDeadAtTopRight();
}

void AMazeGenerator::RemoveDeadEndAtBottomLeft()
{
	int32 NextX = 1;
	int32 NextY = 1;
	if (IsPatternMatching(1, 1, DeadEndUpPattern))
	{
		NextX++;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextX++;
		}
	}
	else if (IsPatternMatching(1, 1, DeadEndRightPattern))
	{
		NextY++;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextY++;
		}
	}
}

void AMazeGenerator::RemoveDeadEndAtTopLeft()
{
	int32 NextX = Width - 3;
	int32 NextY = 1;
	if (IsPatternMatching(Width - 3, 1, DeadEndLeftPattern))
	{
		NextY++;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextY++;
		}
	}
	else if (IsPatternMatching(Width - 3, 1, DeadEndUpPattern))
	{
		NextX--;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextX--;
		}
	}
}

void AMazeGenerator::RemoveDeadEndAtBottomRight()
{
	int32 NextX = 1;
	int32 NextY = Height - 3;
	if (IsPatternMatching(1, Height - 3, DeadEndDownPattern))
	{
		NextX++;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextX++;
		}
	}
	else if (IsPatternMatching(1, Height - 3, DeadEndRightPattern))
	{
		NextY--;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextY--;
		}
	}
}

void AMazeGenerator::RemoveDeadAtTopRight()
{
	int32 NextX = Width - 3;
	int32 NextY = Height - 3;
	if (IsPatternMatching(Width - 3, Height - 3, DeadEndLeftPattern))
	{
		NextY--;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextY--;
		}
	}
	else if (IsPatternMatching(Width - 3, Height - 3, DeadEndDownPattern))
	{
		NextX--;
		while (Maze.IsValid(NextX, NextY) && Maze.IsClosed(NextX, NextY))
		{
			Maze.Open(NextX, NextY);
			NextX--;
		}
	}
}

void AMazeGenerator::PlacePieces() const
{
	for (int32 x = 1; x < Width - 1; x++)
	{
		for (int32 y = 1; y < Height - 1; y++)
		{
			// Straight
			if (IsPatternMatching(x, y, HorizontalStraightPattern)) { PlacePiece(x, y, 90.f, StraightPiece); }
			else if (IsPatternMatching(x, y, VerticalStraightPattern)) { PlacePiece(x, y, 0.f, StraightPiece); }

			// Turns
			else if (IsPatternMatching(x, y, TurnLeftUpPattern)) { PlacePiece(x, y, 0.f, TurnPiece); }
			else if (IsPatternMatching(x, y, TurnLeftDownPattern)) { PlacePiece(x, y, 90.f, TurnPiece); }
			else if (IsPatternMatching(x, y, TurnRightUpPattern)) { PlacePiece(x, y, -90.f, TurnPiece); }
			else if (IsPatternMatching(x, y, TurnRightDownPattern)) { PlacePiece(x, y, 180.f, TurnPiece); }

			// T Junctions
			else if (IsPatternMatching(x, y, TJunctionUpPattern)) { PlacePiece(x, y, -90.f, TJunctionPiece); }
			else if (IsPatternMatching(x, y, TJunctionDownPattern)) { PlacePiece(x, y, 90.f, TJunctionPiece); }
			else if (IsPatternMatching(x, y, TJunctionLeftPattern)) { PlacePiece(x, y, 0.f, TJunctionPiece); }
			else if (IsPatternMatching(x, y, TJunctionRightPattern)) { PlacePiece(x, y, 180.f, TJunctionPiece); }

			// Dead ends
			else if (IsPatternMatching(x, y, DeadEndUpPattern)) { PlacePiece(x, y, 90.f, DeadEndPiece); }
			else if (IsPatternMatching(x, y, DeadEndDownPattern)) { PlacePiece(x, y, -90.f, DeadEndPiece); }
			else if (IsPatternMatching(x, y, DeadEndLeftPattern)) { PlacePiece(x, y, 180.f, DeadEndPiece); }
			else if (IsPatternMatching(x, y, DeadEndRightPattern)) { PlacePiece(x, y, 0.f, DeadEndPiece); }

			// Crossroad
			else if (IsPatternMatching(x, y, CrossroadPattern)) { PlacePiece(x, y, 0.f, CrossroadPiece); }
		}
	}
}

// All patterns are defined in Maze.h
// Example pattern: [ 1 0 1
//                    0 0 0
//                    1 0 1 ] is a crossroad.
bool AMazeGenerator::IsPatternMatching(const int32 X, const int32 Y, const TArray<int32>& Pattern) const
{
	int Count = 0;
	int i = 0;
	for (int y = 1; y > -2; y--)
	{
		for (int x = -1; x < 2; x++)
		{
			if (Pattern[i] == Maze.Get(X + x, Y + y) || Pattern[i] == 5)
			{
				Count++;
			}

			i++;
		}
	}

	return Count == 9;
}

void AMazeGenerator::PlacePiece(const int32 X, const int32 Y, const float& Yaw, const TSubclassOf<AActor>& Piece) const
{
	const FVector Location(X * Scale, Y * Scale, 0);
	const FRotator Rotation(0, Yaw, 0);
	const FActorSpawnParameters SpawnInfo;

	GetWorld()->SpawnActor<AActor>(Piece, Location, Rotation, SpawnInfo);
}
