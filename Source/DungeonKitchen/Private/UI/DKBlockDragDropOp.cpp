// Copyright © 2025 Tartare Studio


#include "UI/DKBlockDragDropOp.h"

static FORCEINLINE FIntPoint RotCell90(const FIntPoint& P, int32 Steps) // 0..3
{
	switch ((Steps % 4 + 4) % 4)
	{
	case 1:  return FIntPoint(P.Y, -P.X); // CW 90
	case 2:  return FIntPoint(-P.X, -P.Y);
	case 3:  return FIntPoint(-P.Y, P.X); // CW 270
	default: return P;                        // 0
	}
}

void UDKBlockDragDropOp::InitFromShape(const FBlockShapeDef& InDef, int32 InRotation)
{
	DragBlockDef = InDef;
	DragRotation = InRotation;
}

TArray<FIntPoint> UDKBlockDragDropOp::GetRotatedCells() const
{
	TArray<FIntPoint> Out;
	RotateCells(DragBlockDef.Cells, DragRotation, Out);
	return Out;
}

void UDKBlockDragDropOp::GetRotatedCellsAndGrab(TArray<FIntPoint>& OutCellsR, FIntPoint& OutGrabR) const
{
	OutCellsR.Reset();

	const int32 Rot = (DragRotation % 4 + 4) % 4;

	// 1) 회전
	TArray<FIntPoint> Tmp;
	Tmp.Reserve(DragBlockDef.Cells.Num());
	for (const FIntPoint& P : DragBlockDef.Cells)
		Tmp.Add(RotCell90(P, Rot));

	OutGrabR = RotCell90(GrabCell, Rot);

	// 2) 정규화(좌상단 → 0,0)
	if (Tmp.Num() == 0) { OutGrabR = FIntPoint::ZeroValue; return; }

	int32 MinR = Tmp[0].X, MinC = Tmp[0].Y;
	for (const FIntPoint& P : Tmp) { MinR = FMath::Min(MinR, P.X); MinC = FMath::Min(MinC, P.Y); }

	OutCellsR.Reserve(Tmp.Num());
	for (const FIntPoint& P : Tmp)
		OutCellsR.Add(FIntPoint(P.X - MinR, P.Y - MinC));

	OutGrabR.X -= MinR;
	OutGrabR.Y -= MinC;
}

void UDKBlockDragDropOp::RotateCells(const TArray<FIntPoint>& In, int32 Rot90, TArray<FIntPoint>& Out)
{
	Out.Reset();
	if (In.Num() == 0) return;

	const int32 Rot = (Rot90 % 4 + 4) % 4;

	// 회전
	TArray<FIntPoint> Tmp;
	Tmp.Reserve(In.Num());
	for (const FIntPoint& P : In)
		Tmp.Add(RotCell90(P, Rot));

	// 정규화
	int32 MinR = Tmp[0].X, MinC = Tmp[0].Y;
	for (const FIntPoint& P : Tmp) { MinR = FMath::Min(MinR, P.X); MinC = FMath::Min(MinC, P.Y); }

	Out.Reserve(Tmp.Num());
	for (const FIntPoint& P : Tmp)
		Out.Add(FIntPoint(P.X - MinR, P.Y - MinC));
}
