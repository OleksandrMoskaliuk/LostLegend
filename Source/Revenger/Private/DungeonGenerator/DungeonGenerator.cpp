// Copyright (c) [2024] [Alex Mercer]All rights reserved.Redistribution and use in source and binary forms, with or withoutmodification, are permitted provided that the following conditions are met:1. Redistributions of source code must retain the above copyright notice,   this list of conditions, and the following disclaimer.2. Redistributions in binary form must reproduce the above copyright notice,   this list of conditions, and the following disclaimer in the documentation   and/or other materials provided with the distribution.For inquiries or more information, contact: @Dru9Dealer on Twitter.THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THEIMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSEARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BELIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, ORCONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OFSUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESSINTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER INCONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THEPOSSIBILITY OF SUCH DAMAGE.


#include "DungeonGenerator/DungeonGenerator.h"
#include "Math/RandomStream.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY(DungeonGenerator);

const FName ADungeonGenerator::DUNGEON_MESH_TAG = FName("Orfeas_Dungeon_Generator");

float ADungeonGenerator::CalculateFloorTileSize(const UStaticMesh& Mesh) const
{
    return FMath::Abs(Mesh.GetBoundingBox().Min.Y) + FMath::Abs(Mesh.GetBoundingBox().Max.Y);
}

FRotator ADungeonGenerator::CalculateWallRotation(bool bWallFacingXProperty, const FTileMatrix::FWallSpawnPoint& WallSpawnPoint, const FVector& WallPivotOffsetOverride, FVector& LocationOffset) const
{
    FRotator WallRotation = FRotator::ZeroRotator;
    LocationOffset = FVector();

    // If the point is generated in a way that is looking at the X axis and the wall is rotated to look at Y make sure to
    // rotate the wall and apply an offset
    // Note: Points looking at X axis are spread along Y
    // WallSpawnPoint.bFacingX = true when the wall is located in an "up/down" tile
    if (!bWallFacingXProperty && WallSpawnPoint.bFacingX) {
        WallRotation = FRotator(0.f, -90.f, 0.f);
        // LocationOffset.Y += FMath::Abs(WallSMPivotOffset.X);
        LocationOffset.Y += FMath::Abs(WallPivotOffsetOverride.X);
    } else if (!WallSpawnPoint.bFacingX && bWallFacingXProperty) {
        WallRotation = FRotator(0.f, -90.f, 0.f);
    } else // No rotation adjustments needed; just apply the original offset
    {
        // LocationOffset+=WallSMPivotOffset;
        LocationOffset += WallPivotOffsetOverride;
    }

    return WallRotation;
}

