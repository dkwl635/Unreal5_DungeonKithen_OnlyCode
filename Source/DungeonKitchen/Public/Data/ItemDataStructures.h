// Copyright © 2025 Tartare Studio
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ItemDataStructures.generated.h"



// 모든 아이템 데이터 구조체들을 한 헤더에 정의
UENUM(BlueprintType)
enum class EDataType : uint8
{
	Item,

};

// 인벤토리 타입
UENUM(BlueprintType)
enum class EDKInventoryType : uint8
{
    Pantry      UMETA(DisplayName="Pantry"),
    PrepTray    UMETA(DisplayName="Prep Tray"),
    Cooking     UMETA(DisplayName="Cooking"),
    Dish		UMETA(DisplayName="Dish"),
};

USTRUCT(BlueprintType)
struct FDKItemData : public FTableRowBase
{
	GENERATED_BODY()

	//아이템 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ItemTag;

	//아이템 고유 번호
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ItemID = -1;

	//아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemName;

	//아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Description;

	//아이템 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* ItemIcon;

	//아이템 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* ItemMesh;
};

USTRUCT(BlueprintType)
//음식 재료
struct FDKFoodData : public FDKItemData
{
	GENERATED_BODY()

	//음식, 재료,  등급
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FoodRank = 0;

	//아이템 슬롯 모양
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShapeId = -1;

};

USTRUCT(BlueprintType)
//음식 요리
struct FDKDishData : public FDKFoodData
{
	GENERATED_BODY()

	//음식 이펙트 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString DishEffect = "DishEffectDesc";

	//음식 효과 번호
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DishEffectIndex = -1;
	//음식 오브젝트... 필요에 에 따라
};


USTRUCT(BlueprintType)
//무기 관련
struct FDKWeaponData : public FDKItemData
{
	GENERATED_BODY()

	//무기 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponName;

	//무기 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponType;

	//무기 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponTag;

	//무기 기본 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rounds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Ammo = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WeaponRank = 0;
};


USTRUCT(BlueprintType)
//소모 아이템
struct FDKGameItemData : public FDKItemData
{
	GENERATED_BODY()

	//아이템 종류
	//아이템 효과

};

// 모양 정의 (Data/Config 레벨: ㄱ/ㄴ/2x2 같은 유형)
USTRUCT(BlueprintType)
struct FBlockShapeDef
{
    GENERATED_BODY()
	// 편한 식별자(에디터에서도 보기 좋게)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShapeId = -1;
	// 원점(0,0) 기준 상대좌표 집합 (칸 단위)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FIntPoint> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<class UTexture2D*> BlockImage;

	// 색(더미 표시)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.0f, 0.6f, 1.0f, 1.0f);
	// 0~3 (90도 단위 회전) — 정의에는 보통 0, 인스턴스에서 바꿔씀
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DefaultRotation = 0;
};

// 실제로 그리드 위에 놓인 ‘인스턴스’ (런타임 상태 + 저장 동일 구조)
USTRUCT(BlueprintType)
struct FPlacedBlock
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Row = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Col = 0;

    // 0~3: 90도 단위 회전 (또는 0/90/180/270 정규화해서 보관)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Rotation = 0;

    // 정의 복원용 키
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShapeId = -1;

    // 저장된 요리 아이템 식별용 ID (DataTable 조회용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DishId = -1;

    // (옵션) 미리 계산해서 들고 다니고 싶으면 유지, 아니면 지워도 됨
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FIntPoint> RotatedCells;

    // 식별용
    UPROPERTY()
    FGuid Id = FGuid::NewGuid();

	// 아이콘 이미지
	UPROPERTY()
	TSoftObjectPtr<UTexture2D> IconTexture;
};


UCLASS(BlueprintType , Abstract)
class DUNGEONKITCHEN_API UDKItem : public UObject
{
	GENERATED_BODY()

public:
	virtual void Init(FDKItemData* Data, int32 InStack);

	virtual int32 AddStack(int32 AddStack);
	virtual int32 RemoveStack(int32 RemoveStack);

	UFUNCTION(BlueprintCallable)
	virtual int32 GetItemID() const { return ItemID; }
	UFUNCTION(BlueprintCallable)
	virtual FName GetItemName() const { return ItemName; }
	UFUNCTION(BlueprintCallable)
	virtual FString GetItemDescription() const { return Description; }
	UFUNCTION(BlueprintCallable)
	virtual UTexture2D* GetItemIcon() const { return ItemIcon; }
	UFUNCTION(BlueprintCallable)
	virtual int32 GetMaxStack() const { return MaxStack; }
	UFUNCTION(BlueprintCallable)
	virtual int32 GetStack() const { return Stack; }
	UFUNCTION(BlueprintCallable)
	virtual UStaticMesh* GetMesh() const {return Mesh;}
protected:
	UPROPERTY(VisibleAnywhere)
	FGameplayTag ItemTag;

	UPROPERTY(VisibleAnywhere)
	int32 ItemID = -1;

	UPROPERTY(VisibleAnywhere)
	FName ItemName;

	UPROPERTY(VisibleAnywhere)
	FString Description;

	UPROPERTY(VisibleAnywhere)
	UTexture2D* ItemIcon;

	UPROPERTY(VisibleAnywhere)
	int32 MaxStack = 1;

	UPROPERTY(VisibleAnywhere)
	int32 Stack = 1;

	UPROPERTY(VisibleAnywhere)
	UStaticMesh* Mesh;

};
UCLASS(BlueprintType)
class DUNGEONKITCHEN_API UDKFood : public UDKItem
{
	GENERATED_BODY()

public:
	virtual void Init(FDKItemData* Data,int32 InStack) override;

	UFUNCTION(BlueprintCallable)
	int32 GetFoodRank() const { return FoodRank; }
	UFUNCTION(BlueprintCallable)
	int32 GetExpirationDate() const { return ExpirationDate; }
	UFUNCTION(BlueprintCallable)
	void RemoveExpirationDate(int32 Count) { ExpirationDate-= Count; }
	UFUNCTION(BlueprintCallable)
	int32 GetShapeId(){return ShapeId;}

protected:
	UPROPERTY(VisibleAnywhere)
	int32 FoodRank = 0;

	UPROPERTY(VisibleAnywhere)
	int32 ExpirationDate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShapeId = -1;
};


UCLASS(BlueprintType)
class DUNGEONKITCHEN_API UDKDish : public UDKFood
{
	GENERATED_BODY()

public:
	virtual void Init(FDKItemData* Data,int32 InStack) override;

public:
	FDKDishEffectData* EffectData;
	bool bIsEffect = false;

	int32 TempInt = 0;
	float TempFloat = 0.0f;
};


USTRUCT(BlueprintType)
struct FDKRecipeData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RecipeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> Ingredients;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ResultDish = -1;
};

USTRUCT(BlueprintType)
struct FDKDishEffectData : public FTableRowBase
{
	GENERATED_BODY()

	//이펙트 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString EffectDesc = "";
	//이펙트 조건
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ConditionTag;
	//이펙트 조건 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConditionValue = 0;
	//이펙트 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time = 0;

	//이펙트1 타겟
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EffectTarget1;
	//이펙트1 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectValue1 = 0.0f;
	//이펙트2 타겟
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EffectTarget2;
	//이펙트2 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectValue2 = 0.0f;

};





//C++
UCLASS()
class DUNGEONKITCHEN_API UItemDataStructures : public UObject
{
	GENERATED_BODY()

};

