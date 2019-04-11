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

// AkSyncLoader.h

/// \file 
/// Class for synchronous calls of asynchronous models

#ifndef _AK_SYNC_CALLER_H_
#define _AK_SYNC_CALLER_H_

#include <AK/Tools/Common/AkPlatformFuncs.h>

namespace AK
{
	namespace SoundEngine
	{
		/// AkSyncLoader: Init to create a sync event, call the asynchronous method, passing
		/// it the address of this object as the cookie, then call Wait. 
		class AkSyncCaller
		{
		public:
			/// Initialize.
			AKRESULT Init()
			{
				if ( AKPLATFORM::AkCreateEvent( m_hEvent ) != AK_Success )
				{
					AKASSERT( !"Could not create synchronization event" );
					return AK_Fail;
				}
				return AK_Success;
			}

			/// Wait until the async function calls its callback.
			AKRESULT Wait( AKRESULT in_eResult )
			{
				if ( in_eResult != AK_Success )
				{
					AKPLATFORM::AkDestroyEvent( m_hEvent );
					return in_eResult;
				}

				// Task queueing successful. Block until completion.
				AKPLATFORM::AkWaitForEvent( m_hEvent );
				AKPLATFORM::AkDestroyEvent( m_hEvent );

				return m_eResult;
			}

			/// Call this from callback to release blocked thread.
			inline void Done() { AKPLATFORM::AkSignalEvent( m_hEvent ); }

			AKRESULT	m_eResult;	///< Operation result

		private:
			AkEvent		m_hEvent;	///< Sync event
		};
	}
}

#endif // _AK_SYNC_CALLER_H_