void ADungeonGenerator::SpawnDungeonFromDataTable()
{
    TArray<FRoomTemplate*> RoomTemplates;
    FString ContextStr;
    RoomTemplatesDataTable->GetAllRows<FRoomTemplate>(ContextStr, RoomTemplates);

    ensure(RoomTemplates.Num() > 0);

    float DataTableFloorTileSize = CalculateFloorTileSize(*(*RoomTemplates[0]).RoomTileMesh);

    TArray<FTileMatrix::FRoom> Rooms;
    TArray<FVector> CorridorFloorTiles;
    TArray<FTileMatrix::FWallSpawnPoint> CorridorWalls;
    TileMatrix.ProjectTileMapLocationsToWorld(DataTableFloorTileSize, Rooms, CorridorFloorTiles, CorridorWalls);

    // Spawn rooms & walls using a random template from the provided table
    for (int32 i = 0; i < Rooms.Num(); i++) {
        FRoomTemplate RoomTemplate = *RoomTemplates[FMath::RandRange(0, RoomTemplates.Num() - 1)];

        for (int32 j = 0; j < Rooms[i].FloorTileWorldLocations.Num(); j++) {
            FVector WorldSpawnLocation = Rooms[i].FloorTileWorldLocations[j];
            SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, WorldSpawnLocation + RoomTemplate.RoomTilePivotOffset), RoomTemplate.RoomTileMesh, RoomTemplate.RoomTileMeshMaterialOverride);
        }

        for (int32 j = 0; j < Rooms[i].WallSpawnPoints.Num(); j++) {
            // FVector WorldSpawnLocation = Rooms[i].WallSpawnPoints[j].WorldLocation;
            FVector WallModifiedOffset = FVector();
            FRotator WallRotation = CalculateWallRotation(RoomTemplate.bIsWallFacingX, Rooms[i].WallSpawnPoints[j], RoomTemplate.WallMeshPivotOffset, WallModifiedOffset);
            FVector WallSpawnLocation = Rooms[i].WallSpawnPoints[j].WorldLocation + WallModifiedOffset;
            SpawnDungeonMesh(FTransform(WallRotation, WallSpawnLocation), RoomTemplate.WallMesh, RoomTemplate.WallMeshMaterialOverride);
            
            
        }
    }
    

    // Get the 1st element of the data table to retrieve any pivot offsets
    // The 1st row of the data table will be used to create corridors connecting various spawned rooms
    FVector FloorTileOffset = RoomTemplates[0]->RoomTilePivotOffset;
    UStaticMesh* CorridorFloorTile = RoomTemplates[0]->RoomTileMesh;
    UStaticMesh* CorridorWall = RoomTemplates[0]->WallMesh;

    // Spawn floor tiles for corridors
    for (int32 i = 0; i < CorridorFloorTiles.Num(); i++) {
        // CorridorFloorTiles[i]+=FloorTileOffset;
        SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, CorridorFloorTiles[i] + FloorTileOffset), CorridorFloorTile);
    }

    bool bCorridorWallFacingX = RoomTemplates[0]->bIsWallFacingX;
    FVector RoomTemplateWallOffset = RoomTemplates[0]->WallMeshPivotOffset;

    // Spawn walls for corridors
    for (int32 i = 0; i < CorridorWalls.Num(); i++) {
        FVector WallModifiedOffset = FVector();
        FRotator WallRotation = CalculateWallRotation(bCorridorWallFacingX, CorridorWalls[i], RoomTemplateWallOffset, WallModifiedOffset);
        FVector WallSpawnPoint = CorridorWalls[i].WorldLocation + WallModifiedOffset;

        SpawnDungeonMesh(FTransform(WallRotation, WallSpawnPoint), CorridorWall);
    }

}

