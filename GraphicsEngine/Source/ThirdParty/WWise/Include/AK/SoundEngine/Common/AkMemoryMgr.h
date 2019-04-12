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
/// Memory Manager namespace.

#ifndef _AKMEMORYMGR_H_
#define _AKMEMORYMGR_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngineExport.h>

#ifndef AK_OPTIMIZED

#define AK_MAX_MEM_POOL_NAME_SIZE 64

/// Set the debug name for a pool. This is the name shown in the Memory tab of the Advanced Profiler.
#define AK_SETPOOLNAME( _poolid, _name )				\
	if( AK_INVALID_POOL_ID != _poolid )					\
	{													\
		AK::MemoryMgr::SetPoolName( _poolid, _name );	\
	}

// #define AK_MEMDEBUG

#ifdef MSTC_SYSTEMATIC_MEMORY_STRESS
#define AK_MEMDEBUG
#endif

#else
#define AK_SETPOOLNAME(_poolid,_name)
#endif


namespace AK
{   
	/// Memory Manager namespace.
	/// \remarks The functions in this namespace are thread-safe, unless stated otherwise.
	/// \sa
	/// - \ref memorymanager
	namespace MemoryMgr
	{
		/// Memory pool statistics. 
		/// \remarks These statistics are not collected in the Release configuration of 
		/// the default memory manager implementation.
		/// \sa 
		/// - AK::MemoryMgr::GetPoolStats()
		/// - \ref memorymanager
		struct PoolStats
		{
			// Current state
			AkUInt32 uReserved;		///< Reserved memory (in bytes)
			AkUInt32 uUsed;			///< Used memory (in bytes)
			AkUInt32 uMaxFreeBlock;	///< Size of biggest free block (in bytes)

			// Statistics
			AkUInt32 uAllocs;		///< Number of Alloc calls since initialization
			AkUInt32 uFrees;		///< Number of Free calls since initialization
			AkUInt32 uPeakUsed;		///< Peak used memory (in bytes)
		};

		/// Memory pool current state. 
		/// \sa 
		/// - AK::MemoryMgr::GetPoolMemoryUsed()
		/// - \ref memorymanager
		struct PoolMemInfo
		{
			// Current state
			AkUInt32 uReserved;		///< Reserved memory (in bytes)
			AkUInt32 uUsed;			///< Used memory (in bytes)
		};


		/// Memory management debug tools.  When specified in Init, each memory allocation will have a extra tag that can be verified periodically.
		/// Enabling this will use a lot of CPU and additional memory.  This should not be enabled unless required by Audiokinetic's support.  These are enabled in Debug configuration only.
		enum DebugFlags
		{
			CheckOverwriteAtFree = 1,	///< Performs a for buffer overflow when an allocation is freed.
			CheckOverwritePerFrame = 2,	///< Performs a check for buffer overflow once per audio frame
			CheckOverwritePerVoice = 4, ///< Performs a check for buffer overflow once per audio voice			
		};

		/// Query whether the Memory Manager has been sucessfully initialized.
		/// \warning This function is not thread-safe. It should not be called at the same time as MemoryMgr::Init or MemoryMgr::Term.
		/// \return True if the Memory Manager is initialized, False otherwise
		/// \sa 
		/// - AK::MemoryMgr::Init()
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( bool, IsInitialized )();

		/// Terminate the Memory Manager.
		/// \warning This function is not thread-safe. 
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( void, Term )();

		////////////////////////////////////////////////////////////////////////
		/// @name Memory Pools
		//@{

		/// Create a new memory pool.
		/// \return The ID of the created memory pool, or AK_INVALID_POOL_ID if creation failed
		/// \aktip
		/// Refer to \ref memorymanager_pools for information about pool resource overhead.
		/// \endaktip
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AkMemPoolId, CreatePool )(
			void *			in_pMemAddress,		///< Memory address of the pool, or NULL if it should be allocated
			AkUInt32		in_uMemSize,		///< Size of the pool (in bytes)
			AkUInt32		in_uBlockSize,		///< Size of a block (in bytes)
			AkUInt32		in_eAttributes,		///< Memory pool attributes: use values of \ref AkMemPoolAttributes
			AkUInt32        in_uBlockAlign = 0	///< Alignment of memory blocks. 
												///< Some plug-ins and specific processors may require specific data alignment.
												///< When allocating space for sound bank data, we recommend using AK_BANK_PLATFORM_DATA_ALIGNMENT.
			);

#ifdef AK_SUPPORT_WCHAR
		/// Set the name of a memory pool.
		/// \return AK_Success if successful
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AKRESULT, SetPoolName )( 
			AkMemPoolId     in_poolId,			///< ID of memory pool
			const wchar_t*	in_pszPoolName		///< Pointer to unicode name string
			);
