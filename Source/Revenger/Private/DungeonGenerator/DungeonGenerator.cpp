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

const FName ADungeonGenerator::DUNGEON_MESH_TAG = FName("dungeon_generated_actor");

float ADungeonGenerator::CalculateFloorTileSize(const UStaticMesh& Mesh) const
{
    return FMath::Abs(Mesh.GetBoundingBox().Min.Y) + FMath::Abs(Mesh.GetBoundingBox().Max.Y);
}

float ADungeonGenerator::CalculateFloorTileSize(const AActor* Actor) const
{
    if (Actor && IsValid(Actor)) {
        UStaticMesh* StaticMesh = nullptr;
        TArray<UStaticMeshComponent*> Components;
        Actor->GetComponents<UStaticMeshComponent>(Components);
        for (int32 i = 0; i < Components.Num(); i++) {
            UStaticMeshComponent* StaticMeshComponent = Components[i];
            StaticMesh = StaticMeshComponent->GetStaticMesh();
        }

        if (StaticMesh) {
            return CalculateFloorTileSize(*StaticMesh);
        }
    }
    // Return a default value or handle the case where ActorTemplate is not valid
    return 0.0f; // Or any appropriate default value
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

    FString ContextStr;
    DungeonTemplatesDataTable->GetAllRows<FDungeonRoomTemplate>(ContextStr, m_RoomTemplates);
     
    ensure(m_RoomTemplates.Num() > 0);

    float DataTableFloorTileSize = 0;
    if (AActor* FloorTileActor = SpawnActor(m_RoomTemplates[0]->RoomFloor, FVector(), FRotator())) 
    {
        FloorTileActor->Tags.Add("dungeon room tile size test");
        DataTableFloorTileSize = CalculateFloorTileSize(FloorTileActor);
        this->DestroyDungeonActors("dungeon room tile size test");
        if ((int)DataTableFloorTileSize == 0) 
        {
            UE_LOG(DungeonGenerator, Warning, TEXT("Can't calculate floor tile size !!!"), DataTableFloorTileSize);
            return;
        }
    }

    m_TileMatrix.ProjectTileMapLocationsToWorld(DataTableFloorTileSize, m_Rooms, m_CorridorFloorTiles, m_CorridorWalls);

    // Spawn rooms & walls using a random template from the provided table
    for (int32 i = 0; i < m_Rooms.Num(); i++) {
        FDungeonRoomTemplate *RoomTemplate = m_RoomTemplates[FMath::RandRange(0, m_RoomTemplates.Num() - 1)];

        // Spawn floor in rooms 
        for (int32 j = 0; j < m_Rooms[i].FloorTileWorldLocations.Num(); j++) {
            FVector WorldSpawnLocation = m_Rooms[i].FloorTileWorldLocations[j];
            SpawnActor(RoomTemplate->RoomFloor, WorldSpawnLocation, FRotator::ZeroRotator);
        }

         // Spawn walls around rooms
        for (int32 j = 0; j < m_Rooms[i].WallSpawnPoints.Num(); j++) {
            // FVector WorldSpawnLocation = Rooms[i].WallSpawnPoints[j].WorldLocation;
            FVector WallModifiedOffset = FVector();
            FVector WallMeshPivotOffset = FVector();
            FRotator WallSpawnRotation = CalculateWallRotation(RoomTemplate->bIsWallFacingX, m_Rooms[i].WallSpawnPoints[j], WallMeshPivotOffset, WallModifiedOffset);
            FVector WallSpawnLocation = m_Rooms[i].WallSpawnPoints[j].WorldLocation + WallModifiedOffset;
            SpawnActor(RoomTemplate->RoomWall, WallSpawnLocation, WallSpawnRotation);
        }
    }

    // Make coridors from random tiles
    if (FDungeonRoomTemplate* RoomTemplate = m_RoomTemplates[FMath::RandRange(0, m_RoomTemplates.Num() - 1)]) {
        // Spawn floor tiles for corridors
        for (int32 i = 0; i < m_CorridorFloorTiles.Num(); i++) {
            SpawnActor(RoomTemplate->CorridorFloor, m_CorridorFloorTiles[i], FRotator::ZeroRotator);
        }

        bool bCorridorWallFacingX = RoomTemplate->bIsWallFacingX;
        // Spawn walls for corridors
        for (int32 i = 0; i < m_CorridorWalls.Num(); i++) {
            FVector WallModifiedOffset = FVector();
            FRotator WallRotation = CalculateWallRotation(bCorridorWallFacingX, m_CorridorWalls[i], FVector::ZeroVector, WallModifiedOffset);
            FVector WallSpawnPoint = m_CorridorWalls[i].WorldLocation + WallModifiedOffset;
            SpawnActor(RoomTemplate->CorridorWall, WallSpawnPoint, WallRotation);
        }
    }
   
    SpawnDoors();
    // Randomly spawn objects in rooms
    for (const auto& room : m_Rooms) 
    {
        for (const auto& pnt : room.WallSpawnPoints) 
        {
        int RandomActorToSpawn = FMath::RandRange(0, 2);
        switch (RandomActorToSpawn) {
        case 0: {
            // Spawn cheast
            FDungeonRoomTemplate* RoomTemplate = m_RoomTemplates[FMath::RandRange(0, m_RoomTemplates.Num() - 1)];
            SpawnRoomObject(RoomTemplate->RoomChest, room, 10, 0.1, RoomTemplate->RoomChestSpawnChance);
            break;
        }
        case 1: {
            // Spawn object
            break;
        }
        case 2: {
            break;
        }
        default:
            break;
        }
        }
    }
    // Example how to spawm room object using SpawnRoomObject method
    for (const auto& room : m_Rooms) {
       
    }
    for (const auto& room : m_Rooms) {
        FDungeonRoomTemplate* RoomTemplate = m_RoomTemplates[FMath::RandRange(0, m_RoomTemplates.Num() - 1)];
        SpawnRoomObject(RoomTemplate->RoomChest, room, 10, 0.2, RoomTemplate->RoomChestSpawnChance);
    }
    for (const auto& room : m_Rooms) {
        FDungeonRoomTemplate* RoomTemplate = m_RoomTemplates[FMath::RandRange(0, m_RoomTemplates.Num() - 1)];
        SpawnRoomObject(RoomTemplate->RoomChest, room, 10, 0.3, RoomTemplate->RoomChestSpawnChance);
    }
    for (const auto& room : m_Rooms) {
        FDungeonRoomTemplate* RoomTemplate = m_RoomTemplates[FMath::RandRange(0, m_RoomTemplates.Num() - 1)];
        SpawnRoomObject(RoomTemplate->RoomChest, room, 10, 0.4, RoomTemplate->RoomChestSpawnChance);
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

void ADungeonGenerator::AlignActorWithWorld(AActor* Actor, const FVector RoomCenter)
{
    if (!Actor) {
        // Ensure the actor and wall spawn points are valid
        return;
    }

    // Find the closest wall to the actor
    FVector ActorLocation = Actor->GetActorLocation();

    // Make actor looking to wall
    FVector DirectionToWall = ActorLocation - RoomCenter;
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


FRotator ADungeonGenerator::AlignActorWithWorld(const FVector Location, const FVector RoomCenter)
{

    // Find the closest wall to the actor
    FVector ActorLocation = Location;

    // Make actor looking to wall
    FVector DirectionToWall = ActorLocation - RoomCenter;
    FRotator NewRotation = DirectionToWall.Rotation();

    // Rotate actotr to 180 degree
    FRotator ActorRotation = NewRotation - FRotator(0, 180, 0);

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
    if (RightAlignmentDegree < SmallestAlignmentDegree) {
        SmallestAlignmentDegree = RightAlignmentDegree;
    }
    if (ForwardAlignmentDegree < SmallestAlignmentDegree) {
        SmallestAlignmentDegree = ForwardAlignmentDegree;
    }
    if (BackwardAlignmentDegree < SmallestAlignmentDegree) {
        SmallestAlignmentDegree = BackwardAlignmentDegree;
    }

    // Set the actor's rotation based on the smallest alignment degree
    if (SmallestAlignmentDegree == LeftAlignmentDegree) {
        ActorRotation = Left;
    } else if (SmallestAlignmentDegree == RightAlignmentDegree) {
        ActorRotation = Right;
    } else if (SmallestAlignmentDegree == ForwardAlignmentDegree) {
        ActorRotation = Forward;
    } else if (SmallestAlignmentDegree == BackwardAlignmentDegree) {
        ActorRotation = Backward;
    }
    
    return ActorRotation;
}

FVector ADungeonGenerator::MoveVectorTowardRotation(const FVector& OriginalVector, const FRotator& Rotation, float Distance)
{
    // Convert the FRotator to a quaternion
    FQuat RotationQuat(Rotation);

    // Rotate the original vector by the quaternion
    FVector RotatedVector = RotationQuat.RotateVector(OriginalVector);

    // Move the vector along its own axis by the specified distance
    FVector MovedVector = RotatedVector + Rotation.Vector() * Distance;

    return MovedVector;
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

void ADungeonGenerator::SpawnRoomObject(TSubclassOf<AActor>& ObjectToSpawn, const FTileMatrix::FRoom& Room, int MaxAmountToSpawn, float Distance, float ChanceToSpawn)
{
    float NotSpawnNearCorridorDistance = 600;
    int SpwannedAmount = 0;

    // How far move object while spawn [Near wall - 0.01 to ... 0.5 - Room center]
    float MoveSpawnObject = FMath::Clamp(Distance, 0.01f, 0.5f);
    TArray<FVector> SpawnPoints;

    // Get wall around for calculating room center
    TArray<FVector> WallsAround;
    for (const auto& wall_pnt : Room.WallSpawnPoints) {
        WallsAround.Add(wall_pnt.WorldLocation);
    }

    // How far we can move object to center
    float RoomSizeX = 0;
    float RoomSizeY = 0;
    RoomSize(WallsAround, RoomSizeX, RoomSizeY);
    float RoomSizeMin = FMath::Min(RoomSizeX, RoomSizeY);
    FVector RoomCenter = GetRoomCenter(WallsAround);

    // Find all corridor points that are close to our rooms fllor
    // Use free space near this room walls
    for (const auto& wall_pnt : Room.WallSpawnPoints) {
        if (SpwannedAmount < MaxAmountToSpawn) {
            FRotator AlignedRotation = AlignActorWithWorld(wall_pnt.WorldLocation, RoomCenter);
            FVector SpawnLocation = (wall_pnt.WorldLocation + AlignedRotation.Vector() * (RoomSizeMin * MoveSpawnObject));
            // Get corridor point if it close don't spawn
            bool CanSpawnHere = true;
            for (const auto& corridor_pnt : m_CorridorFloorTiles) {
                if (FVector::DistXY(wall_pnt.WorldLocation, corridor_pnt) < NotSpawnNearCorridorDistance) {
                    CanSpawnHere = false;
                }
            }
            if (CanSpawnHere) {
                // Check  if there no spawned object near seleceted places
                for (const auto& occ_pnt : m_OccupiedPoints) {
                    if (FVector::DistXY(SpawnLocation, occ_pnt) < 400.f) {
                        CanSpawnHere = false;
                    }
                }
                if (CanSpawnHere) {
                    if (ChanceToSpawn > FMath::RandRange(0.f, 0.99f)) {
                        SpawnActor(ObjectToSpawn, SpawnLocation, AlignedRotation);
                        m_OccupiedPoints.Add(SpawnLocation);
                    }
                    SpwannedAmount++;
                }
            }
        }
    }
}

AActor* ADungeonGenerator::SpawnActor(AActor* Actor, FVector& Location, FRotator& Rotation)
{
    if (!Actor) {
        // Handle error, the template actor is invalid
        return nullptr;
    }
    UClass* ActorClass = Actor->GetClass();
    AActor* NewActor = nullptr;
    if (UWorld* World = GetWorld()) {
        NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation);
        if (NewActor) {
            // Optionally, you can further configure the spawned actor
            // For example, set properties, attach components, etc.
            NewActor->Tags.Add(DUNGEON_MESH_TAG);
            return NewActor;
        }
    }
    return nullptr;
}

AActor* ADungeonGenerator::SpawnActor(TSubclassOf<AActor> &ActorTemplate, FVector Location, FRotator Rotation)
{
    if (ActorTemplate && ActorTemplate.Get()) {
        UWorld* World = GetWorld();
        if (World) {
            AActor* NewActor = World->SpawnActor<AActor>(ActorTemplate, Location, Rotation);
            if (NewActor) {
                // Optionally, you can further configure the spawned actor
                // For example, set properties, attach components, etc.
                NewActor->Tags.Add(DUNGEON_MESH_TAG);
                return NewActor;
            }
        }
     }
     return nullptr;
}

void ADungeonGenerator::SpawnDoors()
{
     FDungeonRoomTemplate* RoomTemplate = m_RoomTemplates[FMath::RandRange(0, m_RoomTemplates.Num() - 1)];
     float DistanceTreashold = 410;
     TArray<FVector> SpawnPoints;
     TArray<FVector> NearRoomTilePoints;
     TArray<FRotator> FaceToPoint;
     // Find all corridor points that are close to our rooms fllor
     for (const auto& room_pnts : m_Rooms) {
        for (const auto& room_pnt : room_pnts.FloorTileWorldLocations) {
            for (const auto& corridor_pnt : m_CorridorFloorTiles) {

                // Spawn shold be only in narrow corridor
                // Check near walls
                int NeighborWalls = 0;
                for (const auto& wall_pnt : m_CorridorWalls) {
                    if (FVector::DistXY(corridor_pnt, wall_pnt.WorldLocation) < 410) {
                        ++NeighborWalls;
                    }
                }

                if (FVector::DistXY(room_pnt, corridor_pnt) < 410 && NeighborWalls > 1) {
                    FRotator SpawnRotation = (room_pnt - corridor_pnt).Rotation();
                    SpawnPoints.Add(corridor_pnt);
                    FaceToPoint.Add(SpawnRotation);
                    NearRoomTilePoints.Add(room_pnt);
                }
            }
        }
     }
     // Spawn doors
     for (int i = 0; i < SpawnPoints.Num(); i++) {
        float Distance = FVector::Dist(SpawnPoints[i], NearRoomTilePoints[i]);
        // Calculate the new position that is 40% of the distance towards the target
        FVector NewPosition = FMath::Lerp(SpawnPoints[i], NearRoomTilePoints[i], 0.5f);
        m_OccupiedPoints.Add(NewPosition);
        SpawnActor(RoomTemplate->CorridorDoors, NewPosition, FaceToPoint[i] - FRotator(0, 90, 0));
     }
}


// Find room size. Useful for pushing items to center treashold. This help to find max distance to push.
void ADungeonGenerator::RoomSize(TArray<FVector>& ArrayOfRoomPoints, float& XLength, float& YLength)
{
     if (ArrayOfRoomPoints.Num() == 0) {
        // Handle the case where the array is empty
        XLength = 0;
        YLength = 0;
        return;
     }

     // Initialize min and max values with the first point
     float MinX = ArrayOfRoomPoints[0].X;
     float MinY = ArrayOfRoomPoints[0].Y;
     float MaxX = ArrayOfRoomPoints[0].X;
     float MaxY = ArrayOfRoomPoints[0].Y;

     // Iterate through the array to find min and max values
     for (const FVector& Point : ArrayOfRoomPoints) {
        MinX = FMath::Min(MinX, Point.X);
        MinY = FMath::Min(MinY, Point.Y);
        MaxX = FMath::Max(MaxX, Point.X);
        MaxY = FMath::Max(MaxY, Point.Y);
     }

     // Calculate the square size
     XLength = FMath::CeilToFloat(MaxX - MinX);
     YLength = FMath::CeilToFloat(MaxY - MinY);
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

void ADungeonGenerator::DestroyDungeonActors()
{
    // Clear Arrays before start to generate something
    m_RoomTemplates.Empty();
    m_Rooms.Empty();
    m_CorridorFloorTiles.Empty();
    m_CorridorWalls.Empty();
    m_OccupiedPoints.Empty();
    DestroyDungeonActors(DUNGEON_MESH_TAG);
}


void ADungeonGenerator::DestroyDungeonActors(FName Tag)
{
    // Erase previously spawned stuff
    TArray<AActor*> SpawnedActors;
    // const UWorld* World = GetWorld();
    UGameplayStatics::GetAllActorsOfClassWithTag(this, AActor::StaticClass(), Tag, SpawnedActors);

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

UStaticMesh* ADungeonGenerator::GetStaticMesh(AActor *Actor)
{
    if (Actor) 
    {
        // Check if the actor has a StaticMeshComponent
        UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (StaticMeshComp) {
            // Access the static mesh
            UStaticMesh* Mesh = StaticMeshComp->GetStaticMesh();

            if (Mesh) {
                return Mesh;
            }
       }
    
    }
    return nullptr;
}



#endif

void ADungeonGenerator::GenerateDungeon()
{
    DestroyDungeonActors();
    m_TileMatrix = FTileMatrix(TileMapRows, TileMapColumns);
    m_TileMatrix.MaxRandomAttemptsPerRoom = MaxRandomAttemptsPerRoom;
    m_TileMatrix.SetRoomSize(MinRoomSize, MaxRoomSize);
    m_TileMatrix.CreateRooms(RoomsToGenerate);

    if (DungeonTemplatesDataTable) {
        SpawnDungeonFromDataTable();
        //SpawnDungeonFurnitureFromDataTable();
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
        m_TileMatrix.ProjectTileMapLocationsToWorld(FloorTileSize, FloorTiles, WallSpawnPoints);
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