void ADungeonGenerator::SpawnDungeonFurnitureFromDataTable()
{
    TArray<FRoomTemplate*> RoomTemplates;
    FString ContextStr;
    RoomTemplatesDataTable->GetAllRows<FRoomTemplate>(ContextStr, RoomTemplates);

    ensure(RoomTemplates.Num() > 0);

    float DataTableFloorTileSize = CalculateFloorTileSize(*(*RoomTemplates[0]).RoomTileMesh);

    TArray<FTileMatrix::FRoom> Rooms;
    TArray<FVector> CorridorFloorTiles;
    TArray<FTileMatrix::FWallSpawnPoint> CorridorWalls;
    TileMatrix.ProjectTileMapLocationsToWorld(DataTableFloorTileSize, Rooms, CorridorFloorTiles, CorridorWalls);

    // spawn door frame and walls
    FRoomTemplate RoomTemplate0 = *RoomTemplates[FMath::RandRange(0, RoomTemplates.Num() - 1)];
    float TreasholdDistance = 400.f + 10.f;
    float TreasholdDistance2 = 565.f + 10.f; 
    // Wall or Door points to spwan for all rooms
    TArray<FVector> WallOrDoorsPoints;
    for (auto& corridor_pnt : CorridorFloorTiles) 
    {
        for (auto &Room : Rooms) 
        {
            // Make array to hold point that can be walls or door frame 
            for (auto& room_pnt : Room.FloorTileWorldLocations) 
            {
                if (FVector::DistXY(corridor_pnt, room_pnt) < TreasholdDistance) 
                {
                    WallOrDoorsPoints.Add(corridor_pnt);
                }
            }
        }
    }
    // Decide waht spawn wall or door frame
    // Check if corridor point do not have more than 3 closer room tiles if so spawn wall else spawn door frame
    for (const auto& room : Rooms) {
        for (const auto& flor_pnt : room.FloorTileWorldLocations) {
            int NeighborCoridorTile = 0;
            TArray<FVector> SpawnPoints;
            for (const auto& pnt : WallOrDoorsPoints) // for each room
            {
                if (FVector::DistXY(pnt, flor_pnt) < TreasholdDistance) {
                    SpawnPoints.Add(pnt);
                }
                if (FVector::DistXY(pnt, flor_pnt) < TreasholdDistance2) {
                    ++NeighborCoridorTile;
                }
            }
            if (NeighborCoridorTile > 2) {
                // spawn wall
            } 
            else {
                for (const auto sp_pnt : SpawnPoints) 
                {
                    // spwn door frame
                    AActor* act = SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, sp_pnt + RoomTemplate0.PillarPivotOffset), RoomTemplate0.PillarMesh, RoomTemplate0.RoomPillarMeshMaterialOverride);
                    FaceActorToPoint(act, flor_pnt);
                }
            }
        }
    }


    // Iterate through rooms
    for (int32 i = 0; i < Rooms.Num(); i++) {
        FRoomTemplate RoomTemplate = *RoomTemplates[FMath::RandRange(0, RoomTemplates.Num() - 1)];

        TArray<FVector> FloorPoints;

        for (int32 j = 0; j < Rooms[i].WallSpawnPoints.Num(); j++) {
            // Use only wall spawn point for more precision spawn
            FVector WorldSpawnLocation = Rooms[i].WallSpawnPoints[j].WorldLocation;
            // Calculate offsets
            FVector WallModifiedOffset;
            FRotator WallRotation;
            bool bShouldPlaceObject = true;
            FloorPoints.Add(WorldSpawnLocation);
            if (false) {
                // Check if the point is close to a wall
                if (IsPointCloseToWall(WorldSpawnLocation, Rooms[i].WallSpawnPoints, RoomTemplate.bIsWallFacingX, WallModifiedOffset, WallRotation, bShouldPlaceObject)) {
                    // Check if the object should be placed based on a random chance
                    if (bShouldPlaceObject) {
                        // Add an additional offset for placing the object from the wall
                        FVector ObjectPlacementOffset = FVector(50.0f, 0.0f, 0.0f); // Adjust this offset based on your requirements
                        SpawnDungeonMesh(FTransform(WallRotation, WorldSpawnLocation + WallModifiedOffset + ObjectPlacementOffset), RoomTemplate.WallMesh, RoomTemplate.WallMeshMaterialOverride);
                    }
                } else {
                    // Regular room tile placement
                    SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, WorldSpawnLocation + RoomTemplate.PillarPivotOffset), RoomTemplate.PillarMesh, RoomTemplate.RoomPillarMeshMaterialOverride);
                }
            }
        }
        

        // Spawn center, works as expected, Only if use wapp spawn poinats
        if (false) 
        {
        SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, GetRoomCenter(FloorPoints) +RoomTemplate.PillarPivotOffset), RoomTemplate.PillarMesh, RoomTemplate.RoomPillarMeshMaterialOverride);
        FloorPoints.Empty();
        }

        if (0) 
        {
        TArray<FVector> CornersSpawnPoints = GetRoomPointsCloseToCornersLocatoin(FloorPoints);
        for (int r = 0; r < FloorPoints.Num(); r++) {
                if (0) {
                    FVector SpawPnt = PushSpawnPointToCenter(FloorPoints[r], FloorPoints);
                    AActor* Actor = SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, SpawPnt + RoomTemplate.PillarPivotOffset),
                        RoomTemplate.PillarMesh, RoomTemplate.RoomPillarMeshMaterialOverride);
                    AlignActorWithWorld(Actor, FloorPoints);
                }
                bool CanSpawn = true;
                for (auto& pnt : CornersSpawnPoints) {
                    if (pnt.Equals(FloorPoints[r], 0.1)) {
                        CanSpawn = false;
                    }
                }
                if (CanSpawn) {
                    FVector SpawPnt = PushSpawnPointToCenter(FloorPoints[r], FloorPoints);
                    AActor* Actor = SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, SpawPnt + RoomTemplate.PillarPivotOffset),
                        RoomTemplate.PillarMesh, RoomTemplate.RoomPillarMeshMaterialOverride);
                    AlignActorWithWorld(Actor, FloorPoints);
                }
        }
        FloorPoints.Empty();
        }

    }

}

