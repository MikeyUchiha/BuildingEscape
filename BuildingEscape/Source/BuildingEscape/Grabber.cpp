// Copyright © 2016 Star Fantasy Games

#include "BuildingEscape.h"
#include "Grabber.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	FindPhysicsHandleComponent();
	SetupInputComponent();
}

// Look for attached Physics Handle Component
void UGrabber::FindPhysicsHandleComponent()
{
	/// Look for attached Physics Handle
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle)
	{

	} else
	{
		UE_LOG(LogTemp, Error, TEXT("The %s object does not have a UPhysicsHandleComponent"), *GetOwner()->GetName());
	}
}

// Set up attached Input Component (only appears at run time)
void UGrabber::SetupInputComponent()
{
	/// Look for attached Input
	Input = GetOwner()->FindComponentByClass<UInputComponent>();
	if (Input)
	{
		UE_LOG(LogTemp, Warning, TEXT("The %s object has a UInputComponent"), *GetOwner()->GetName());
		/// Bind the input axis
		Input->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		Input->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("The %s object does not have a UInputComponent"), *GetOwner()->GetName());
	}
}

const FHitResult UGrabber::GetFirstPhysicsBodyInReach()
{
	/// Get player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	FVector LineTraceDirection = PlayerViewPointRotation.Vector();
	FVector LineTraceEnd = PlayerViewPointLocation + LineTraceDirection * Reach;

	/// Setup query parameters
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());

	/// Line-trace(ray-cast) out to reach distance
	FHitResult Hit;
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		PlayerViewPointLocation,
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);

	/// See what we hit
	if (Hit.GetActor())
	{
		FString ActorName = Hit.GetActor()->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Line-trace hit: %s"), *ActorName);
	}

	return Hit;
}

// Called every frame
void UGrabber::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	/// Get player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	FVector LineTraceDirection = PlayerViewPointRotation.Vector();
	FVector LineTraceEnd = PlayerViewPointLocation + LineTraceDirection * Reach;

	// if the physics handle is attached
	if (PhysicsHandle->GrabbedComponent)
	{
		// move the object that we're holding
		PhysicsHandle->SetTargetLocation(LineTraceEnd);
	}
}

void UGrabber::Grab()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab pressed"));

	/// LINE TRACE and see if we reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();
	auto ActorHit = HitResult.GetActor();

	/// If we hit something then attach a physics handle
	if (ActorHit)
	{
		PhysicsHandle->GrabComponent(
			ComponentToGrab,
			NAME_None,
			ComponentToGrab->GetOwner()->GetActorLocation(),
			true // allow rotation
		);
	}
}

void UGrabber::Release()
{
	UE_LOG(LogTemp, Warning, TEXT("Grab released"));

	/// Release physics handle
	PhysicsHandle->ReleaseComponent();
}