#endif //AK_SUPPORT_WCHAR

		/// Set the name of a memory pool.
		/// \return AK_Success if successful
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AKRESULT, SetPoolName )( 
			AkMemPoolId     in_poolId,			///< ID of memory pool
			const char*		in_pszPoolName		///< Pointer to name string
			);

		/// Get the name of a memory pool.
		/// \return A pointer to the name of the memory pool (NULL if the operation failed)
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AkOSChar*, GetPoolName )( 
			AkMemPoolId     in_poolId			///< ID of memory pool
			);
		
        /// Enables or disables error notifications posted by a memory pool.
        /// The notifications are enabled by default when creating a pool.
        /// They are always disabled in the Release build.
        /// \return AK_Success if the pool exists
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AKRESULT, SetMonitoring )(
            AkMemPoolId     in_poolId,			///< ID of memory pool
            bool            in_bDoMonitor       ///< Enables error monitoring (has no effect in Release build)
            );

	    /// Destroy a memory pool.
		/// \return AK_Success if successful
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( AKRESULT, DestroyPool )(
			AkMemPoolId     in_poolId			///< ID of memory pool
		    );

        /// Get a memory pool's statistics.
		/// \sa
		/// - AK::MemoryMgr::PoolStats
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( AKRESULT, GetPoolStats )(
			AkMemPoolId     in_poolId,			///< ID of memory pool
			PoolStats&      out_stats			///< Returned statistics structure
		    );

		/// Get a memory pool current used size.
		/// Mostly used by the memory threshold features.
		/// If this function cannot be implemented if your memory manager, at least set the member uUsed to 0, that
		/// will disable the memory threshold feature.
		/// \sa
		/// - AK::MemoryMgr::PoolMemInfo
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( void, GetPoolMemoryUsed )(
			AkMemPoolId     in_poolId,			///< ID of memory pool
			PoolMemInfo&    out_memInfo			///< Returned statistics structure
		    );

		/// Get the current number of memory pools.
		/// \return The current number of memory pools
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AkInt32, GetNumPools )();

		/// Get the maximum number of memory pools.
		/// \return The maximum number of memory pools
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AkInt32, GetMaxPools )();

		/// Test the validity of a pool ID.
		/// This is used to verify the validity of a memory pool ID.
		/// \return AK_Success if the pool exists, AK_InvalidID otherwise
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AKRESULT, CheckPoolId )(
			AkMemPoolId		in_poolId			///< ID of memory pool to test
			);

		/// Get pool attributes.
		/// \return The memory pool's attributes.
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC( AkMemPoolAttributes, GetPoolAttributes )(
			AkMemPoolId		in_poolId			///< ID of memory pool
			);

		//@}

		////////////////////////////////////////////////////////////////////////
		/// @name Memory Allocation
		//@{

#if defined (AK_MEMDEBUG)
		/// Allocate memory from a pool: debug version.
		/// \return A pointer to the start of the allocated memory (NULL if the system is out of memory)
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( void *, dMalloc )(
			AkMemPoolId in_poolId,				///< ID of the memory pool
		    size_t		in_uSize,				///< Number of bytes to allocate
		    const char *in_pszFile,				///< Debug file name
		    AkUInt32	in_uLine				///< Debug line number
			);		
#endif
		/// Allocate memory from a pool.
		/// \return A pointer to the start of the allocated memory (NULL if the system is out of memory)
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( void *, Malloc )(
			AkMemPoolId in_poolId,				///< ID of the memory pool
		    size_t		in_uSize 				///< Number of bytes to allocate
		    );

		/// Reallocate memory from a pool.
		/// \return A pointer to the start of the allocated memory (NULL if the system is out of memory)
		/// \sa
		/// - \ref memorymanager
		AK_EXTERNAPIFUNC(void *, Realloc)(
			AkMemPoolId in_poolId,				///< ID of the memory pool
			void *in_pAlloc,					///< Pointer to the start of the allocated memory
			size_t		in_uSize 				///< Number of bytes to allocate
			);

		/// Free memory from a pool.
		/// \return AK_Success if successful
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( AKRESULT, Free )(
			AkMemPoolId in_poolId,				///< ID of the memory pool
			void *		in_pMemAddress			///< Pointer to the start of memory allocated with Malloc
		    );