TArray<FVector> ADungeonGenerator::GetRoomPointsCloseToCornersLocatoin(TArray<FVector>& RoomPoints)
{
    TArray<FVector> ResultPoints;
    FVector RomCenter = GetRoomCenter(RoomPoints);
    float MaxDistanceToCorner = 0;
    // Find distances. Max distance will be equal corner point
    for (auto& pnt : RoomPoints) {
        float distance = FVector::DistSquaredXY(pnt, RomCenter);
        if (distance > MaxDistanceToCorner) {
        MaxDistanceToCorner = distance;
        }
    }
    // Write 4 corners points
    for (auto& pnt : RoomPoints) {
        float distance = FVector::DistSquaredXY(pnt, RomCenter);
        if ((int)distance >= (int)MaxDistanceToCorner) {
        ResultPoints.Add(pnt);
        }
    }
    // Find closest point to corner points
    float DistanceThreashold = 200;
    TArray<FVector> CloseToCornerPoints;
    for (auto& crn : ResultPoints) {
        for (auto& pnt : RoomPoints) {
        float distance = FVector::DistSquaredXY(pnt, crn);
        if (distance < DistanceThreashold) {
                CloseToCornerPoints.Add(pnt);
        }
        }
    }
    // Merge Arrays
    for (auto pnt : CloseToCornerPoints) {
        ResultPoints.Add(pnt);
    }
    return ResultPoints;
}

void ADungeonGenerator::AlignActorWithWorld(AActor* Actor, const TArray<FVector>& WallSpawnPoints)
{
    if (!Actor || WallSpawnPoints.Num() == 0) {
        // Ensure the actor and wall spawn points are valid
        return;
    }

    // Find the closest wall to the actor
    FVector ActorLocation = Actor->GetActorLocation();
    FVector ClosestWallLocation;
    float MinDistanceSquared = MAX_FLT;

    for (const FVector& WallSpawnPoint : WallSpawnPoints) {
        float DistanceSquared = FVector::DistSquared(ActorLocation, WallSpawnPoint);
        if (DistanceSquared < MinDistanceSquared) {
        MinDistanceSquared = DistanceSquared;
        ClosestWallLocation = WallSpawnPoint;
        }
    }
    
    // Make actor looking to wall
    FVector DirectionToWall = ActorLocation - GetRoomCenter(WallSpawnPoints);
    FRotator NewRotation = DirectionToWall.Rotation();
    // Rotate actotr to 180 degree
    Actor->SetActorRotation(NewRotation - FRotator(0,180,0));

    FRotator ActorRotation = Actor->GetActorRotation();

    FRotator Left = FVector::LeftVector.Rotation();
    float LeftAlignmentDegree = FQuat::FindBetweenNormals(ActorRotation.Vector(), Left.Vector()).GetAngle();

    FRotator Right = FVector::RightVector.Rotation();
    float RightAlignmentDegree = FQuat::FindBetweenNormals(ActorRotation.Vector(), Right.Vector()).GetAngle();

    FRotator Forward = FVector::ForwardVector.Rotation();
    float ForwardAlignmentDegree = FQuat::FindBetweenNormals(ActorRotation.Vector(), Forward.Vector()).GetAngle();

    FRotator Backward = (-FVector::ForwardVector).Rotation();
    float BackwardAlignmentDegree = FQuat::FindBetweenNormals(ActorRotation.Vector(), Backward.Vector()).GetAngle();

    // Find the smallest alignment degree
    float SmallestAlignmentDegree = LeftAlignmentDegree;
    if (RightAlignmentDegree < SmallestAlignmentDegree) 
    {
        SmallestAlignmentDegree = RightAlignmentDegree;
    }
    if (ForwardAlignmentDegree < SmallestAlignmentDegree) 
    {
        SmallestAlignmentDegree = ForwardAlignmentDegree;
    }
    if (BackwardAlignmentDegree < SmallestAlignmentDegree) 
    {
        SmallestAlignmentDegree = BackwardAlignmentDegree;
    }

    // Set the actor's rotation based on the smallest alignment degree
    if (SmallestAlignmentDegree == LeftAlignmentDegree) {
        Actor->SetActorRotation(Left);
    } else if (SmallestAlignmentDegree == RightAlignmentDegree) {
        Actor->SetActorRotation(Right);
    } else if (SmallestAlignmentDegree == ForwardAlignmentDegree) {
        Actor->SetActorRotation(Forward);
    } else if (SmallestAlignmentDegree == BackwardAlignmentDegree) {
        Actor->SetActorRotation(Backward);
    }
}

