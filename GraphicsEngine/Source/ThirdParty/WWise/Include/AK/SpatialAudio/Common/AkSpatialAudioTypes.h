/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

Version: v2018.1.2  Build: 6762
Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

/// \file 
/// Spatial audio data type definitions.

#pragma once

#include <AK/Tools/Common/AkKeyArray.h>
#include <AK/Tools/Common/AkSet.h>
#include <AK/Tools/Common/AkString.h>
#include <AK/Tools/Common/AkLock.h>

class AkAcousticRoom;
class AkAcousticPortal;
class AkImageSourceTriangle;
class AkImageSourcePlane;

#define AK_MAX_REFLECT_ORDER 4
#define AK_MAX_REFLECTION_PATH_LENGTH (AK_MAX_REFLECT_ORDER + 2)
#define AK_MAX_SOUND_PROPAGATION_DEPTH 8
#define AK_DEFAULT_DIFFR_SHADOW_DEGREES (30.0f)
#define AK_DEFAULT_DIFFR_SHADOW_ATTEN (2.0f)
#define AK_SA_EPSILON (0.001f)
#define AK_SA_PLANARITY_EPSILON (0.01f)

const AkReal32 kDefaultMaxPathLength = 100.f;

const AkReal32 kMaxDiffraction = 1.0f;

extern AkMemPoolId g_SpatialAudioPoolId;

AK_DEFINE_ARRAY_POOL(_ArrayPoolSpatialAudio, g_SpatialAudioPoolId);
typedef AkArrayAllocatorNoAlign<_ArrayPoolSpatialAudio> ArrayPoolSpatialAudio;
typedef AkArrayAllocatorAlignedSimd<_ArrayPoolSpatialAudio> ArrayPoolSpatialAudioSIMD;

namespace AK
{
	namespace SpatialAudio
	{
		typedef AkString<ArrayPoolSpatialAudio, wchar_t> WString;		///< Wide string type for use in Wwise Spatial Audio
		typedef AkString<ArrayPoolSpatialAudio, AkOSChar> OsString;		///< OS string type for use in Wwise Spatial Audio
		typedef AkString<ArrayPoolSpatialAudio, char> String;			///< String type for use in Wwise Spatial Audio
		typedef AkDbString<ArrayPoolSpatialAudio, char, CAkLock> DbString; ///< Instanced string type.

		typedef AkUInt16 Idx;
	}
}

typedef AkUInt16 AkVertIdx;
typedef AkUInt16 AkTriIdx;
typedef AkUInt16 AkSurfIdx;
typedef AkUInt16 AkEdgeIdx;

#define AK_INVALID_VERTEX ((AkVertIdx)(-1))
#define AK_INVALID_TRIANGLE ((AkTriIdx)(-1))
#define AK_INVALID_SURFACE ((AkSurfIdx)(-1))
#define AK_INVALID_EDGE ((AkEdgeIdx)(-1))

/// Base type for ID's used by Wwise spatial audio.  
struct AkSpatialAudioID
{
	/// Default constructor.  Creates an invalid ID.
	AkSpatialAudioID() : id((AkUInt64)-1) {}
	
	/// Construct from a 64-bit int.
	AkSpatialAudioID(AkUInt64 _id) : id(_id) {}

	/// Conversion from a pointer to a AkSpatialAudioID
	explicit AkSpatialAudioID(const void * ptr) : id(reinterpret_cast<AkUInt64>(ptr)) {}

	bool operator == (AkSpatialAudioID rhs) const { return id == rhs.id; }
	bool operator != (AkSpatialAudioID rhs) const { return id != rhs.id; }
	bool operator < (AkSpatialAudioID rhs) const { return id < rhs.id; }
	bool operator > (AkSpatialAudioID rhs) const { return id > rhs.id; }
	bool operator <= (AkSpatialAudioID rhs) const { return id <= rhs.id; }
	bool operator >= (AkSpatialAudioID rhs) const { return id >= rhs.id; }

	/// Determine if this ID is valid.
	bool IsValid() const { return id != (AkUInt64)-1; }

	/// Conversion function used internally to convert from a AkSpatialAudioID to a AkGameObjectID.
	AkGameObjectID AsGameObjectID() const { return (AkGameObjectID)id; }
	
	operator AkUInt64 () { return id; }

	AkUInt64 id;
};

/// Spatial Audio Room ID type.  This ID type exists in the same ID-space as game object ID's.  The client is responsible for not choosing room ID's
/// that conflict with registered game objects' ID's.  Internally, the spatial audio rooms and portals API manages registration and un-registration of game objects that 
/// represent rooms using AkRoomID's provided by the client; AkRoomID's are convertied to AkGameObjectID's by calling AsGameObjectID(). 
/// \sa 
///	- \ref AK::SpatialAudio::SetRoom
///	- \ref AK::SpatialAudio::RemoveRoom
struct AkRoomID : public AkSpatialAudioID
{
	/// A game object ID that is in the reserved range, used for 'outdoor' rooms, ie when not in a room.
	static const AkGameObjectID OutdoorsGameObjID = (AkGameObjectID)-4;

	/// Default constructor.  Creates an invalid ID.
	AkRoomID() : AkSpatialAudioID() {}

	/// Construct from a 64-bit int.
	AkRoomID(AkUInt64 _id) : AkSpatialAudioID(_id) {}

	/// Conversion from a pointer to a AkRoomID
	explicit AkRoomID(const void * ptr) : AkSpatialAudioID(ptr) {}

	/// Conversion function used internally to convert AkRoomID's to AkGameObjectIDs.
	AkGameObjectID AsGameObjectID() const { return IsValid() ? (AkGameObjectID)id : OutdoorsGameObjID; }
};

///< Unique ID for portals.  This ID type exists in the same ID-space as game object ID's.  The client is responsible for not choosing portal ID's
/// that conflict with registered game objects' ID's.  Internally, the spatial audio rooms and portals API manages registration and un-registration of game objects that 
/// represent portals using AkPortalID's provided by the client; AkPortalID's are convertied to AkGameObjectID's by calling AsGameObjectID(). 
/// \sa 
///	- \ref AK::SpatialAudio::SetPortal
///	- \ref AK::SpatialAudio::RemovePortal  
typedef AkSpatialAudioID AkPortalID;		

///< Unique ID for identifying geometry sets.  Chosen by the client using any means desired.  
/// \sa 
///	- \ref AK::SpatialAudio::SetGeometry
///	- \ref AK::SpatialAudio::RemoveGeometry
typedef AkSpatialAudioID AkGeometrySetID;


