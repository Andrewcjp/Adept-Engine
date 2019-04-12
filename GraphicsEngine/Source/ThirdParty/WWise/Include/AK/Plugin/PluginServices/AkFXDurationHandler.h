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
 
#ifndef _AKFXDURATIONHANDLER_H_
#define _AKFXDURATIONHANDLER_H_

#include <AK/SoundEngine/Common/AkTypes.h>

/// Duration handler service for source plug-in.
/// Duration may change between different execution
class AkFXDurationHandler
{
public:

	/// Setup duration handler
	inline void Setup(	
		AkReal32 in_fDuration,		///< Duration (in secs)
		AkInt16 in_iLoopingCount,	///< Number of loop iterations (0 == infinite)
		AkUInt32 in_uSampleRate 	///< Sample rate
		)
	{
		m_uSampleRate = in_uSampleRate;
		SetDuration( in_fDuration );
		SetLooping( in_iLoopingCount );
		Reset();
	}

	/// Reset looping and frame counters and start again.
	inline void Reset()
	{
		m_uFrameCount = 0;
	}

	/// Change number of loop iterations (0 == infinite).
	inline void SetLooping( AkInt16 in_iNumLoops )
	{
		m_iNumLoops = in_iNumLoops;
	}

	/// Set current duration per iteration (in secs).
	inline void SetDuration( AkReal32 in_fDuration )
	{
		m_uIterationFrame = (AkUInt32) (in_fDuration*m_uSampleRate);
		m_uIterationFrame = (m_uIterationFrame + 3) & ~3; // Align to next 4 frame boundary for SIMD alignment
	}

	/// Return current total duration (considering looping) in secs.
	inline AkReal32 GetDuration() const
	{
		// Note: Infinite looping will return a duration of 0 secs.
		return (AkReal32)(m_uIterationFrame*m_iNumLoops)/m_uSampleRate;
	}

	/// Set current number of frames to be produced (validFrames) 
	/// and output state of audio buffer and advance internal state.
	inline void ProduceBuffer( AkAudioBuffer * io_pBuffer )
	{
		io_pBuffer->eState = ProduceBuffer( io_pBuffer->MaxFrames(), io_pBuffer->uValidFrames );
	}

	/// Set current number of frames to be produced (validFrames) 
	/// and output state of audio buffer and advance internal state.
	inline AKRESULT ProduceBuffer( AkUInt16 in_uMaxFrames, AkUInt16 & out_uValidFrames )
	{
		// Infinite looping or not reached the end, always producing full capacity
		out_uValidFrames = in_uMaxFrames;
		AKRESULT eState = AK_DataReady;

		if ( m_iNumLoops != 0 )
		{
			// Finite looping, produce full buffer untill the end.
			const AkUInt32 uTotalFrames = m_iNumLoops*m_uIterationFrame;
			if ( m_uFrameCount < uTotalFrames )
			{
				const AkUInt32 uFramesRemaining = uTotalFrames-m_uFrameCount;
				if ( uFramesRemaining <= in_uMaxFrames )
				{
					out_uValidFrames = (AkUInt16)uFramesRemaining;
					eState = AK_NoMoreData;
				}
			}
			else
			{
				out_uValidFrames = 0;
				eState = AK_NoMoreData;
			}
		}	
		m_uFrameCount += out_uValidFrames;

		return eState;
	}

protected:

	AkUInt32	m_uIterationFrame;	// Number of frames in a single loop iteration 
	AkUInt32	m_uFrameCount;		// Number of frames output in the current iteration
	AkUInt32	m_uSampleRate;		// Sample rate used to convert time to samples
	AkInt16		m_iNumLoops;		// Number of loop iterations (0 == infinite looping)
	
};

#endif // _AKFXDURATIONHANDLER_H_
