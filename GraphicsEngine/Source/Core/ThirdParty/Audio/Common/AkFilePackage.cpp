/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2018.1.2  Build: 6762
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/
//////////////////////////////////////////////////////////////////////
// 
// AkFilePackage.h
//
// This class represents a file package that was created with the 
// AkFilePackager utility app (located in ($WWISESDK)/samples/FilePackager/). 
// It holds a system file handle and a look-up table (CAkFilePackageLUT).
//
// CAkFilePackage objects can be chained together using the ListFilePackages
// typedef defined below.
//
//////////////////////////////////////////////////////////////////////


#include "AkFilePackage.h"

// Destroy file package and free memory / destroy pool.
void CAkFilePackage::Destroy()
{
	// Cache memory pointer and pool ID because memory pool is destroyed _after_ deleting this.
	AkMemPoolId	poolID	= m_poolID;
	void * pToRelease	= m_pToRelease;
	bool bIsInternalPool = m_bIsInternalPool;

	// Call destructor.
	this->~CAkFilePackage();

	// Free memory.
	ClearMemory( poolID, pToRelease, bIsInternalPool );
}

void CAkFilePackage::ClearMemory(
	AkMemPoolId in_poolID,			// Pool to destroy.
	void *		in_pMemToRelease,	// Memory block to free before destroying pool.
	bool		in_bIsInternalPool	// Pool was created internally (and needs to be destroyed).
	)
{
	if ( in_poolID != AK_INVALID_POOL_ID )
	{
		if ( in_pMemToRelease )
		{
			if ( in_bIsInternalPool )
			{
				AK::MemoryMgr::ReleaseBlock( in_poolID, in_pMemToRelease );
				// Destroy pool
				AKVERIFY( AK::MemoryMgr::DestroyPool( in_poolID ) == AK_Success );
			}
			else
			{
				if ( AK::MemoryMgr::GetPoolAttributes( in_poolID ) & AkBlockMgmtMask )
					AK::MemoryMgr::ReleaseBlock( in_poolID, in_pMemToRelease );
				else
					AkFree( in_poolID, in_pMemToRelease );
			}
		}
		else
			AKASSERT( !in_bIsInternalPool );	// Internal pools allocation is guaranteed.
	}
}