#if defined (AK_MEMDEBUG)
		/// Allocate memory from a pool, overriding the pool's default memory alignment. Needs to be used
		/// in conjunction with AK::MemoryMgr::Falign. debug version.
		/// \return A pointer to the start of the allocated memory (NULL if the system is out of memory)
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( void *, dMalign )(
			AkMemPoolId in_poolId,				///< ID of the memory pool
		    size_t		in_uSize,				///< Number of bytes to allocate
		    AkUInt32	in_uAlignment, 			///< Alignment (in bytes)
		    const char*	 in_pszFile,			///< Debug file name
		    AkUInt32	in_uLine				///< Debug line number
			);
#endif

		/// Allocate memory from a pool, overriding the pool's default memory alignment. Needs to be used
		/// in conjunction with AK::MemoryMgr::Falign.
		/// \return A pointer to the start of the allocated memory (NULL if the system is out of memory)
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( void *, Malign )(
			AkMemPoolId in_poolId,				///< ID of the memory pool
		    size_t		in_uSize, 				///< Number of bytes to allocate
		    AkUInt32	in_uAlignment 			///< Alignment (in bytes)
		    );

		/// Free memory from a pool, overriding the pool's default memory alignment. Needs to be used in
		/// conjunction with AK::MemoryMgr::Malign.
		/// 
		/// \return AK_Success if successful
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( AKRESULT, Falign )(
			AkMemPoolId in_poolId,				///< ID of the memory pool
			void *		in_pMemAddress			///< Pointer to the start of memory allocated with Malloc
		    );

		//@}

		////////////////////////////////////////////////////////////////////////
		/// @name Fixed-Size Blocks Memory Allocation Mode
		//@{

		/// Get a block from a Fixed-Size Block type pool. To be used with pools created with AkFixedSizeBlocksMode
		/// block management type, along with any of the block allocation types.
		/// \return A pointer to the start of the allocated memory (NULL if the system is out of memory)
		///			The size of the memory block is always in_uBlockSize, specified in AK::MemoryMgr::CreatePool.
		/// \warning This method is not thread-safe. Fixed-Size Block pool access must be protected.
		/// \sa 
		/// - AK::MemoryMgr::CreatePool
		/// - AkMemPoolAttributes
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( void *, GetBlock )(
			AkMemPoolId in_poolId				///< ID of the memory pool
		    );

		/// Free memory from a Fixed-Size Block type pool.
		/// \return AK_Success if successful
		/// \warning This method is not thread-safe. Fixed-Size Block pool access must be protected.
		/// \sa
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( AKRESULT, ReleaseBlock )(
			AkMemPoolId in_poolId,				///< ID of the memory pool
			void *		in_pMemAddress			///< Pointer to the start of memory allocated with Malloc
		    );

		/// Get block size of blocks obtained with GetBlock() for a given memory pool.
		/// The block size is fixed and set when creating a pool with AkFixedSizeBlocksMode.
		/// \return Block size
		/// \sa
		/// - AK::MemoryMgr::CreatePool
		/// - AK::MemoryMgr::GetBlock
		/// - \ref memorymanager
	    AK_EXTERNAPIFUNC( AkUInt32, GetBlockSize )(
			AkMemPoolId in_poolId				///< ID of the memory pool
			);

		/// Called to start profiling memory usage for one thread (the calling thread).
		/// \note Not implementing this will result in the Soundbank tab of the Wwise Profiler to show 0 bytes for memory usage.
		AK_EXTERNAPIFUNC( void, StartProfileThreadUsage) (
			AkMemPoolId in_PoolId	///< Pool to profile
			);

		/// Called to stop profiling memory usage for the current thread.
		/// \return The amount of memory allocated by this thread since \ref StartProfileThreadUsage was called.
		/// \note Not implementing this will result in the Soundbank tab of the Wwise Profiler to show 0 bytes for memory usage.
		AK_EXTERNAPIFUNC( AkUInt32, StopProfileThreadUsage ) (
			AkMemPoolId in_PoolId	///< Pool to profile
			);


		/// Debugging method that verifies if buffer overflow occurred in a specific pool.
		/// Called at various moments depending on the DebugFlags set in AkMemSettings.
		/// In the default implementation it is not called in Release otherwise will assert if overrun found.
		/// Implementation is not mendatory if the MemoryMgr is overriden.
		AK_EXTERNAPIFUNC(void, CheckForOverwrite) (
			AkUInt32 in_uPoolID
			);		

#if defined (AK_MEMDEBUG)
		/// Debugging method that dumps a snapshot list of actual memory allocations to a file.
		/// The list contains the size allocated and the source file and line number where the memory allocation was done.
		AK_EXTERNAPIFUNC(void, DumpToFile) (const char* strFileName = "AkMemDump.txt");
#endif
		//@}
    }
}

#endif // _AKMEMORYMGR_H_