FVector ADungeonGenerator::PushSpawnPointToCenter(FVector SpawnPoint, const TArray<FVector>& WallSpawnPoints)
{
    if (WallSpawnPoints.Num() == 0) {
        // Ensure the actor and wall spawn points are valid
        return SpawnPoint;
    }

    // Find the closest wall to the actor
    FVector ActorLocation = SpawnPoint;
    FVector ClosestWallLocation;
    float MinDistanceSquared = MAX_FLT;

    for (const FVector& WallSpawnPoint : WallSpawnPoints) {
        float DistanceSquared = FVector::DistSquared(ActorLocation, WallSpawnPoint);
        if (DistanceSquared < MinDistanceSquared) {
        MinDistanceSquared = DistanceSquared;
        ClosestWallLocation = WallSpawnPoint;
        }
    }

    // Calculate the direction vector from the actor to the center of the room
    FVector DirectionToCenter = GetRoomCenter(WallSpawnPoints) - ActorLocation; // Replace 'CenterOfRoom' with the actual center of the room

    // Project the direction vector onto the plane defined by the wall (remove the component perpendicular to the wall)
    // DirectionToCenter -= FVector::DotProduct(DirectionToCenter, ClosestWallLocation) * ClosestWallLocation;

    // Normalize the direction vector to get the movement direction
    DirectionToCenter.Normalize();

    // Set the new actor location
    float PushDistance = 200.0f; // Adjust this value based on your requirements
    FVector NewLocation = ActorLocation + DirectionToCenter * PushDistance;

    return NewLocation;
}

// Helper function to calculate the center of a room
FVector ADungeonGenerator::GetRoomCenter(const TArray<FVector>& RoomPoints) const
{
    if (RoomPoints.Num() == 0) {
        return FVector::ZeroVector; // Return zero vector for an empty array
    }

    float MaxDistanceSquared = 0.0f;
    FVector Center(0.0f, 0.0f, 0.0f);

    // Find the pair of points with the maximum distance
    for (int32 i = 0; i < RoomPoints.Num() - 1; ++i) {
        for (int32 j = i + 1; j < RoomPoints.Num(); ++j) {
            float DistanceSquared = FVector::DistSquared(RoomPoints[i], RoomPoints[j]);
            if (DistanceSquared > MaxDistanceSquared) {
                MaxDistanceSquared = DistanceSquared;
                Center = (RoomPoints[i] + RoomPoints[j]) / 2.0f; // Midpoint of the pair
            }
        }
    }

    return Center;
}

bool ADungeonGenerator::IsPointCloseToWall(const FVector& Point, const TArray<FTileMatrix::FWallSpawnPoint>& WallSpawnPoints, bool bIsWallFacingX, FVector& OutModifiedOffset, FRotator& OutRotation, bool& bShouldPlaceObject) const
{
    const float DistanceThreshold = 200.0f; // Adjust this based on your requirements
    const float ObjectPlacementChance = 0.1f; // Adjust this based on your desired chance

    for (const FTileMatrix::FWallSpawnPoint& WallSpawnPoint : WallSpawnPoints) {
        float Distance = FVector::Dist(Point, WallSpawnPoint.WorldLocation);

        if (Distance < DistanceThreshold) {
            // Check if the point is in the corridor
            if (IsPointInCorridor(Point, WallSpawnPoints)) {
                bShouldPlaceObject = false; // Point is in the corridor, don't place object
                return false;
            }

            // Point is close to a wall, calculate offsets and rotation
            OutModifiedOffset = FVector(); // Add your offset calculation logic here
            OutRotation = CalculateWallRotation(bIsWallFacingX, WallSpawnPoint, FVector::ZeroVector, OutModifiedOffset);

            // Check if the object should be placed based on a random chance
            bShouldPlaceObject = FMath::FRand() < ObjectPlacementChance;

            return true;
        }
    }

    // Point is not close to a wall
    bShouldPlaceObject = true; // Object can be placed
    return false;
}

// Function to check if a point is in the corridor
bool ADungeonGenerator::IsPointInCorridor(const FVector& Point, const TArray<FTileMatrix::FWallSpawnPoint>& WallSpawnPoints) const
{
    // Choose a distance threshold for corridor detection
    const float CorridorThreshold = 300.f;

    for (const FTileMatrix::FWallSpawnPoint& WallSpawnPoint : WallSpawnPoints) {
        float Distance = FVector::Dist(Point, WallSpawnPoint.WorldLocation);

        if (Distance < CorridorThreshold) {
            return true; // Point is in the corridor
        }
    }

    return false; // Point is not in the corridor
}

void ADungeonGenerator::SpawnGenericDungeon(const TArray<FVector>& FloorTileLocations, const TArray<FTileMatrix::FWallSpawnPoint>& WallSpawnPoints)
{
    for (int32 i = 0; i < FloorTileLocations.Num(); i++) {
        // Draw debug boxes if needed
#if WITH_EDITOR
        if (bDebugActive) {
            DrawDebugBox(GetWorld(), FloorTileLocations[i], DebugVertexBoxExtents, DefaultFloorSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
            DrawDebugBox(GetWorld(), FloorTileLocations[i] + FloorPivotOffset, DebugVertexBoxExtents, OffsetedFloorSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
        }
#endif

        SpawnDungeonMesh(FTransform(FRotator::ZeroRotator, FloorTileLocations[i] + FloorPivotOffset), FloorSM);
    }
    for (int32 i = 0; i < WallSpawnPoints.Num(); i++) {

        FVector WallModifiedOffset = FVector();
        FRotator WallRotation = CalculateWallRotation(bWallFacingX, WallSpawnPoints[i], WallSMPivotOffset, WallModifiedOffset);
        FVector WallSpawnPoint = WallSpawnPoints[i].WorldLocation + WallModifiedOffset;

        // Draw debug boxes if needed
#if WITH_EDITOR
        if (bDebugActive) {
            DrawDebugBox(GetWorld(), WallSpawnPoints[i].WorldLocation, DebugVertexBoxExtents, DefaultWallSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
            DrawDebugBox(GetWorld(), WallSpawnPoint, DebugVertexBoxExtents, OffsetedWallSpawnLocationColor.ToFColor(true), true, 1555.f, 15);
        }
#endif

        SpawnDungeonMesh(FTransform(WallRotation, WallSpawnPoint), WallSM);
    }

    if (OnDungeonSpawned.IsBound()) {
        OnDungeonSpawned.Broadcast();
    }
}

void ADungeonGenerator::FaceActorToPoint(AActor* Actor, FVector Point)
{
    if (Actor) 
    {
      FVector ActorLocation = Actor->GetActorLocation();
      // Make actor looking to wall
      FVector DirectionToPoint = Point - ActorLocation;
      FRotator NewRotation = DirectionToPoint.Rotation();
      Actor->SetActorRotation(NewRotation);
    }
}

void ADungeonGenerator::DestroyDungeonMeshes()
{
    // Erase previously spawned stuff
    TArray<AActor*> SpawnedActors;
    // const UWorld* World = GetWorld();
    UGameplayStatics::GetAllActorsOfClassWithTag(this, AActor::StaticClass(), DUNGEON_MESH_TAG, SpawnedActors);

    for (int32 i = SpawnedActors.Num() - 1; i >= 0; i--) {
        if (SpawnedActors[i]) {
            SpawnedActors[i]->Destroy();
        }
    }
}

AStaticMeshActor* ADungeonGenerator::SpawnDungeonMesh(const FTransform& InTransform, UStaticMesh* SMToSpawn, UMaterialInterface* OverrideMaterial)
{
    FActorSpawnParameters ActorSpawnParams;
    ActorSpawnParams.Owner = this;

    AStaticMeshActor* SMActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), InTransform, ActorSpawnParams);
    if (SMActor) {
        // Only needed for runtime because the editor generates warnings and doesn't assign meshes
        // Meshes will switch static if used from within the editor
        SMActor->SetMobility(EComponentMobility::Movable);

        SMActor->GetStaticMeshComponent()->SetStaticMesh(SMToSpawn);

        if (OverrideMaterial) {
            SMActor->GetStaticMeshComponent()->SetMaterial(0, OverrideMaterial);
        }

        SMActor->Tags.Add(DUNGEON_MESH_TAG);
    }
    return SMActor;
}

// Sets default values
ADungeonGenerator::ADungeonGenerator()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ADungeonGenerator::BeginPlay()
{
    Super::BeginPlay();
}

#if WITH_EDITOR
void ADungeonGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (FloorSM && bAutoFloorTileSizeGeneration) {
        FloorTileSize = CalculateFloorTileSize(*FloorSM);
    }
}
#endif

void ADungeonGenerator::GenerateDungeon()
{

    TileMatrix = FTileMatrix(TileMapRows, TileMapColumns);
    TileMatrix.MaxRandomAttemptsPerRoom = MaxRandomAttemptsPerRoom;
    TileMatrix.SetRoomSize(MinRoomSize, MaxRoomSize);

    TileMatrix.CreateRooms(RoomsToGenerate);
    DestroyDungeonMeshes();

    if (RoomTemplatesDataTable) {
        SpawnDungeonFromDataTable();
        SpawnDungeonFurnitureFromDataTable();
    } else {
        if (!FloorSM) {
            UE_LOG(DungeonGenerator, Warning, TEXT("Cannot generate dungeon"));
            UE_LOG(DungeonGenerator, Error, TEXT("Invalid FloorSM. Verify you have assigned a valid floor mesh"));
            return;
        }

        if (!WallSM) {
            UE_LOG(DungeonGenerator, Warning, TEXT("Cannot generate dungeon"));
            UE_LOG(DungeonGenerator, Error, TEXT("Invalid WallSM. Verify you have assigned a valid wall mesh"));
            return;
        }

        TArray<FVector> FloorTiles;
        TArray<FTileMatrix::FWallSpawnPoint> WallSpawnPoints;
        TileMatrix.ProjectTileMapLocationsToWorld(FloorTileSize, FloorTiles, WallSpawnPoints);
        SpawnGenericDungeon(FloorTiles, WallSpawnPoints);
    }
}

void ADungeonGenerator::SetNewRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize)
{
    MinRoomSize = NewMinRoomSize;
    MaxRoomSize = NewMaxRoomSize;
}

void ADungeonGenerator::SetNewFloorMesh(UStaticMesh* NewFloorMesh, FVector NewFloorPivotOffset, bool bAutoFloorSizeGeneration, float OverrideFloorTileSize)
{
    if (NewFloorMesh) {
        FloorSM = NewFloorMesh;

        FloorTileSize = (bAutoFloorTileSizeGeneration) ? CalculateFloorTileSize(*FloorSM) : OverrideFloorTileSize;
        FloorPivotOffset = NewFloorPivotOffset;
    }
}

void ADungeonGenerator::SetNewWallMesh(UStaticMesh* NewWallMesh, FVector NewWallSMPivotOffset, bool bIsWallFacingX /*= true*/)
{
    if (NewWallMesh) {
        WallSM = NewWallMesh;
        WallSMPivotOffset = NewWallSMPivotOffset;
        bWallFacingX = bIsWallFacingX;
    }
}
