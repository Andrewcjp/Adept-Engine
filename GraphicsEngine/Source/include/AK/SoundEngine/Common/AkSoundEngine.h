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

// AkSoundEngine.h

/// \file 
/// The main sound engine interface.


#ifndef _AK_SOUNDENGINE_H_
#define _AK_SOUNDENGINE_H_

#include <AK/SoundEngine/Common/AkSoundEngineExport.h>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/SoundEngine/Common/AkCallback.h>

#ifdef AK_WIN
#include <AK/SoundEngine/Platforms/Windows/AkWinSoundEngine.h>

#elif defined (AK_MAC_OS_X)
#include <AK/SoundEngine/Platforms/Mac/AkMacSoundEngine.h>

#elif defined (AK_IOS)
#include <AK/SoundEngine/Platforms/iOS/AkiOSSoundEngine.h>

#elif defined (AK_XBOXONE)
#include <AK/SoundEngine/Platforms/XboxOne/AkXboxOneSoundEngine.h>

#elif defined( AK_LUMIN )
#include <AK/SoundEngine/Platforms/Lumin/AkLuminSoundEngine.h>

#elif defined( AK_ANDROID )
#include <AK/SoundEngine/Platforms/Android/AkAndroidSoundEngine.h>

#elif defined (AK_PS4)
#include <AK/SoundEngine/Platforms/PS4/AkPS4SoundEngine.h>

#elif defined( AK_LINUX )
#include <AK/SoundEngine/Platforms/Linux/AkLinuxSoundEngine.h>

#elif defined( AK_EMSCRIPTEN )
#include <AK/SoundEngine/Platforms/Emscripten/AkEmscriptenSoundEngine.h>

#elif defined( AK_QNX  )
#include <AK/SoundEngine/Platforms/QNX/AkQNXSoundEngine.h>

#elif defined( AK_NX )
#include <AK/SoundEngine/Platforms/NX/AkNXSoundEngine.h>

#else
#error AkSoundEngine.h: Undefined platform
#endif

#ifndef AK_ASSERT_HOOK
	/// Function called on assert handling, optional
	/// \sa 
	/// - AkInitSettings
	AK_CALLBACK( void, AkAssertHook)( 
		const char * in_pszExpression,	///< Expression
		const char * in_pszFileName,	///< File Name
		int in_lineNumber				///< Line Number
		);
	#define AK_ASSERT_HOOK
#endif

/// Callback function prototype for User Music notifications
///	It is useful for reacting to user music playback.
///
/// \sa
/// - \ref AkGlobalCallbackFunc
/// - \ref AK::SoundEngine::iOS::AudioInterruptionCallbackFunc
/// - \ref AkPlatformInitSettings
/// - \ref background_music_and_dvr
///
typedef AKRESULT ( * AkBackgroundMusicChangeCallbackFunc )(
	bool in_bBackgroundMusicMuted,	///< Flag indicating whether the busses tagged as "background music" in the project are muted or not.
	void* in_pCookie ///< User-provided data, e.g. a user structure.
	);

/// Platform-independent initialization settings of output devices.
struct AkOutputSettings
{
	AkOutputSettings() :
		audioDeviceShareset(AK_INVALID_UNIQUE_ID),		
		idDevice(0),		
		ePanningRule(AkPanningRule_Speakers),
		channelConfig(){};

	AkOutputSettings(const char* in_szDeviceShareSet, AkUniqueID in_idDevice = AK_INVALID_UNIQUE_ID, AkChannelConfig in_channelConfig = AkChannelConfig(), AkPanningRule in_ePanning = AkPanningRule_Speakers);		

#ifdef AK_SUPPORT_WCHAR
	AkOutputSettings(const wchar_t* in_szDeviceShareSet, AkUniqueID in_idDevice = AK_INVALID_UNIQUE_ID, AkChannelConfig in_channelConfig = AkChannelConfig(), AkPanningRule in_ePanning = AkPanningRule_Speakers);
#endif

	AkUniqueID		audioDeviceShareset;	///< Unique ID of a custom audio device to be used. Custom audio devices are defined in the Audio Device Shareset section of the Wwise project.
											///< If you want to output normally through the default system, leave this field to its default value (AK_INVALID_UNIQUE_ID).
											///< Typical usage: AkInitSettings.eOutputSettings.audioDeviceShareset = AK::SoundEngine::GetIDFromString("InsertYourAudioDeviceSharesetNameHere");
											/// \sa <tt>\ref AK::SoundEngine::GetIDFromString()</tt>
											/// \sa \ref soundengine_plugins_audiodevices
											/// \sa \ref integrating_secondary_outputs
											/// \sa \ref default_audio_devices

	AkUInt32		idDevice;				///< Device specific identifier, when multiple devices of the same type are possible.  If only one device is possible, leave to 0.
											///< - PS4 Controller-Speakers: UserID as returned from sceUserServiceGetLoginUserIdList
											///< - XBoxOne Controller-Headphones: Use the AK::GetDeviceID function to get the ID from an IMMDevice. Find the player's device with the WASAPI API (IMMDeviceEnumerator, see Microsoft documentation) or use AK::GetDeviceIDFromName.											
											///< - Windows: Use AK::GetDeviceID or AK::GetDeviceIDFromName to get the correct ID.  Leave to 0 for the default Windows device as seen in Audio Properties.
											///< - All other outputs: use 0 to select the default for the selected audio device type (shareset)	

	AkPanningRule	ePanningRule;			///< Rule for 3D panning of signals routed to a stereo bus. In AkPanningRule_Speakers mode, the angle of the front loudspeakers 
											///< (uSpeakerAngles[0]) is used. In AkPanningRule_Headphones mode, the speaker angles are superseded with constant power panning
											///< between two virtual microphones spaced 180 degrees apart.

	AkChannelConfig	channelConfig;			///< Channel configuration for this output. Call AkChannelConfig::Clear() to let the engine use the default output configuration.  
											///< Hardware might not support the selected configuration.
};

/// Possible values for the floor plane axis used in the Game Object 3D Viewer in Wwise
enum AkFloorPlane
{
	AkFloorPlane_XZ = 0,
	AkFloorPlane_XY,
	AkFloorPlane_YZ,
	AkFloorPlane_Default = AkFloorPlane_XZ
};

/// The execution context for a task.
struct AkTaskContext
{
    AkUInt32 uIdxThread;           ///< The current thread index [0, AkTaskSchedulerDesc::uNumSchedulerWorkerThreads[.
};

/// Task function: process the [in_uIdxBegin,in_uIdxEnd[ range of items in the in_pData array.
typedef void(*AkParallelForFunc)(void* in_pData, AkUInt32 in_uIdxBegin, AkUInt32 in_uIdxEnd, AkTaskContext in_ctx, void* in_pUserData);

/// Description of the client task scheduler.
struct AkTaskSchedulerDesc
{
    /// Execute in_func in parallel over the [in_uIdxBegin,in_uIdxEnd[ range of items in the in_pData array,
	/// and wait for the work to finish.
	typedef void(*ParallelForFunc)(
		void * in_pData,            ///< A pointer to a data array.
		AkUInt32 in_uIdxBegin,     ///< The first element of data to process.
		AkUInt32 in_uIdxEnd,       ///< The one past the last element of data to process.
		AkUInt32 in_uTileSize,     ///< The desired maximum tile size in which to subdivide the data.
		AkParallelForFunc in_func, ///< The data processing function.
		void * in_pUserData,       ///< User data to be passed into 'func'.
		const char * in_szDebugName);    ///< Debug name for the workload.

    ParallelForFunc fcnParallelFor;             ///< ParallelForFunc callback.
    AkUInt32        uNumSchedulerWorkerThreads; ///< The number of worker threads in the schduler.
};

/// Platform-independent initialization settings of the sound engine
/// \sa 
/// - <tt>AK::SoundEngine::Init()</tt>
/// - <tt>AK::SoundEngine::GetDefaultInitSettings()</tt>
/// - \ref soundengine_integration_init_advanced
/// - \ref soundengine_initialization_advanced_soundengine_using_memory_threshold
struct AkInitSettings
{
    AkAssertHook        pfnAssertHook;				///< External assertion handling function (optional)

    AkUInt32            uMaxNumPaths;				///< Maximum number of paths for positioning
    AkUInt32            uDefaultPoolSize;			///< Size of the default memory pool, in bytes
	AkReal32            fDefaultPoolRatioThreshold;	///< 0.0f to 1.0f value: The percentage of occupied memory where the sound engine should enter in Low memory Mode. \ref soundengine_initialization_advanced_soundengine_using_memory_threshold
	AkUInt32            uCommandQueueSize;			///< Size of the command queue, in bytes
	AkMemPoolId			uPrepareEventMemoryPoolID;	///< Memory pool where data allocated by <tt>AK::SoundEngine::PrepareEvent()</tt> and <tt>AK::SoundEngine::PrepareGameSyncs()</tt> will be done. 
	bool				bEnableGameSyncPreparation;	///< Sets to true to enable AK::SoundEngine::PrepareGameSync usage.
	AkUInt32			uContinuousPlaybackLookAhead;	///< Number of quanta ahead when continuous containers should instantiate a new voice before which next sounds should start playing. This look-ahead time allows I/O to occur, and is especially useful to reduce the latency of continuous containers with trigger rate or sample-accurate transitions. 
													///< Default is 1 audio quantum, also known as an audio frame. Its size is equal to AkInitSettings::uNumSamplesPerFrame / AkPlatformInitSettings::uSampleRate. For many platforms the default values - which can be overridden - are respectively 1,024 samples and 48 kHz. This gives a default 21.3 ms for an audio quantum, which is adequate if you have a RAM-based streaming device that completes transfers within 20 ms. With 1 look-ahead quantum, voices spawned by continuous containers are more likely to be ready when they are required to play, thereby improving the overall precision of sound scheduling. If your device completes transfers in 30 ms instead, you might consider increasing this value to 2 because it will grant new voices 2 audio quanta (~43 ms) to fetch data. 

	AkUInt32			uNumSamplesPerFrame;		///< Number of samples per audio frame (256, 512, 1024, or 2048).

    AkUInt32            uMonitorPoolSize;			///< Size of the monitoring pool, in bytes. This parameter is not used in Release build.
    AkUInt32            uMonitorQueuePoolSize;		///< Size of the monitoring queue pool, in bytes. This parameter is not used in Release build.
	
	AkOutputSettings	settingsMainOutput;			///< Main output device settings.
	AkUInt32			uMaxHardwareTimeoutMs;		///< Amount of time to wait for HW devices to trigger an audio interrupt. If there is no interrupt after that time, the sound engine will revert to  silent mode and continue operating until the HW finally comes back. Default value: 2000 (2 seconds)

	bool				bUseSoundBankMgrThread;		///< Use a separate thread for loading sound banks. Allows asynchronous operations.
	bool				bUseLEngineThread;			///< Use a separate thread for processing audio. If set to false, audio processing will occur in RenderAudio(). \ref goingfurther_eventmgrthread

	AkBackgroundMusicChangeCallbackFunc BGMCallback; ///< Application-defined audio source change event callback function.
	void*				BGMCallbackCookie;			///< Application-defined user data for the audio source change event callback function.
	AkOSChar *			szPluginDLLPath;			///< When using DLLs for plugins, specify their path. Leave NULL if DLLs are in the same folder as the game executable.

	AkFloorPlane		eFloorPlane;				///< Floor plane axis for 3D game object viewing.
    AkTaskSchedulerDesc taskSchedulerDesc;			///< The defined client task scheduler that AkSoundEngine will use to schedule internal tasks.
};

/// Necessary settings for setting externally-loaded sources
struct AkSourceSettings
{
	AkUniqueID	sourceID;							///< Source ID (available in the SoundBank content files)
	AkUInt8*	pMediaMemory;						///< Pointer to the data to be set for the source
	AkUInt32	uMediaSize;							///< Size, in bytes, of the data to be set for the source
};

/// Return values for GetSourcePlayPositions.
struct AkSourcePosition
{
	AkUniqueID	audioNodeID;						///< Audio Node ID of playing item
	AkUniqueID	mediaID;							///< Media ID of playing item. (corresponds to 'ID' attribute of 'File' element in SoundBank metadata file)
	AkTimeMs	msTime;								///< Position of the source (in ms) associated with that playing item
};

/// Audiokinetic namespace
namespace AK
{
	class IReadBytes;
	class IWriteBytes;

	/// Audiokinetic sound engine namespace
	/// \remarks The functions in this namespace are thread-safe, unless stated otherwise.
	namespace SoundEngine
	{
        ///////////////////////////////////////////////////////////////////////
		/// @name Initialization
		//@{

		/// Query whether or not the sound engine has been successfully initialized.
		/// \warning This function is not thread-safe. It should not be called at the same time as \c SoundEngine::Init() or \c SoundEngine::Term().
		/// \return \c True if the sound engine has been initialized, \c False otherwise.
		/// \sa
		/// - \ref soundengine_integration_init_advanced
		/// - <tt>AK::SoundEngine::Init()</tt>
		/// - <tt>AK::SoundEngine::Term()</tt>
		AK_EXTERNAPIFUNC( bool, IsInitialized )();

		/// Initialize the sound engine.
		/// \warning This function is not thread-safe.
		/// \remark The initial settings should be initialized using <tt>AK::SoundEngine::GetDefaultInitSettings()</tt>
		///			and <tt>AK::SoundEngine::GetDefaultPlatformInitSettings()</tt> to fill the structures with their 
		///			default settings. This is not mandatory, but it helps avoid backward compatibility problems.
		///		
		/// \return 
		/// - AK_Success if the initialization was successful
		/// - AK_MemManagerNotInitialized if the memory manager is not available or not properly initialized
		/// - AK_StreamMgrNotInitialized if the stream manager is not available or not properly initialized
		/// - AK_SSEInstructionsNotSupported if the machine does not support SSE instruction (only on the PC)
		/// - AK_InsufficientMemory or AK_Fail if there is not enough memory available to initialize the sound engine properly
		/// - AK_InvalidParameter if some parameters are invalid
		/// - AK_Fail if the sound engine is already initialized, or if the provided settings result in insufficient 
		/// resources for the initialization.
		/// \sa
		/// - \ref soundengine_integration_init_advanced
		/// - <tt>AK::SoundEngine::Term()</tt>
		/// - <tt>AK::SoundEngine::GetDefaultInitSettings()</tt>
		/// - <tt>AK::SoundEngine::GetDefaultPlatformInitSettings()</tt>
        AK_EXTERNAPIFUNC( AKRESULT, Init )(
            AkInitSettings *			in_pSettings,   		///< Initialization settings (can be NULL, to use the default values)
            AkPlatformInitSettings *	in_pPlatformSettings  	///< Platform-specific settings (can be NULL, to use the default values)
		    );

		/// Gets the default values of the platform-independent initialization settings.
		/// \warning This function is not thread-safe.
		/// \sa
		/// - \ref soundengine_integration_init_advanced
		/// - <tt>AK::SoundEngine::Init()</tt>
		/// - <tt>AK::SoundEngine::GetDefaultPlatformInitSettings()</tt>
		AK_EXTERNAPIFUNC( void, GetDefaultInitSettings )(
            AkInitSettings &			out_settings   			///< Returned default platform-independent sound engine settings
		    );

		/// Gets the default values of the platform-specific initialization settings.
		///
		/// Windows Specific:
		///		When initializing for Windows platform, the HWND value returned in the 
		///		AkPlatformInitSettings structure is the foreground HWND at the moment of the 
		///		initialization of the sound engine and may not be the correct one for your need.
		///		Each game must specify the HWND that will be passed to DirectSound initialization.
		///		It is required that each game provides the correct HWND to be used or it could cause
		///		one of the following problem:
		///				- Random Sound engine initialization failure.
		///				- Audio focus to be located on the wrong window.
		///
		/// \warning This function is not thread-safe.
		/// \sa 
		/// - \ref soundengine_integration_init_advanced
		/// - <tt>AK::SoundEngine::Init()</tt>
		/// - <tt>AK::SoundEngine::GetDefaultInitSettings()</tt>
		AK_EXTERNAPIFUNC( void, GetDefaultPlatformInitSettings )(
            AkPlatformInitSettings &	out_platformSettings  	///< Returned default platform-specific sound engine settings
		    );
		
        /// Terminates the sound engine.
		/// If some sounds are still playing or events are still being processed when this function is 
		///	called, they will be stopped.
		/// \warning This function is not thread-safe.
		/// \warning Before calling Term, you must ensure that no other thread is accessing the sound engine.
		/// \sa 
		/// - \ref soundengine_integration_init_advanced
		/// - <tt>AK::SoundEngine::Init()</tt>
        AK_EXTERNAPIFUNC( void, Term )();

		/// Gets the configured audio settings.
		/// Call this function to get the configured audio settings.
		/// 
		/// \warning This function is not thread-safe.
		/// \warning Call this function only after the sound engine has been properly initialized.
		AK_EXTERNAPIFUNC( AKRESULT, GetAudioSettings )(
			AkAudioSettings &	out_audioSettings  	///< Returned audio settings
			);

		/// Gets the output speaker configuration of the specified output.
		/// Call this function to get the speaker configuration of the output (which may not correspond
		/// to the physical output format of the platform, in the case of downmixing provided by the platform itself). 
		/// You may initialize the sound engine with a user-specified configuration, but the resulting 
		/// configuration is determined by the sound engine, based on the platform, output type and
		/// platform settings (for e.g. system menu or control panel option). 
		/// 
		/// \warning Call this function only after the sound engine has been properly initialized. If you are initializing the sound engine with AkInitSettings::bUseLEngineThread to false, it is required to call RenderAudio() at least once before calling this function to complete the sound engine initialization.
		/// \return The output configuration. An empty AkChannelConfig (!AkChannelConfig::IsValid()) if device does not exist.
		/// \sa 
		/// - AkSpeakerConfig.h
		/// - AkOutputSettings
		AK_EXTERNAPIFUNC( AkChannelConfig, GetSpeakerConfiguration )(
			AkOutputDeviceID	in_idOutput = 0				///< Output ID to set the bus on.  As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output	
			);

		/// Gets the panning rule of the specified output.
		/// \warning Call this function only after the sound engine has been properly initialized.
		/// \return One of the supported configuration: 
		/// - AkPanningRule_Speakers
		/// - AkPanningRule_Headphone
		/// \sa 
		/// - AkSpeakerConfig.h
		AK_EXTERNAPIFUNC( AKRESULT, GetPanningRule )(
			AkPanningRule &		out_ePanningRule,			///< Returned panning rule (AkPanningRule_Speakers or AkPanningRule_Headphone) for given output.
			AkOutputDeviceID	in_idOutput = 0				///< Output ID to set the bus on.  As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output	
			);

		/// Sets the panning rule of the specified output.
		/// This may be changed anytime once the sound engine is initialized.
		/// \warning This function posts a message through the sound engine's internal message queue, whereas GetPanningRule() queries the current panning rule directly.
		AK_EXTERNAPIFUNC( AKRESULT, SetPanningRule )( 
			AkPanningRule		in_ePanningRule,			///< Panning rule.
			AkOutputDeviceID	in_idOutput = 0				///< Output ID to set the bus on.  As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output	
			);

		/// Gets speaker angles of the specified device. Speaker angles are used for 3D positioning of sounds over standard configurations.
		/// Note that the current version of Wwise only supports positioning on the plane.
		/// The speaker angles are expressed as an array of loudspeaker pairs, in degrees, relative to azimuth ]0,180].
		/// Supported loudspeaker setups are always symmetric; the center speaker is always in the middle and thus not specified by angles.
		/// Angles must be set in ascending order. 
		/// You may call this function with io_pfSpeakerAngles set to NULL to get the expected number of angle values in io_uNumAngles, 
		/// in order to allocate your array correctly. You may also obtain this number by calling
		/// AK::GetNumberOfAnglesForConfig( AK_SPEAKER_SETUP_DEFAULT_PLANE ).
		/// If io_pfSpeakerAngles is not NULL, the array is filled with up to io_uNumAngles.
		/// Typical usage:
		/// - AkUInt32 uNumAngles;
		/// - GetSpeakerAngles( NULL, uNumAngles, AkOutput_Main );
		/// - AkReal32 * pfSpeakerAngles = AkAlloca( uNumAngles * sizeof(AkReal32) );
		/// - GetSpeakerAngles( pfSpeakerAngles, uNumAngles, AkOutput_Main );
		/// \aknote 
		///	On most platforms, the angle set on the plane consists of 3 angles, to account for 7.1. 
		/// - When panning to stereo (speaker mode, see <tt>AK::SoundEngine::SetPanningRule()</tt>), only angle[0] is used, and 3D sounds in the back of the listener are mirrored to the front. 
		/// - When panning to 5.1, the front speakers use angle[0], and the surround speakers use (angle[2] - angle[1]) / 2.
		/// \endaknote
		/// \warning Call this function only after the sound engine has been properly initialized.
		/// \return AK_Success if device exists.
		/// \sa SetSpeakerAngles()
		AK_EXTERNAPIFUNC( AKRESULT, GetSpeakerAngles )(
			AkReal32 *			io_pfSpeakerAngles,			///< Returned array of loudspeaker pair angles, in degrees relative to azimuth [0,180]. Pass NULL to get the required size of the array.
			AkUInt32 &			io_uNumAngles,				///< Returned number of angles in io_pfSpeakerAngles, which is the minimum between the value that you pass in, and the number of angles corresponding to AK::GetNumberOfAnglesForConfig( AK_SPEAKER_SETUP_DEFAULT_PLANE ), or just the latter if io_pfSpeakerAngles is NULL.
			AkReal32 &			out_fHeightAngle,			///< Elevation of the height layer, in degrees relative to the plane [-90,90].
			AkOutputDeviceID	in_idOutput = 0				///< Output ID to set the bus on.  As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output			
			);
		
		/// Sets speaker angles of the specified device. Speaker angles are used for 3D positioning of sounds over standard configurations.
		/// Note that the current version of Wwise only supports positioning on the plane.
		/// The speaker angles are expressed as an array of loudspeaker pairs, in degrees, relative to azimuth ]0,180].
		/// Supported loudspeaker setups are always symmetric; the center speaker is always in the middle and thus not specified by angles.
		/// Angles must be set in ascending order. 
		/// Typical usage: 
		/// - Initialize the sound engine and/or add secondary output(s).
		/// - Get number of speaker angles and their value into an array using GetSpeakerAngles().
		/// - Modify the angles and call SetSpeakerAngles().
		/// This function posts a message to the audio thread through the command queue, so it is thread safe. However the result may not be immediately read with GetSpeakerAngles().
		/// \warning This function only applies to configurations (or subset of these configurations) that are standard and whose speakers are on the plane (2D).
		/// \return AK_NotCompatible if the channel configuration of the device is not standard (AK_ChannelConfigType_Standard), 
		/// AK_Success if successful (device exists and angles are valid), AK_Fail otherwise.
		/// \sa GetSpeakerAngles()
		AK_EXTERNAPIFUNC( AKRESULT, SetSpeakerAngles )(
			AkReal32 *			in_pfSpeakerAngles,			///< Array of loudspeaker pair angles, in degrees relative to azimuth [0,180].
			AkUInt32			in_uNumAngles,				///< Number of elements in in_pfSpeakerAngles. It must correspond to AK::GetNumberOfAnglesForConfig( AK_SPEAKER_SETUP_DEFAULT_PLANE ) (the value returned by GetSpeakerAngles()).
			AkReal32 			in_fHeightAngle,			///< Elevation of the height layer, in degrees relative to the plane  [-90,90].
			AkOutputDeviceID	in_idOutput = 0				///< Output ID to set the bus on.  As returned from AddOutput or GetOutputID.  You can pass 0 for the main (default) output			
			);

		/// Allows the game to set the volume threshold to be used by the sound engine to determine if a voice must go virtual.
		/// This may be changed anytime once the sound engine was initialized.
		/// If this function is not called, the used value will be the value specified in the platform specific project settings.
		/// \return 
		/// - AK_InvalidParameter if the threshold was not between 0 and -96.3 dB.
		/// - AK_Success if successful
		AK_EXTERNAPIFUNC( AKRESULT, SetVolumeThreshold )( 
			AkReal32 in_fVolumeThresholdDB ///< Volume Threshold, must be a value between 0 and -96.3 dB
			);

		/// Allows the game to set the maximum number of non virtual voices to be played simultaneously.
		/// This may be changed anytime once the sound engine was initialized.
		/// If this function is not called, the used value will be the value specified in the platform specific project settings.
		/// \return 
		/// - AK_InvalidParameter if the threshold was not between 1 and MaxUInt16.
		/// - AK_Success if successful
		AK_EXTERNAPIFUNC( AKRESULT, SetMaxNumVoicesLimit )( 
			AkUInt16 in_maxNumberVoices ///< Maximum number of non-virtual voices.
			);
				
        //@}

		////////////////////////////////////////////////////////////////////////
		/// @name Rendering Audio
		//@{

		/// Processes all commands in the sound engine's command queue.
		/// This method has to be called periodically (usually once per game frame).
		/// \sa 
		/// - \ref concept_events
		/// - \ref soundengine_events
		/// - <tt>AK::SoundEngine::PostEvent()</tt>
		/// \return Always returns AK_Success
        AK_EXTERNAPIFUNC( AKRESULT, RenderAudio )( 
			bool in_bAllowSyncRender = true				///< When AkInitSettings::bUseLEngineThread is false, RenderAudio may generate an audio buffer -- unless in_bAllowSyncRender is set to false. Use in_bAllowSyncRender=false when calling RenderAudio from a Sound Engine callback.
			);

		//@}

		////////////////////////////////////////////////////////////////////////
		/// @name Component Registration
		//@{

		/// Query interface to global plug-in context used for plug-in registration/initialization.
		/// \return Global plug-in context.
		AK_EXTERNAPIFUNC(AK::IAkGlobalPluginContext *, GetGlobalPluginContext)();

		/// Registers a plug-in with the sound engine and sets the callback functions to create the 
		/// plug-in and its parameter node.  
		/// \aknote 
		///	This function is deprecated. Registration is now automatic if you link plug-ins statically. If plug-ins are dynamic libraries (such as DLLs or SOs), use \c RegisterPluginDLL.
		/// \endaknote
		/// \sa
		/// - \ref register_effects
		/// - \ref plugin_xml
		/// \return AK_Success if successful, AK_InvalidParameter if invalid parameters were provided or Ak_Fail otherwise. Possible reasons for an AK_Fail result are:
		/// - Insufficient memory to register the plug-in
		/// - Plug-in ID already registered
		/// \remarks
		/// Codecs and plug-ins must be registered before loading banks that use them.\n
		/// Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
        AK_EXTERNAPIFUNC( AKRESULT, RegisterPlugin )( 
			AkPluginType in_eType,						///< Plug-in type (for example, source or effect)
			AkUInt32 in_ulCompanyID,					///< Company identifier (as declared in the plug-in description XML file)
			AkUInt32 in_ulPluginID,						///< Plug-in identifier (as declared in the plug-in description XML file)
			AkCreatePluginCallback in_pCreateFunc,		///< Pointer to the plug-in's creation function
            AkCreateParamCallback in_pCreateParamFunc	///< Pointer to the plug-in's parameter node creation function
            );

		/// Loads a plug-in dynamic library and registers it with the sound engine.  
		/// With dynamic linking, all plugins are automatically registered.
		/// The plug-in DLL must be in the OS-specific library path or in the same location as the executable. If not, set AkInitSettings.szPluginDLLPath.
		/// \return 
		/// - Ak_Success if successful.  
		/// - AK_FileNotFound if the DLL is not found in the OS path or if it has extraneous dependencies not found.  
		/// - AK_InvalidFile if the symbol g_pAKPluginList is not exported by the dynamic library
		AK_EXTERNAPIFUNC( AKRESULT, RegisterPluginDLL ) (
			const AkOSChar* in_DllName					///< Name of the DLL to load, without "lib" prefix or extension.  
			);
		
		/// Registers a codec type with the sound engine and set the callback functions to create the 
		/// codec's file source and bank source nodes.
		/// \aknote 
		///	This function is deprecated. Registration is now automatic if you link plugins statically. If plugins are dynamic libraries (such as DLLs or SOs), use RegisterPluginDLL.
		/// \endaknote		
		/// \sa 
		/// - \ref register_effects
		/// \return AK_Success if successful, AK_InvalidParameter if invalid parameters were provided, or Ak_Fail otherwise. Possible reasons for an AK_Fail result are:
		/// - Insufficient memory to register the codec
		/// - Codec ID already registered
		/// \remarks
		/// Codecs and plug-ins must be registered before loading banks that use them.\n
		/// Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the Event will fail.
        AK_EXTERNAPIFUNC( AKRESULT, RegisterCodec )( 
			AkUInt32 in_ulCompanyID,						///< Company identifier (as declared in the plug-in description XML file)
			AkUInt32 in_ulCodecID,							///< Codec identifier (as declared in the plug-in description XML file)
			AkCreateFileSourceCallback in_pFileCreateFunc,	///< Pointer to the codec's file source node creation function
            AkCreateBankSourceCallback in_pBankCreateFunc	///< Pointer to the codec's bank source node creation function
            );

		/// Registers a global callback function. This function will be called from the audio rendering thread, at the
		/// location specified by in_eLocation. This function will also be called from the thread calling 			
		/// AK::SoundEngine::Term with in_eLocation set to AkGlobalCallbackLocation_Term.
		/// For example, in order to be called at every audio rendering pass, and once during teardown for releasing resources, you would call 
		/// RegisterGlobalCallback(myCallback, AkGlobalCallbackLocation_BeginRender  | AkGlobalCallbackLocation_Term, myCookie, AkPluginTypeNone, 0, 0);  
		/// \remarks
		/// A Plugin Type, Company ID and Plugin ID can be provided to this function to enable timing in the performance montior.
		/// If the callback is being timed, it will contribute to the Total Plug-in CPU measurement, and also appear in the Plug-ins tab of the Advanced Profiler by plug-in type and ID. 
		/// It is illegal to call this function while already inside of a global callback.
		/// This function might stall for several milliseconds before returning.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnregisterGlobalCallback()</tt>
		/// - AkGlobalCallbackFunc
		/// - AkGlobalCallbackLocation
		AK_EXTERNAPIFUNC(AKRESULT, RegisterGlobalCallback)(
			AkGlobalCallbackFunc in_pCallback,				///< Function to register as a global callback.
			AkUInt32 in_eLocation = AkGlobalCallbackLocation_BeginRender, ///< Callback location defined in AkGlobalCallbackLocation. Bitwise OR multiple locations if needed.
			void * in_pCookie = NULL,						///< User cookie.
			AkPluginType in_eType = AkPluginTypeNone,		///< Plug-in type (for example, source or effect). AkPluginTypeNone for no timing.
			AkUInt32 in_ulCompanyID = 0,					///< Company identifier (as declared in the plug-in description XML file). 0 for no timing. 
			AkUInt32 in_ulPluginID = 0						///< Plug-in identifier (as declared in the plug-in description XML file). 0 for no timing. 
			);

		/// Unregisters a global callback function, previously registered using RegisterGlobalCallback.
		/// \remarks
		/// It is legal to call this function while already inside of a global callback, If it is unregistering itself and not
		/// another callback.
		/// This function might stall for several milliseconds before returning.
		/// \sa 
		/// - <tt>AK::SoundEngine::RegisterGlobalCallback()</tt>
		/// - AkGlobalCallbackFunc
		/// - AkGlobalCallbackLocation
		AK_EXTERNAPIFUNC(AKRESULT, UnregisterGlobalCallback)(
			AkGlobalCallbackFunc in_pCallback,				///< Function to unregister as a global callback.
			AkUInt32 in_eLocation = AkGlobalCallbackLocation_BeginRender ///< Must match in_eLocation as passed to RegisterGlobalCallback for this callback.
			);

		/// Registers a callback for the Audio Device status changes.  
		/// The callback will be called from the audio thread
		/// Can be called prior to AK::SoundEngine::Init
		AK_EXTERNAPIFUNC(AKRESULT, RegisterAudioDeviceStatusCallback)(
			AK::AkDeviceStatusCallbackFunc in_pCallback				///< Function to register as a status callback.						
			);

		/// Unregisters the callback for the Audio Device status changes, registered by RegisterAudioDeviceStatusCallback				
		AK_EXTERNAPIFUNC(AKRESULT, UnregisterAudioDeviceStatusCallback)();
		//@}

#ifdef AK_SUPPORT_WCHAR
		////////////////////////////////////////////////////////////////////////
		/// @name Getting ID from strings
		//@{

		/// Universal converter from Unicode string to ID for the sound engine.
		/// This function will hash the name based on a algorithm ( provided at : /AK/Tools/Common/AkFNVHash.h )
		/// Note:
		///		This function does return a AkUInt32, which is totally compatible with:
		///		AkUniqueID, AkStateGroupID, AkStateID, AkSwitchGroupID, AkSwitchStateID, AkRtpcID, and so on...
		/// \sa
		/// - <tt>AK::SoundEngine::PostEvent</tt>
		/// - <tt>AK::SoundEngine::SetRTPCValue</tt>
		/// - <tt>AK::SoundEngine::SetSwitch</tt>
		/// - <tt>AK::SoundEngine::SetState</tt>
		/// - <tt>AK::SoundEngine::PostTrigger</tt>
		/// - <tt>AK::SoundEngine::SetGameObjectAuxSendValues</tt>
		/// - <tt>AK::SoundEngine::LoadBank</tt>
		/// - <tt>AK::SoundEngine::UnloadBank</tt>
		/// - <tt>AK::SoundEngine::PrepareEvent</tt>
		/// - <tt>AK::SoundEngine::PrepareGameSyncs</tt>
		AK_EXTERNAPIFUNC( AkUInt32, GetIDFromString )( const wchar_t* in_pszString );
#endif //AK_SUPPORT_WCHAR

		/// Universal converter from string to ID for the sound engine.
		/// This function will hash the name based on a algorithm ( provided at : /AK/Tools/Common/AkFNVHash.h )
		/// Note:
		///		This function does return a AkUInt32, which is totally compatible with:
		///		AkUniqueID, AkStateGroupID, AkStateID, AkSwitchGroupID, AkSwitchStateID, AkRtpcID, and so on...
		/// \sa
		/// - <tt>AK::SoundEngine::PostEvent</tt>
		/// - <tt>AK::SoundEngine::SetRTPCValue</tt>
		/// - <tt>AK::SoundEngine::SetSwitch</tt>
		/// - <tt>AK::SoundEngine::SetState</tt>
		/// - <tt>AK::SoundEngine::PostTrigger</tt>
		/// - <tt>AK::SoundEngine::SetGameObjectAuxSendValues</tt>
		/// - <tt>AK::SoundEngine::LoadBank</tt>
		/// - <tt>AK::SoundEngine::UnloadBank</tt>
		/// - <tt>AK::SoundEngine::PrepareEvent</tt>
		/// - <tt>AK::SoundEngine::PrepareGameSyncs</tt>
		AK_EXTERNAPIFUNC( AkUInt32, GetIDFromString )( const char* in_pszString );

		//@}

		////////////////////////////////////////////////////////////////////////
		/// @name Event Management
		//@{

		/// Asynchronously posts an Event to the sound engine (by event ID).
		/// The callback function can be used to be noticed when markers are reached or when the event is finished.
		/// An array of wave file sources can be provided to resolve External Sources triggered by the event. 
        /// \return The playing ID of the event launched, or AK_INVALID_PLAYING_ID if posting the event failed
		/// \remarks
		/// If used, the array of external sources should contain the information for each external source triggered by the
		/// event. When triggering an event with multiple external sources, you need to differentiate each source 
		/// by using the cookie property in the External Source in the Wwise project and in AkExternalSourceInfo.
		/// \aknote If an event triggers the playback of more than one external source, they must be named uniquely in the project 
		/// (therefore have a unique cookie) in order to tell them apart when filling the AkExternalSourceInfo structures.
		/// \endaknote
		/// \sa 
		/// - \ref concept_events
		/// - \ref integrating_external_sources
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
        AK_EXTERNAPIFUNC( AkPlayingID, PostEvent )(
	        AkUniqueID in_eventID,							///< Unique ID of the event
	        AkGameObjectID in_gameObjectID,					///< Associated game object ID
			AkUInt32 in_uFlags = 0,							///< Bitmask: see \ref AkCallbackType
			AkCallbackFunc in_pfnCallback = NULL,			///< Callback function
			void * in_pCookie = NULL,						///< Callback cookie that will be sent to the callback function along with additional information
			AkUInt32 in_cExternals = 0,						///< Optional count of external source structures
			AkExternalSourceInfo *in_pExternalSources = NULL,///< Optional array of external source resolution information
			AkPlayingID	in_PlayingID = AK_INVALID_PLAYING_ID///< Optional (advanced users only) Specify the playing ID to target with the event. Will Cause active actions in this event to target an existing Playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any for normal playback.
	        );

#ifdef AK_SUPPORT_WCHAR
		/// Posts an Event to the sound engine (by Event name), using callbacks.
		/// The callback function can be used to be noticed when markers are reached or when the event is finished.
		/// An array of wave file sources can be provided to resolve External Sources triggered by the event. 
        /// \return The playing ID of the event launched, or AK_INVALID_PLAYING_ID if posting the event failed
		/// \remarks
		/// If used, the array of external sources should contain the information for each external source triggered by the
		/// event. When triggering an event with multiple external sources, you need to differentiate each source 
		/// by using the cookie property in the External Source in the Wwise project and in AkExternalSourceInfo.
		/// \aknote If an event triggers the playback of more than one external source, they must be named uniquely in the project 
		/// (therefore have a unique cookie) in order to tell them appart when filling the AkExternalSourceInfo structures.
		/// \endaknote
		/// \sa 
		/// - \ref concept_events
		/// - \ref integrating_external_sources
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
        AK_EXTERNAPIFUNC( AkPlayingID, PostEvent )(
	        const wchar_t* in_pszEventName,					///< Name of the event
	        AkGameObjectID in_gameObjectID,					///< Associated game object ID
			AkUInt32 in_uFlags = 0,							///< Bitmask: see \ref AkCallbackType
			AkCallbackFunc in_pfnCallback = NULL,			///< Callback function
			void * in_pCookie = NULL,						///< Callback cookie that will be sent to the callback function along with additional information.
			AkUInt32 in_cExternals = 0,						///< Optional count of external source structures
			AkExternalSourceInfo *in_pExternalSources = NULL,///< Optional array of external source resolution information
			AkPlayingID	in_PlayingID = AK_INVALID_PLAYING_ID///< Optional (advanced users only) Specify the playing ID to target with the event. Will Cause active actions in this event to target an existing Playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any for normal playback.
	        );
#endif //AK_SUPPORT_WCHAR

		/// Posts an Event to the sound engine (by Event name), using callbacks.
		/// The callback function can be used to be noticed when markers are reached or when the event is finished.
		/// An array of Wave file sources can be provided to resolve External Sources triggered by the event. P
        /// \return The playing ID of the event launched, or AK_INVALID_PLAYING_ID if posting the event failed
		/// \remarks
		/// If used, the array of external sources should contain the information for each external source triggered by the
		/// event. When triggering an Event with multiple external sources, you need to differentiate each source 
		/// by using the cookie property in the External Source in the Wwise project and in AkExternalSourceInfo.
		/// \aknote If an event triggers the playback of more than one external source, they must be named uniquely in the project 
		/// (therefore have a unique cookie) in order to tell them apart when filling the AkExternalSourceInfo structures.
		/// \endaknote
		/// \sa 
		/// - \ref concept_events
		/// - \ref integrating_external_sources
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
        AK_EXTERNAPIFUNC( AkPlayingID, PostEvent )(
	        const char* in_pszEventName,					///< Name of the event
	        AkGameObjectID in_gameObjectID,					///< Associated game object ID
			AkUInt32 in_uFlags = 0,							///< Bitmask: see \ref AkCallbackType
			AkCallbackFunc in_pfnCallback = NULL,			///< Callback function
			void * in_pCookie = NULL,						///< Callback cookie that will be sent to the callback function along with additional information.
			AkUInt32 in_cExternals = 0,						///< Optional count of external source structures
			AkExternalSourceInfo *in_pExternalSources = NULL,///< Optional array of external source resolution information
			AkPlayingID	in_PlayingID = AK_INVALID_PLAYING_ID///< Optional (advanced users only) Specify the playing ID to target with the event. Will Cause active actions in this event to target an existing Playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any for normal playback.
	        );

		// If you modify AkActionOnEventType, don't forget to modify the WAAPI validation schema accordingly.

		/// AkActionOnEventType
		/// \sa
		/// - <tt>AK::SoundEngine::ExecuteActionOnEvent()</tt>
		enum AkActionOnEventType
		{
			AkActionOnEventType_Stop	= 0,			///< Stop
			AkActionOnEventType_Pause	= 1,			///< Pause
			AkActionOnEventType_Resume	= 2,			///< Resume
			AkActionOnEventType_Break	= 3,			///< Break
			AkActionOnEventType_ReleaseEnvelope	= 4		///< Release envelope
		};

		/// Executes an action on all nodes that are referenced in the specified event in an action of type play.
		/// \sa
		/// - <tt>AK::SoundEngine::AkActionOnEventType</tt>
		AK_EXTERNAPIFUNC( AKRESULT, ExecuteActionOnEvent )(
			AkUniqueID in_eventID,												///< Unique ID of the event
			AkActionOnEventType in_ActionType,									///< Action to execute on all the elements that were played using the specified event.
	        AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,			///< Associated game object ID
			AkTimeMs in_uTransitionDuration = 0,								///< Fade duration
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the transition
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID					///< Associated PlayingID
			);

#ifdef AK_SUPPORT_WCHAR
		/// Executes an action on all nodes that are referenced in the specified event in an action of type play.
		/// \sa
		/// - <tt>AK::SoundEngine::AkActionOnEventType</tt>
		AK_EXTERNAPIFUNC( AKRESULT, ExecuteActionOnEvent )(
			const wchar_t* in_pszEventName,										///< Name of the event
			AkActionOnEventType in_ActionType,									///< Action to execute on all the elements that were played using the specified event.
	        AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,			///< Associated game object ID
			AkTimeMs in_uTransitionDuration = 0,								///< Fade duration
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the transition
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID					///< Associated PlayingID
			);
#endif //AK_SUPPORT_WCHAR

		/// Executes an Action on all nodes that are referenced in the specified Event in an Action of type Play.
		/// \sa
		/// - <tt>AK::SoundEngine::AkActionOnEventType</tt>
		AK_EXTERNAPIFUNC( AKRESULT, ExecuteActionOnEvent )(
			const char* in_pszEventName,										///< Name of the event
			AkActionOnEventType in_ActionType,									///< Action to execute on all the elements that were played using the specified event.
	        AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,			///< Associated game object ID
			AkTimeMs in_uTransitionDuration = 0,								///< Fade duration
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the transition
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID					///< Associated PlayingID
			);


		/// Executes a number of MIDI Events on all nodes that are referenced in the specified Event in an Action of type Play.
		/// Each MIDI event will be posted in AkMIDIPost::uOffset samples from the start of the current frame. The duration of
		/// a sample can be determined from the sound engine's audio settings, via a call to AK::SoundEngine::GetAudioSettings.
		/// \sa
		/// - <tt>AK::SoundEngine::GetAudioSettings</tt>
		/// - <tt>AK::SoundEngine::StopMIDIOnEvent</tt>
		AK_EXTERNAPIFUNC( AKRESULT, PostMIDIOnEvent )(
			AkUniqueID in_eventID,											///< Unique ID of the Event
	        AkGameObjectID in_gameObjectID,									///< Associated game object ID
			AkMIDIPost* in_pPosts,											///< MIDI Events to post
			AkUInt16 in_uNumPosts											///< Number of MIDI Events to post
			);

		/// Stops MIDI notes on all nodes that are referenced in the specified event in an action of type play,
		/// with the specified Game Object. Invalid parameters are interpreted as wildcards. For example, calling
		/// this function with in_eventID set to AK_INVALID_UNIQUE_ID will stop all MIDI notes for Game Object
		/// in_gameObjectID.
		/// \sa
		/// - <tt>AK::SoundEngine::PostMIDIOnEvent</tt>
		AK_EXTERNAPIFUNC( AKRESULT, StopMIDIOnEvent )(
			AkUniqueID in_eventID = AK_INVALID_UNIQUE_ID,					///< Unique ID of the Event
	        AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT			///< Associated game object ID
			);


		/// Starts streaming the first part of all streamed files referenced by an Event into a cache buffer. Caching streams are serviced when no other streams require the 
		/// available bandwidth. The files will remain cached until UnpinEventInStreamCache is called, or a higher priority pinned file needs the space and the limit set by 
		/// uMaxCachePinnedBytes is exceeded.  
		/// \remarks The amount of data from the start of the file that will be pinned to cache is determined by the prefetch size. The prefetch size is set via the authoring tool and stored in the sound banks.  
		/// \remarks It is possible to override the prefetch size stored in the sound bank via the low level IO. For more information see <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> and AkFileSystemFlags.
		/// \remarks If this function is called additional times with the same event, then the priority of the caching streams are updated. Note however that priority is passed down to the stream manager 
		///	on a file-by-file basis, and if another event is pinned to cache that references the same file but with a different priority, then the first priority will be updated with the most recent value.
		/// \remarks If the event references files that are chosen based on a State Group (via a switch container), all files in all states will be cached. Those in the current active state
		/// will get cached with active priority, while all other files will get cached with inactive priority.
		/// \remarks in_uInactivePriority is only relevant for events that reference switch containers that are assigned to State Groups. This parameter is ignored for all other events, including events that only reference
		/// switch containers that are assigned to Switch Groups. Files that are chosen based on a Switch Group have a different switch value per game object, and are all effectively considered active by the pin-to-cache system.
		/// \sa
		/// - <tt>AK::SoundEngine::GetBufferStatusForPinnedEvent</tt>
		/// - <tt>AK::SoundEngine::UnpinEventInStreamCache</tt>
		/// - <tt>AK::StreamMgr::IAkFileLocationResolver::Open</tt>
		/// - AkFileSystemFlags
		AK_EXTERNAPIFUNC( AKRESULT, PinEventInStreamCache )(
			AkUniqueID in_eventID,											///< Unique ID of the event
			AkPriority in_uActivePriority,									///< Priority of active stream caching I/O
			AkPriority in_uInactivePriority 								///< Priority of inactive stream caching I/O
			);

#ifdef AK_SUPPORT_WCHAR
		/// Starts streaming the first part of all streamed files referenced by an event into a cache buffer. Caching streams are serviced when no other streams require the 
		/// available bandwidth. The files will remain cached until UnpinEventInStreamCache is called, or a higher priority pinned file needs the space and the limit set by 
		/// uMaxCachePinnedBytes is exceeded.  
		/// \remarks The amount of data from the start of the file that will be pinned to cache is determined by the prefetch size. The prefetch size is set via the authoring tool and stored in the sound banks.  
		/// \remarks It is possible to override the prefetch size stored in the sound bank via the low level IO. For more information see <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> and AkFileSystemFlags.
		/// \remarks If this function is called additional times with the same event, then the priority of the caching streams are updated. Note however that priority is passed down to the stream manager 
		///	on a file-by-file basis, and if another event is pinned to cache that references the same file but with a different priority, then the first priority will be updated with the most recent value.
		/// \remarks If the event references files that are chosen based on a State Group (via a Switch Container), all files in all states will be cached. Those in the current active state
		/// will get cached with active priority, while all other files will get cached with inactive priority.
		/// \remarks in_uInactivePriority is only relevant for events that reference switch containers that are assigned to State Groups. This parameter is ignored for all other events, including events that only reference
		/// switch containers that are assigned to Switch Groups. Files that are chosen based on a Switch Group have a different switch value per game object, and are all effectively considered active by the pin-to-cache system.
		/// \sa
		/// - <tt>AK::SoundEngine::GetBufferStatusForPinnedEvent</tt>
		/// - <tt>AK::SoundEngine::UnpinEventInStreamCache</tt>
		/// - <tt>AK::StreamMgr::IAkFileLocationResolver::Open</tt>
		/// - AkFileSystemFlags
		AK_EXTERNAPIFUNC( AKRESULT, PinEventInStreamCache )(
			const wchar_t* in_pszEventName,									///< Name of the event
			AkPriority in_uActivePriority,									///< Priority of active stream caching I/O
			AkPriority in_uInactivePriority 								///< Priority of inactive stream caching I/O
			);
#endif //AK_SUPPORT_WCHAR

		/// Starts streaming the first part of all streamed files referenced by an event into a cache buffer. Caching streams are serviced when no other streams require the 
		/// available bandwidth. The files will remain cached until UnpinEventInStreamCache is called, or a higher priority pinned file needs the space and the limit set by 
		/// uMaxCachePinnedBytes is exceeded.  
		/// \remarks The amount of data from the start of the file that will be pinned to cache is determined by the prefetch size. The prefetch size is set via the authoring tool and stored in the sound banks.  
		/// \remarks It is possible to override the prefetch size stored in the sound bank via the low level IO. For more information see <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> and AkFileSystemFlags.
		/// \remarks If this function is called additional times with the same event, then the priority of the caching streams are updated. Note however that priority is passed down to the stream manager 
		/// on a file-by-file basis, and if another event is pinned to cache that references the same file but with a different priority, then the first priority will be updated with the most recent value.
		/// \remarks If the event references files that are chosen based on a State Group (via a switch container), all files in all states will be cached. Those in the current active state
		/// will get cached with active priority, while all other files will get cached with inactive priority.
		/// \remarks in_uInactivePriority is only relevant for events that reference switch containers that are assigned to State Groups. This parameter is ignored for all other events, including events that only reference
		/// switch containers that are assigned to Switch Groups. Files that are chosen based on a Switch Group have a different switch value per game object, and are all effectively considered active by the pin-to-cache system.
		/// \sa
		/// - <tt>AK::SoundEngine::GetBufferStatusForPinnedEvent</tt>
		/// - <tt>AK::SoundEngine::UnpinEventInStreamCache</tt>
		/// - <tt>AK::StreamMgr::IAkFileLocationResolver::Open</tt>
		/// - AkFileSystemFlags
		AK_EXTERNAPIFUNC( AKRESULT, PinEventInStreamCache )(
			const char* in_pszEventName,									///< Name of the event
			AkPriority in_uActivePriority,									///< Priority of active stream caching I/O
			AkPriority in_uInactivePriority 								///< Priority of inactive stream caching I/O
			);

		/// Releases the set of files that were previously requested to be pinned into cache via <tt>AK::SoundEngine::PinEventInStreamCache()</tt>. The file may still remain in stream cache
		/// after <tt>AK::SoundEngine::UnpinEventInStreamCache()</tt> is called, until the memory is reused by the streaming memory manager in accordance with to its cache management algorithm.
		/// \sa
		/// - <tt>AK::SoundEngine::PinEventInStreamCache</tt>
		/// - <tt>AK::SoundEngine::GetBufferStatusForPinnedEvent</tt>
		AK_EXTERNAPIFUNC( AKRESULT, UnpinEventInStreamCache )(
			AkUniqueID in_eventID											///< Unique ID of the event
			);	

#ifdef AK_SUPPORT_WCHAR
		/// Releases the set of files that were previously requested to be pinned into cache via <tt>AK::SoundEngine::PinEventInStreamCache()</tt>. The file may still remain in stream cache
		/// after <tt>AK::SoundEngine::UnpinEventInStreamCache()</tt> is called, until the memory is reused by the streaming memory manager in accordance with to its cache management algorithm.
		/// \sa
		/// - <tt>AK::SoundEngine::PinEventInStreamCache</tt>
		/// - <tt>AK::SoundEngine::GetBufferStatusForPinnedEvent</tt>
		AK_EXTERNAPIFUNC( AKRESULT, UnpinEventInStreamCache )(
			const wchar_t* in_pszEventName									///< Name of the event
			);
#endif //AK_SUPPORT_WCHAR

		/// Releases the set of files that were previously requested to be pinned into cache via <tt>AK::SoundEngine::PinEventInStreamCache()</tt>. The file may still remain in stream cache
		/// after <tt>AK::SoundEngine::UnpinEventInStreamCache()</tt> is called, until the memory is reused by the streaming memory manager in accordance with to its cache management algorithm.
		/// \sa
		/// - <tt>AK::SoundEngine::PinEventInStreamCache</tt>
		/// - <tt>AK::SoundEngine::GetBufferStatusForPinnedEvent</tt>
		AK_EXTERNAPIFUNC( AKRESULT, UnpinEventInStreamCache )(
			const char* in_pszEventName										///< Name of the event
			);

		/// Returns information about an Event that was requested to be pinned into cache via <tt>AK::SoundEngine::PinEventInStreamCache()</tt>.
		/// Retrieves the smallest buffer fill-percentage for each file referenced by the event, and whether 
		/// the cache-pinned memory limit is preventing any of the files from filling up their buffer.
		/// \remarks To set the limit for the maximum number of bytes that can be pinned to cache, see \c AkDeviceSettings 
		/// \sa
		/// - <tt>AK::SoundEngine::PinEventInStreamCache</tt>
		/// - <tt>AK::SoundEngine::UnpinEventInStreamCache</tt>
		/// - AkDeviceSettings
		AK_EXTERNAPIFUNC( AKRESULT, GetBufferStatusForPinnedEvent )(
			AkUniqueID in_eventID,											///< Unique ID of the event
			AkReal32& out_fPercentBuffered,									///< Fill-percentage (out of 100) of requested buffer size for least buffered file in the event.
			bool& out_bCachePinnedMemoryFull								///< True if at least one file can not complete buffering because the cache-pinned memory limit would be exceeded.
			);

		/// Returns information about an Event that was requested to be pinned into cache via \c <tt>AK::SoundEngine::PinEventInStreamCache()</tt>.
		/// Retrieves the smallest buffer fill-percentage for each file referenced by the event, and whether 
		/// the cache-pinned memory limit is preventing any of the files from filling up their buffer.
		/// \remarks To set the limit for the maximum number of bytes that can be pinned to cache, see AkDeviceSettings 
		/// \sa
		/// - <tt>AK::SoundEngine::PinEventInStreamCache</tt>
		/// - <tt>AK::SoundEngine::UnpinEventInStreamCache</tt>
		/// - AkDeviceSettings
		AK_EXTERNAPIFUNC( AKRESULT, GetBufferStatusForPinnedEvent )(
			const char* in_pszEventName,									///< Name of the event
			AkReal32& out_fPercentBuffered,									///< Fill-percentage (out of 100) of requested buffer size for least buffered file in the event.
			bool& out_bCachePinnedMemoryFull								///< True if at least one file can not complete buffering because the cache-pinned memory limit would be exceeded.
			);

#ifdef AK_SUPPORT_WCHAR
		/// Returns information about an Event that was requested to be pinned into cache via \c <tt>AK::SoundEngine::PinEventInStreamCache()</tt>.
		/// Retrieves the smallest buffer fill-percentage for each file referenced by the event, and whether 
		/// the cache-pinned memory limit is preventing any of the files from filling up their buffer.
		/// \remarks To set the limit for the maximum number of bytes that can be pinned to cache, see AkDeviceSettings 
		/// \sa
		/// - <tt>AK::SoundEngine::PinEventInStreamCache</tt>
		/// - <tt>AK::SoundEngine::UnpinEventInStreamCache</tt>
		/// - AkDeviceSettings
		AK_EXTERNAPIFUNC( AKRESULT, GetBufferStatusForPinnedEvent )(
			const wchar_t* in_pszEventName,									///< Name of the event
			AkReal32& out_fPercentBuffered,									///< Fill-percentage (out of 100) of requested buffer size for least buffered file in the event.
			bool& out_bCachePinnedMemoryFull								///< True if at least one file can not complete buffering because the cache-pinned memory limit would be exceeded.
			);
#endif

		/// Seeks inside all playing objects that are referenced in Play Actions of the specified Event.
		///
		/// Notes:
		///		- This works with all objects of the actor-mixer hierarchy, and also with Music Segments and Music Switch Containers. 
		///		- There is a restriction with sounds that play within a continuous sequence. Seeking is ignored 
		///			if one of their ancestors is a continuous (random or sequence) container with crossfade or 
		///			trigger rate transitions. Seeking is also ignored with sample-accurate transitions, unless
		///			the sound that is currently playing is the first sound of the sequence.
		///		- Seeking is also ignored with voices that can go virtual with "From Beginning" behavior. 
		///		- Sounds/segments are stopped if in_iPosition is greater than their duration.
		///		- in_iPosition is clamped internally to the beginning of the sound/segment.
		///		- If the option "Seek to nearest marker" is used, the seeking position snaps to the nearest marker.
		///			With objects of the actor-mixer hierarchy, markers are embedded in wave files by an external wave editor.
		///			Note that looping regions ("sampler loop") are not considered as markers. Also, the "add file name marker" of the 
		///			conversion settings dialog adds a marker at the beginning of the file, which is considered when seeking
		///			to nearest marker. In the case of objects of the interactive music hierarchy, user (wave) markers are ignored:
		///			seeking occurs to the nearest segment cue (authored in the segment editor), including the Entry Cue, but excluding the Exit Cue.
		///		- This method posts a command in the sound engine queue, thus seeking will not occur before 
		///			the audio thread consumes it (after a call to RenderAudio()). 
		///
		///	Notes specific to Music Segments:
		///		- With Music Segments, in_iPosition is relative to the Entry Cue, in milliseconds. Use a negative
		///			value to seek within the Pre-Entry.
		///		- Music segments cannot be looped. You may want to listen to the AK_EndOfEvent notification 
		///			in order to restart them if required.
		///		- In order to restart at the correct location, with all their tracks synchronized, Music Segments 
		///			take the "look-ahead time" property of their streamed tracks into account for seeking. 
		///			Consequently, the resulting position after a call to SeekOnEvent() might be earlier than the 
		///			value that was passed to the method. Use <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> to query 
		///			the exact position of a segment. Also, the segment will be silent during that time
		///			(so that it restarts precisely at the position that you specified). <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> 
		///			also informs you about the remaining look-ahead time.  
		///
		/// Notes specific to Music Switch Containers:
		///		- Seeking triggers a music transition towards the current (or target) segment.
		///			This transition is subject to the container's transition rule that matches the current and defined in the container,
		///			so the moment when seeking occurs depends on the rule's "Exit At" property. On the other hand, the starting position 
		///			in the target segment depends on both the desired seeking position and the rule's "Sync To" property.
		///		- If the specified time is greater than the destination segment's length, the modulo is taken.
		///
		/// \sa
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::PostEvent()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
		/// - <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SeekOnEvent )( 
			AkUniqueID in_eventID,										///< Unique ID of the event
			AkGameObjectID in_gameObjectID,								///< Associated game object ID; use AK_INVALID_GAME_OBJECT to affect all game objects
			AkTimeMs in_iPosition,										///< Desired position where playback should restart, in milliseconds
			bool in_bSeekToNearestMarker = false,						///< If true, the final seeking position will be made equal to the nearest marker (see note above)
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID			///< Specify the playing ID for the seek to be applied to. Will result in the seek happening only on active actions of the playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any, to seek on all active actions of this event ID.
			);

#ifdef AK_SUPPORT_WCHAR
		/// Seeks inside all playing objects that are referenced in Play Actions of the specified Event.
		///
		/// Notes:
		///		- This works with all objects of the actor-mixer hierarchy, and also with Music Segments and Music Switch Containers. 
		///		- There is a restriction with sounds that play within a continuous sequence. Seeking is ignored 
		///			if one of their ancestors is a continuous (random or sequence) container with crossfade or 
		///			trigger rate transitions. Seeking is also ignored with sample-accurate transitions, unless
		///			the sound that is currently playing is the first sound of the sequence.
		///		- Seeking is also ignored with voices that can go virtual with "From Beginning" behavior. 
		///		- With Music Segments, in_iPosition is relative to the Entry Cue, in milliseconds. Use a negative
		///			value to seek within the Pre-Entry.
		///		- Sounds/segments are stopped if in_iPosition is greater than their duration.
		///		- in_iPosition is clamped internally to the beginning of the sound/segment.
		///		- If the option "Seek to nearest marker" is used, the seeking position snaps to the nearest marker.
		///			With objects of the actor-mixer hierarchy, markers are embedded in wave files by an external wave editor.
		///			Note that looping regions ("sampler loop") are not considered as markers. Also, the "add file name marker" of the 
		///			conversion settings dialog adds a marker at the beginning of the file, which is considered when seeking
		///			to nearest marker. In the case of objects of the interactive music hierarchy, user (wave) markers are ignored:
		///			seeking occurs to the nearest segment cue (authored in the segment editor), including the Entry Cue, but excluding the Exit Cue.
		///		- This method posts a command in the sound engine queue, thus seeking will not occur before 
		///			the audio thread consumes it (after a call to RenderAudio()). 
		///
		///	Notes specific to Music Segments:
		///		- With Music Segments, in_iPosition is relative to the Entry Cue, in milliseconds. Use a negative
		///			value to seek within the Pre-Entry.
		///		- Music segments cannot be looped. You may want to listen to the AK_EndOfEvent notification 
		///			in order to restart them if required.
		///		- In order to restart at the correct location, with all their tracks synchronized, Music Segments 
		///			take the "look-ahead time" property of their streamed tracks into account for seeking. 
		///			Consequently, the resulting position after a call to SeekOnEvent() might be earlier than the 
		///			value that was passed to the method. Use <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> to query 
		///			the exact position of a segment. Also, the segment will be silent during that time
		///			(so that it restarts precisely at the position that you specified). <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> 
		///			also informs you about the remaining look-ahead time. 
		///
		/// Notes specific to Music Switch Containers:
		///		- Seeking triggers a music transition towards the current (or target) segment.
		///			This transition is subject to the container's transition rule that matches the current and defined in the container,
		///			so the moment when seeking occurs depends on the rule's "Exit At" property. On the other hand, the starting position 
		///			in the target segment depends on both the desired seeking position and the rule's "Sync To" property.
		///		- If the specified time is greater than the destination segment's length, the modulo is taken.
		///
		/// \sa
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::PostEvent()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
		/// - <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SeekOnEvent )( 
			const wchar_t* in_pszEventName,								///< Name of the event
			AkGameObjectID in_gameObjectID,								///< Associated game object ID; use AK_INVALID_GAME_OBJECT to affect all game objects
			AkTimeMs in_iPosition,										///< Desired position where playback should restart, in milliseconds
			bool in_bSeekToNearestMarker = false,						///< If true, the final seeking position will be made equal to the nearest marker (see note above)
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID			///< Specify the playing ID for the seek to be applied to. Will result in the seek happening only on active actions of the playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any, to seek on all active actions of this event ID.	
			);
#endif //AK_SUPPORT_WCHAR

		/// Seeks inside all playing objects that are referenced in Play Actions of the specified Event.
		///
		/// Notes:
		///		- This works with all objects of the actor-mixer hierarchy, and also with Music Segments and Music Switch Containers. 
		///		- There is a restriction with sounds that play within a continuous sequence. Seeking is ignored 
		///			if one of their ancestors is a continuous (random or sequence) container with crossfade or 
		///			trigger rate transitions. Seeking is also ignored with sample-accurate transitions, unless
		///			the sound that is currently playing is the first sound of the sequence.
		///		- Seeking is also ignored with voices that can go virtual with "From Beginning" behavior. 
		///		- With Music Segments, in_iPosition is relative to the Entry Cue, in milliseconds. Use a negative
		///			value to seek within the Pre-Entry.
		///		- Sounds/segments are stopped if in_iPosition is greater than their duration.
		///		- in_iPosition is clamped internally to the beginning of the sound/segment.
		///		- If the option "Seek to nearest marker" is used, the seeking position snaps to the nearest marker.
		///			With objects of the actor-mixer hierarchy, markers are embedded in wave files by an external wave editor.
		///			Note that looping regions ("sampler loop") are not considered as markers. Also, the "add file name marker" of the 
		///			conversion settings dialog adds a marker at the beginning of the file, which is considered when seeking
		///			to nearest marker. In the case of objects of the interactive music hierarchy, user (wave) markers are ignored:
		///			seeking occurs to the nearest segment cue (authored in the segment editor), including the Entry Cue, but excluding the Exit Cue.
		///		- This method posts a command in the sound engine queue, thus seeking will not occur before 
		///			the audio thread consumes it (after a call to RenderAudio()). 
		///
		///	Notes specific to Music Segments:
		///		- With Music Segments, in_iPosition is relative to the Entry Cue, in milliseconds. Use a negative
		///			value to seek within the Pre-Entry.
		///		- Music segments cannot be looped. You may want to listen to the AK_EndOfEvent notification 
		///			in order to restart them if required.
		///		- In order to restart at the correct location, with all their tracks synchronized, Music Segments 
		///			take the "look-ahead time" property of their streamed tracks into account for seeking. 
		///			Consequently, the resulting position after a call to SeekOnEvent() might be earlier than the 
		///			value that was passed to the method. Use <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> to query 
		///			the exact position of a segment. Also, the segment will be silent during that time
		///			(so that it restarts precisely at the position that you specified). <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> 
		///			also informs you about the remaining look-ahead time. 
		///
		/// Notes specific to Music Switch Containers:
		///		- Seeking triggers a music transition towards the current (or target) segment.
		///			This transition is subject to the container's transition rule that matches the current and defined in the container,
		///			so the moment when seeking occurs depends on the rule's "Exit At" property. On the other hand, the starting position 
		///			in the target segment depends on both the desired seeking position and the rule's "Sync To" property.
		///		- If the specified time is greater than the destination segment's length, the modulo is taken.
		///
		/// \sa
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::PostEvent()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
		/// - <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SeekOnEvent )( 
			const char* in_pszEventName,								///< Name of the event
			AkGameObjectID in_gameObjectID,								///< Associated game object ID; use AK_INVALID_GAME_OBJECT to affect all game objects
			AkTimeMs in_iPosition,										///< Desired position where playback should restart, in milliseconds
			bool in_bSeekToNearestMarker = false,						///< If true, the final seeking position will be made equal to the nearest marker (see note above)	
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID			///< Specify the playing ID for the seek to be applied to. Will result in the seek happening only on active actions of the playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any, to seek on all active actions of this event ID.	
			);

		/// Seeks inside all playing objects that are referenced in Play Actions of the specified Event.
		/// Seek position is specified as a percentage of the sound's total duration, and takes looping into account. 
		///
		/// Notes:
		///		- This works with all objects of the actor-mixer hierarchy, and also with Music Segments and Music Switch Containers. 
		///		- There is a restriction with sounds that play within a continuous sequence. Seeking is ignored 
		///			if one of their ancestors is a continuous (random or sequence) container with crossfade or 
		///			trigger rate transitions. Seeking is also ignored with sample-accurate transitions, unless
		///			the sound that is currently playing is the first sound of the sequence.
		///		- Seeking is also ignored with voices that can go virtual with "From Beginning" behavior. 
		///		- in_iPosition is clamped internally to the beginning of the sound/segment.
		///		- If the option "Seek to nearest marker" is used, the seeking position snaps to the nearest marker.
		///			With objects of the actor-mixer hierarchy, markers are embedded in wave files by an external wave editor.
		///			Note that looping regions ("sampler loop") are not considered as markers. Also, the "add file name marker" of the 
		///			conversion settings dialog adds a marker at the beginning of the file, which is considered when seeking
		///			to nearest marker. In the case of objects of the interactive music hierarchy, user (wave) markers are ignored:
		///			seeking occurs to the nearest segment cue (authored in the segment editor), including the Entry Cue, but excluding the Exit Cue.
		///		- This method posts a command in the sound engine queue, thus seeking will not occur before 
		///			the audio thread consumes it (after a call to RenderAudio()). 
		///
		///	Notes specific to Music Segments:
		///		- With Music Segments, \c in_fPercent is relative to the Entry Cue, and the segment's duration is the 
		///			duration between its entry and exit cues. Consequently, you cannot seek within the pre-entry or
		///			post-exit of a segment using this method. Use absolute values instead.
		///		- Music Segments cannot be looped. You may want to listen to the \c AK_EndOfEvent notification 
		///			in order to restart them if required.
		///		- In order to restart at the correct location, with all their tracks synchronized, Music Segments 
		///			take the "look-ahead time" property of their streamed tracks into account for seeking. 
		///			Consequently, the resulting position after a call to SeekOnEvent() might be earlier than the 
		///			value that was passed to the method. Use <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> to query 
		///			the exact position of a segment. Also, the segment will be silent during the time that period
		///			(so that it restarts precisely at the position that you specified). <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> 
		///			also informs you about the remaining look-ahead time. 
		///
		/// Notes specific to Music Switch Containers:
		///		- Seeking triggers a music transition towards the current (or target) segment.
		///			This transition is subject to the container's transition rule that matches the current and defined in the container,
		///			so the moment when seeking occurs depends on the rule's "Exit At" property. On the other hand, the starting position 
		///			in the target segment depends on both the desired seeking position and the rule's "Sync To" property.
		///		- If the specified time is greater than the destination segment's length, the modulo is taken.
		///
		/// \sa
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::PostEvent()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
		/// - <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SeekOnEvent )( 
			AkUniqueID in_eventID,										///< Unique ID of the event
			AkGameObjectID in_gameObjectID ,							///< Associated game object ID; use AK_INVALID_GAME_OBJECT to affect all game objects
			AkReal32 in_fPercent,										///< Desired position where playback should restart, expressed in a percentage of the file's total duration, between 0 and 1.f (see note above about infinite looping sounds)
			bool in_bSeekToNearestMarker = false,						///< If true, the final seeking position will be made equal to the nearest marker (see note above)	
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID			///< Specify the playing ID for the seek to be applied to. Will result in the seek happening only on active actions of the playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any, to seek on all active actions of this event ID.	
			);

#ifdef AK_SUPPORT_WCHAR
		/// Seeks inside all playing objects that are referenced in Play Actions of the specified Event.
		/// Seek position is specified as a percentage of the sound's total duration, and takes looping into account. 
		///
		/// Notes:
		///		- This works with all objects of the actor-mixer hierarchy, and also with Music Segments and Music Switch Containers. 
		///		- There is a restriction with sounds that play within a continuous sequence. Seeking is ignored 
		///			if one of their ancestors is a continuous (random or sequence) container with crossfade or 
		///			trigger rate transitions. Seeking is also ignored with sample-accurate transitions, unless
		///			the sound that is currently playing is the first sound of the sequence.
		///		- Seeking is also ignored with voices that can go virtual with "From Beginning" behavior. 
		///		- If the option "Seek to nearest marker" is used, the seeking position snaps to the nearest marker.
		///			With objects of the actor-mixer hierarchy, markers are embedded in wave files by an external wave editor.
		///			Note that looping regions ("sampler loop") are not considered as markers. Also, the "add file name marker" of the 
		///			conversion settings dialog adds a marker at the beginning of the file, which is considered when seeking
		///			to nearest marker. In the case of objects of the interactive music hierarchy, user (wave) markers are ignored:
		///			seeking occurs to the nearest segment cue (authored in the segment editor), including the Entry Cue, but excluding the Exit Cue.
		///		- This method posts a command in the sound engine queue, thus seeking will not occur before 
		///			the audio thread consumes it (after a call to RenderAudio()). 
		///
		///	Notes specific to Music Segments:
		///		- With Music Segments, \c in_fPercent is relative to the Entry Cue, and the segment's duration is the 
		///			duration between its entry and exit cues. Consequently, you cannot seek within the pre-entry or
		///			post-exit of a segment using this method. Use absolute values instead.
		///		- Music Segments cannot be looped. You may want to listen to the \c AK_EndOfEvent notification 
		///			in order to restart them if required.
		///		- In order to restart at the correct location, with all their tracks synchronized, Music Segments 
		///			take the "look-ahead time" property of their streamed tracks into account for seeking. 
		///			Consequently, the resulting position after a call to SeekOnEvent() might be earlier than the 
		///			value that was passed to the method. Use <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> to query 
		///			the exact position of a segment. Also, the segment will be silent during the time that period
		///			(so that it restarts precisely at the position that you specified). <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> 
		///			also informs you about the remaining look-ahead time. 
		///
		/// Notes specific to Music Switch Containers:
		///		- Seeking triggers a music transition towards the current (or target) segment.
		///			This transition is subject to the container's transition rule that matches the current and defined in the container,
		///			so the moment when seeking occurs depends on the rule's "Exit At" property. On the other hand, the starting position 
		///			in the target segment depends on both the desired seeking position and the rule's "Sync To" property.
		///		- If the specified time is greater than the destination segment's length, the modulo is taken.
		///
		/// \sa
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::PostEvent()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
		/// - <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SeekOnEvent )( 
			const wchar_t* in_pszEventName,								///< Name of the event
			AkGameObjectID in_gameObjectID ,							///< Associated game object ID; use AK_INVALID_GAME_OBJECT to affect all game objects
			AkReal32 in_fPercent ,										///< Desired position where playback should restart, expressed in a percentage of the file's total duration, between 0 and 1.f (see note above about infinite looping sounds)
			bool in_bSeekToNearestMarker = false,						///< If true, the final seeking position will be made equal to the nearest marker (see note above)	
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID			///< Specify the playing ID for the seek to be applied to. Will result in the seek happening only on active actions of the playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any, to seek on all active actions of this event ID.	
			);
#endif //AK_SUPPORT_WCHAR

		/// Seeks inside all playing objects that are referenced in Play Actions of the specified Event.
		/// Seek position is specified as a percentage of the sound's total duration, and takes looping into account. 
		///
		/// Notes:
		///		- This works with all objects of the actor-mixer hierarchy, and also with Music Segments and Music Switch Containers. 
		///		- There is a restriction with sounds that play within a continuous sequence. Seeking is ignored 
		///			if one of their ancestors is a continuous (random or sequence) container with crossfade or 
		///			trigger rate transitions. Seeking is also ignored with sample-accurate transitions, unless
		///			the sound that is currently playing is the first sound of the sequence.
		///		- Seeking is also ignored with voices that can go virtual with "From Beginning" behavior. 
		///		- If the option "Seek to nearest marker" is used, the seeking position snaps to the nearest marker.
		///			With objects of the actor-mixer hierarchy, markers are embedded in wave files by an external wave editor.
		///			Note that looping regions ("sampler loop") are not considered as markers. Also, the "add file name marker" of the 
		///			conversion settings dialog adds a marker at the beginning of the file, which is considered when seeking
		///			to nearest marker. In the case of objects of the interactive music hierarchy, user (wave) markers are ignored:
		///			seeking occurs to the nearest segment cue (authored in the segment editor), including the Entry Cue, but excluding the Exit Cue.
		///		- This method posts a command in the sound engine queue, thus seeking will not occur before 
		///			the audio thread consumes it (after a call to RenderAudio()). 
		///
		///	Notes specific to Music Segments:
		///		- With Music Segments, in_fPercent is relative to the Entry Cue, and the segment's duration is the 
		///			duration between its entry and exit cues. Consequently, you cannot seek within the pre-entry or
		///			post-exit of a segment using this method. Use absolute values instead.
		///		- Music segments cannot be looped. You may want to listen to the AK_EndOfEvent notification 
		///			in order to restart them if required.
		///		- In order to restart at the correct location, with all their tracks synchronized, Music Segments 
		///			take the "look-ahead time" property of their streamed tracks into account for seeking. 
		///			Consequently, the resulting position after a call to SeekOnEvent() might be earlier than the 
		///			value that was passed to the method. Use <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> to query 
		///			the exact position of a segment. Also, the segment will be silent during the time that period
		///			(so that it restarts precisely at the position that you specified). <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt> 
		///			also informs you about the remaining look-ahead time. 
		///
		/// Notes specific to Music Switch Containers:
		///		- Seeking triggers a music transition towards the current (or target) segment.
		///			This transition is subject to the container's transition rule that matches the current and defined in the container,
		///			so the moment when seeking occurs depends on the rule's "Exit At" property. On the other hand, the starting position 
		///			in the target segment depends on both the desired seeking position and the rule's "Sync To" property.
		///		- If the specified time is greater than the destination segment's length, the modulo is taken.
		///
		/// \sa
		/// - <tt>AK::SoundEngine::RenderAudio()</tt>
		/// - <tt>AK::SoundEngine::PostEvent()</tt>
		/// - <tt>AK::SoundEngine::GetSourcePlayPosition()</tt>
		/// - <tt>AK::MusicEngine::GetPlayingSegmentInfo()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SeekOnEvent )( 
			const char* in_pszEventName,								///< Name of the event
			AkGameObjectID in_gameObjectID,								///< Associated game object ID; use AK_INVALID_GAME_OBJECT to affect all game objects
			AkReal32 in_fPercent,										///< Desired position where playback should restart, expressed in a percentage of the file's total duration, between 0 and 1.f (see note above about infinite looping sounds)
			bool in_bSeekToNearestMarker = false,						///< If true, the final seeking position will be made equal to the nearest marker (see notes above).	
			AkPlayingID in_PlayingID = AK_INVALID_PLAYING_ID			///< Specify the playing ID for the seek to be applied to. Will result in the seek happening only on active actions of the playing ID. Let it be AK_INVALID_PLAYING_ID or do not specify any, to seek on all active actions of this event ID.	
			);

		/// Cancels all Event callbacks associated with a specific callback cookie.\n
		/// \sa 
		/// - \c <tt>AK::SoundEngine::PostEvent()</tt>
		AK_EXTERNAPIFUNC( void, CancelEventCallbackCookie )( 
			void * in_pCookie 							///< Callback cookie to be cancelled
			);

		/// Cancels all Event callbacks associated with a specific game object.\n
		/// \sa 
		/// - \c <tt>AK::SoundEngine::PostEvent()</tt>
		AK_EXTERNAPIFUNC( void, CancelEventCallbackGameObject )(
			AkGameObjectID in_gameObjectID 				///< ID of the game object to be cancelled
			);

		/// Cancels all Event callbacks for a specific playing ID.
		/// \sa 
		/// - \c <tt>AK::SoundEngine::PostEvent()</tt>
		AK_EXTERNAPIFUNC( void, CancelEventCallback )( 
			AkPlayingID in_playingID 					///< Playing ID of the event that must not use callbacks
			);

		/// Gets the current position of the source associated with this playing ID, obtained from PostEvent(). If more than one source is playing,
		/// the first to play is returned.
		/// Notes:
		/// - You need to pass AK_EnableGetSourcePlayPosition to PostEvent() in order to use this function, otherwise
		/// 	it returns AK_Fail, even if the playing ID is valid.
		/// - The source's position is updated at every audio frame, and the time at which this occurs is stored. 
		///		When you call this function from your thread, you therefore query the position that was updated in the previous audio frame.
		///		If in_bExtrapolate is true (default), the returned position is extrapolated using the elapsed time since last 
		///		sound engine update and the source's playback rate.
		/// \return AK_Success if successful.
		///			It returns AK_InvalidParameter if the provided pointer is not valid.
		///			It returns AK_Fail if the playing ID is invalid (not playing yet, or finished playing).
		/// \sa 
		/// - \ref soundengine_query_pos
		/// - \ref concept_events
		AK_EXTERNAPIFUNC( AKRESULT, GetSourcePlayPosition )(
			AkPlayingID		in_PlayingID,				///< Playing ID returned by <tt>AK::SoundEngine::PostEvent()</tt>
			AkTimeMs*		out_puPosition,				///< Position of the source (in ms) associated with the specified playing ID
			bool			in_bExtrapolate = true		///< Position is extrapolated based on time elapsed since last sound engine update.
			);

		/// Gets the current position of the sources associated with this playing ID, obtained from PostEvent().
		/// Notes:
		/// - You need to pass AK_EnableGetSourcePlayPosition to PostEvent() in order to use this function, otherwise
		/// 	it returns AK_Fail, even if the playing ID is valid.
		/// - The source's position is updated at every audio frame, and the time at which this occurs is stored. 
		///		When you call this function from your thread, you therefore query the position that was updated in the previous audio frame.
		///		If in_bExtrapolate is true (default), the returned position is extrapolated using the elapsed time since last 
		///		sound engine update and the source's playback rate.
		/// - If 0 is passed in for the number of entries (*in_pcPositions == 0) then only the number of positions will be returned and the
		///     position array (out_puPositions) will not be updated.
		/// - The io_pcPositions pointer must be non-NULL.
		///     out_puPositions may be NULL if *io_pcPositions == 0, otherwise it must be non-NULL.
		/// \return AK_Success if successful.
		///			It returns AK_InvalidParameter if the provided pointers are not valid.
		///			It returns AK_Fail if the playing ID is invalid (not playing yet, or finished playing).
		/// \sa 
		/// - \ref soundengine_query_pos
		/// - \ref concept_events
		AK_EXTERNAPIFUNC(AKRESULT, GetSourcePlayPositions)(
			AkPlayingID		in_PlayingID,				///< Playing ID returned by <tt>AK::SoundEngine::PostEvent()</tt>
			AkSourcePosition* out_puPositions,			///< Audio Node IDs and positions of sources associated with the specified playing ID
			AkUInt32 *		io_pcPositions,				///< Number of entries in out_puPositions. Needs to be set to the size of the array: it is adjusted to the actual number of returned entries
			bool			in_bExtrapolate = true		///< Position is extrapolated based on time elapsed since last sound engine update
			);

		/// Gets the stream buffering of the sources associated with this playing ID, obtained from PostEvent().
		/// Notes:
		/// - You need to pass AK_EnableGetSourceStreamBuffering to PostEvent() in order to use this function, otherwise
		/// 	it returns AK_Fail, even if the playing ID is valid.
		/// - The sources stream buffering is updated at every audio frame. If there are multiple sources associated with this playing ID,
		///		the value returned corresponds to the least buffered source. 
		/// - The returned buffering status out_bIsBuffering will be true If any of the sources associated with the playing ID are actively being buffered.
		///		It will be false if all of them have reached the end of file, or have reached a state where they are buffered enough and streaming is temporarily idle.
		/// - Purely in-memory sources are excluded from this database. If all sources are in-memory, GetSourceStreamBuffering() will return AK_Fail.
		/// - The returned buffering amount and state is not completely accurate with some hardware-accelerated codecs. In such cases, the amount of stream buffering is generally underestimated.
		///		On the other hand, it is not guaranteed that the source will be ready to produce data at the next audio frame even if out_bIsBuffering has turned to false.
		/// \return 
		///	- AK_Success if successful.
		/// - AK_Fail if the source data associated with this playing ID is not found, for example if PostEvent() was not called with AK_EnableGetSourceStreamBuffering, or if the header was not parsed.
		/// \sa 
		/// - \ref concept_events
		AK_EXTERNAPIFUNC( AKRESULT, GetSourceStreamBuffering )(
			AkPlayingID		in_PlayingID,				///< Playing ID returned by <tt>AK::SoundEngine::PostEvent()</tt>
			AkTimeMs &		out_buffering,				///< Returned amount of buffering (in ms) of the source (or one of the sources) associated with that playing ID
			bool &			out_bIsBuffering			///< Returned buffering status of the source(s) associated with that playing ID
			);

		/// Stops the current content playing associated to the specified game object ID.
		/// If no game object is specified, all sounds will be stopped.
		AK_EXTERNAPIFUNC( void, StopAll )( 
			AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT ///< (Optional)Specify a game object to stop only playback associated to the provided game object ID.
			);

		/// Stop the current content playing associated to the specified playing ID.
		AK_EXTERNAPIFUNC( void, StopPlayingID )( 
			AkPlayingID in_playingID,											///< Playing ID to be stopped.
			AkTimeMs in_uTransitionDuration = 0,								///< Fade duration
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear	///< Curve type to be used for the transition
			);

		/// Sets the random seed value. Can be used to synchronize randomness
		/// across instances of the Sound Engine.
		/// \remark This seeds the number generator used for all container randomizations;
		/// 		since it acts globally, this should be called right before any PostEvent
		///			call where randomness synchronization is required, and cannot guarantee
		///			similar results for continuous containers.
		AK_EXTERNAPIFUNC( void, SetRandomSeed )( 
			AkUInt32 in_uSeed													///< Random seed.
			);

		/// Mutes/Unmutes the busses tagged as background music.  
		/// This is automatically called for platforms that have user-music support.
		/// This function is provided to give the same behavior on platforms that don't have user-music support.
		AK_EXTERNAPIFUNC( void, MuteBackgroundMusic ) (
			bool in_bMute ///< Sets true to mute, false to unmute.
			);
		//@}

		/// Gets the state of the Background Music busses. This state is either set directly 
		/// with \c AK::SoundEngine::MuteBackgroundMusic or by the OS, if it has User Music services.
		/// \return true if the background music busses are muted, false if not.
		AK_EXTERNAPIFUNC(bool, GetBackgroundMusicMute) ();
		//@}


		/// Sends custom game data to a plug-in that resides on a bus (insert Effect or mixer plug-in).
		/// Data will be copied and stored into a separate list.
		/// Previous entry is deleted when a new one is sent.
		/// Sets the data pointer to NULL to clear item from the list.
		/// \aknote The plug-in type and ID is passed and matched with plugins set on the desired bus. 
		/// This means that you cannot send different data to various instances of the plug-in on a same bus.\endaknote
		/// \return AK_Success if data was sent successfully.
		AK_EXTERNAPIFUNC( AKRESULT, SendPluginCustomGameData ) (
			AkUniqueID in_busID,			///< Bus ID
			AkGameObjectID in_busObjectID,	///< Bus Object ID. Pass AK_INVALID_GAME_OBJECT to send custom data with global scope. Game object scope supersedes global scope, as with RTPCs. 
			AkPluginType in_eType,			///< Plug-in type (for example, source or effect)
			AkUInt32 in_uCompanyID,		///< Company identifier (as declared in the plug-in description XML file)
			AkUInt32 in_uPluginID,			///< Plug-in identifier (as declared in the plug-in description XML file)
			const void* in_pData,			///< The data blob
			AkUInt32 in_uSizeInBytes		///< Size of data
			);
		//@}

		////////////////////////////////////////////////////////////////////////
		/// @name Game Objects
		//@{
		
        /// Registers a game object.
		/// \return
		/// - AK_Success if successful
		///	- AK_Fail if the specified AkGameObjectID is invalid (0 and -1 are invalid)
		/// \remark Registering a game object twice does nothing. Unregistering it once unregisters it no 
		///			matter how many times it has been registered.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnregisterGameObj()</tt>
		/// - <tt>AK::SoundEngine::UnregisterAllGameObj()</tt>
		/// - \ref concept_gameobjects
		AK_EXTERNAPIFUNC(AKRESULT, RegisterGameObj)(
			AkGameObjectID in_gameObjectID								///< ID of the game object to be registered			
	        );

		/// Registers a game object.
		/// \return
		/// - AK_Success if successful
		///	- AK_Fail if the specified AkGameObjectID is invalid (0 and -1 are invalid)
		/// \remark Registering a game object twice does nothing. Unregistering it once unregisters it no 
		///			matter how many times it has been registered.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnregisterGameObj()</tt>
		/// - <tt>AK::SoundEngine::UnregisterAllGameObj()</tt>
		/// - \ref concept_gameobjects
        AK_EXTERNAPIFUNC( AKRESULT, RegisterGameObj )(
	        AkGameObjectID in_gameObjectID,							///< ID of the game object to be registered
			const char * in_pszObjName								///< Name of the game object (for monitoring purpose)
	        );

        /// Unregisters a game object.
		/// \return 
		/// - AK_Success if successful
		///	- AK_Fail if the specified AkGameObjectID is invalid (0 is an invalid ID)
		/// \remark Registering a game object twice does nothing. Unregistering it once unregisters it no 
		///			matter how many times it has been registered. Unregistering a game object while it is 
		///			in use is allowed, but the control over the parameters of this game object is lost.
		///			For example, say a sound associated with this game object is a 3D moving sound. This sound will 
		///			stop moving when the game object is unregistered, and there will be no way to regain control over the game object.
		/// \sa 
		/// - <tt>AK::SoundEngine::RegisterGameObj()</tt>
		/// - <tt>AK::SoundEngine::UnregisterAllGameObj()</tt>
		/// - \ref concept_gameobjects
        AK_EXTERNAPIFUNC( AKRESULT, UnregisterGameObj )(
	        AkGameObjectID in_gameObjectID				///< ID of the game object to be unregistered. Use 
	        											/// AK_INVALID_GAME_OBJECT to unregister all game objects.
	        );

    /// Unregister all game objects, or all game objects with a particular matching set of property flags.
	/// This function to can be used to unregister all game objects.  
	/// \return
	/// - AK_Success if successful
	/// \remark Registering a game object twice does nothing. Unregistering it once unregisters it no 
	///			matter how many times it has been registered. Unregistering a game object while it is 
	///			in use is allowed, but the control over the parameters of this game object is lost.
	///			For example, if a sound associated with this game object is a 3D moving sound, it will 
	///			stop moving once the game object is unregistered, and there will be no way to recover 
	///			the control over this game object.
	/// \sa 
	/// - <tt>AK::SoundEngine::RegisterGameObj()</tt>
	/// - <tt>AK::SoundEngine::UnregisterGameObj()</tt>
	/// - \ref concept_gameobjects
    AK_EXTERNAPIFUNC( AKRESULT, UnregisterAllGameObj )(
		);

       	/// Sets the position of a game object.
		/// \warning The object's orientation vector (in_Position.Orientation) must be normalized.
		/// \return 
		/// - AK_Success when successful
		/// - AK_InvalidParameter if parameters are not valid.
		/// \sa 
		/// - \ref soundengine_3dpositions
        AK_EXTERNAPIFUNC( AKRESULT, SetPosition )( 
			AkGameObjectID in_GameObjectID,		///< Game Object identifier
			const AkSoundPosition & in_Position	///< Position to set; in_Position.Orientation must be normalized.
		    );

		/// Sets multiple positions to a single game object.
		/// Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
		/// This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
		/// \aknote Calling <tt>AK::SoundEngine::SetMultiplePositions()</tt> with only one position is the same as calling <tt>AK::SoundEngine::SetPosition()</tt> \endaknote
		/// \return 
		/// - AK_Success when successful
		/// - AK_InvalidParameter if parameters are not valid.
		/// \sa 
		/// - \ref soundengine_3dpositions
		/// - \ref soundengine_3dpositions_multiplepos
		/// - \ref AK::SoundEngine::MultiPositionType
        AK_EXTERNAPIFUNC( AKRESULT, SetMultiplePositions )( 
			AkGameObjectID in_GameObjectID,						///< Game Object identifier.
			const AkSoundPosition * in_pPositions,				///< Array of positions to apply.
			AkUInt16 in_NumPositions,							///< Number of positions specified in the provided array.
			MultiPositionType in_eMultiPositionType = MultiPositionType_MultiDirections ///< \ref AK::SoundEngine::MultiPositionType
		    );

		/// Sets multiple positions to a single game object, with flexible assignment of input channels.
		/// Setting multiple positions on a single game object is a way to simulate multiple emission sources while using the resources of only one voice.
		/// This can be used to simulate wall openings, area sounds, or multiple objects emitting the same sound in the same area.
		/// \aknote Calling <tt>AK::SoundEngine::SetMultiplePositions()</tt> with only one position is the same as calling <tt>AK::SoundEngine::SetPosition()</tt> \endaknote
		/// \return 
		/// - AK_Success when successful
		/// - AK_InvalidParameter if parameters are not valid.
		/// \sa 
		/// - \ref soundengine_3dpositions
		/// - \ref soundengine_3dpositions_multiplepos
		/// - \ref AK::SoundEngine::MultiPositionType
        AK_EXTERNAPIFUNC( AKRESULT, SetMultiplePositions )( 
			AkGameObjectID in_GameObjectID,						///< Game Object identifier.
			const AkChannelEmitter * in_pPositions,			///< Array of positions to apply.
			AkUInt16 in_NumPositions,							///< Number of positions specified in the provided array.
			MultiPositionType in_eMultiPositionType = MultiPositionType_MultiDirections ///< \ref AK::SoundEngine::MultiPositionType
		    );

        /// Sets the scaling factor of a Game Object.
		/// Modify the attenuation computations on this Game Object to simulate sounds with a larger or smaller area of effect.
		/// \return 
		/// - AK_Success when successful
		/// - AK_InvalidParameter if the scaling factor specified was 0 or negative.
		AK_EXTERNAPIFUNC( AKRESULT, SetScalingFactor )(
			AkGameObjectID in_GameObjectID,				///< Game object identifier
			AkReal32 in_fAttenuationScalingFactor		///< Scaling Factor, 1 means 100%, 0.5 means 50%, 2 means 200%, and so on.
			);

        //@}

		////////////////////////////////////////////////////////////////////////
		/// @name Bank Management
		//@{

		/// Unload all currently loaded banks.
		/// It also internally calls ClearPreparedEvents() since at least one bank must have been loaded to allow preparing events.
		/// \return 
		/// - AK_Success if successful
		///	- AK_Fail if the sound engine was not correctly initialized or if there is not enough memory to handle the command
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - \ref soundengine_banks
		AK_EXTERNAPIFUNC( AKRESULT, ClearBanks )();

		/// Sets the I/O settings of the bank load and prepare event processes.
        /// The sound engine uses default values unless explicitly set by calling this method.
		/// \warning This function must be called before loading banks.
		/// \return 
		/// - AK_Success if successful
		/// - AK_Fail if the sound engine was not correctly initialized
		/// - AK_InvalidParameter if some parameters are invalid
		/// \sa 
		/// - \ref soundengine_banks
        /// - \ref streamingdevicemanager
        AK_EXTERNAPIFUNC( AKRESULT, SetBankLoadIOSettings )(
            AkReal32            in_fThroughput,         ///< Average throughput of bank data streaming (bytes/ms) (the default value is AK_DEFAULT_BANK_THROUGHPUT)
            AkPriority          in_priority             ///< Priority of bank streaming (the default value is AK_DEFAULT_PRIORITY)
            );

#ifdef AK_SUPPORT_WCHAR
		/// Load a bank synchronously (by Unicode string).\n
		/// The bank name is passed to the Stream Manager.
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// You can specify a custom pool for storage of media, the engine will create a new pool if AK_DEFAULT_POOL_ID is passed.
		/// A bank load request will be posted, and consumed by the Bank Manager thread.
		/// The function returns when the request has been completely processed.
		/// \return 
		/// The bank ID, which is obtained by hashing the bank name (see GetIDFromString()). 
		/// You may use this ID with UnloadBank().
		///	- AK_Success: Load or unload successful.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The sound engine internally calls GetIDFromString(in_pszString) to return the correct bank ID.
		/// Therefore, in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. The path should be resolved in 
		/// your implementation of the Stream Manager, or in the Low-Level I/O module if you use the default Stream Manager's implementation.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank() function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::MemoryMgr::CreatePool()</tt>
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref streamingdevicemanager
		/// - \ref streamingmanager_lowlevel
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
	        const wchar_t*      in_pszString,		    ///< Name of the bank to load
            AkMemPoolId         in_memPoolId,			///< Memory pool ID (the pool is created if AK_DEFAULT_POOL_ID is passed)
            AkBankID &          out_bankID				///< Returned bank ID
	        );
#endif //AK_SUPPORT_WCHAR

		/// Loads a bank synchronously.\n
		/// The bank name is passed to the Stream Manager.
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// You can specify a custom pool for storage of media, the engine will create a new pool if AK_DEFAULT_POOL_ID is passed.
		/// A bank load request will be posted, and consumed by the Bank Manager thread.
		/// The function returns when the request has been completely processed.
		/// \return 
		/// The bank ID, which is obtained by hashing the bank name (see GetIDFromString()). 
		/// You may use this ID with UnloadBank().
		///	- AK_Success: Load or unload successful.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The sound engine internally calls GetIDFromString(in_pszString) to return the correct bank ID.
		/// Therefore, in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. The path should be resolved in 
		/// your implementation of the Stream Manager, or in the Low-Level I/O module if you use the default Stream Manager's implementation.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank() function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString</tt>
		/// - <tt>AK::MemoryMgr::CreatePool()</tt>
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref streamingdevicemanager
		/// - \ref streamingmanager_lowlevel
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
	        const char*         in_pszString,		    ///< Name of the bank to load
            AkMemPoolId         in_memPoolId,			///< Memory pool ID (the pool is created if AK_DEFAULT_POOL_ID is passed)
            AkBankID &          out_bankID				///< Returned bank ID
	        );

        /// Loads a bank synchronously (by ID).\n
		/// \aknote Requires that the "Use SoundBank names" option be unchecked in the Wwise Project Settings. \endaknote
		/// The bank ID is passed to the Stream Manager.
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// You can specify a custom pool for storage of media, the engine will create a new pool if AK_DEFAULT_POOL_ID is passed.
		/// A bank load request will be posted, and consumed by the Bank Manager thread.
		/// The function returns when the request has been completely processed.
		/// \return 
		///	- AK_Success: Load or unload successful.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - <tt>AK::MemoryMgr::CreatePool()</tt>
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
	        AkBankID			in_bankID,              ///< Bank ID of the bank to load
            AkMemPoolId         in_memPoolId			///< Memory pool ID (the pool is created if AK_DEFAULT_POOL_ID is passed)
            );

		/// Loads a bank synchronously (from in-memory data, in-place).\n
		///
		/// IMPORTANT: Banks loaded from memory with in-place data MUST be unloaded using the UnloadBank function
		/// providing the same memory pointer. Make sure you are using the correct UnloadBank(...) overload
		///
		/// Use this overload when you want to manage I/O on your side. Load the bank file
		/// in a buffer and pass its address to the sound engine.
		/// In-memory loading is in-place: *** the memory must be valid until the bank is unloaded. ***
		/// A bank load request will be posted, and consumed by the Bank Manager thread.
		/// The function returns when the request has been completely processed.
		/// \return 
		/// The bank ID, which is stored in the first few bytes of the bank file. You may use this 
		/// ID with UnloadBank().
		///	- AK_Success: Load or unload successful.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The memory must be aligned on platform-specific AK_BANK_PLATFORM_DATA_ALIGNMENT bytes (see AkTypes.h).
		/// - (XboxOne only): If the bank may contain XMA in memory data, the memory must be allocated using the Physical memory allocator.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank function.
		/// - Avoid using this function for banks containing a lot of events or structure data.  
		///	  This data will be loaded in the Default Pool anyway thus duplicating memory (one copy in the Default Pool 
		///   and one in the block you provided). For event/structure-only banks, prefer the other versions of LoadBank().
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
	        const void *		in_pInMemoryBankPtr,	///< Pointer to the in-memory bank to load (pointer is stored in sound engine, memory must remain valid)
			AkUInt32			in_uInMemoryBankSize,	///< Size of the in-memory bank to load
            AkBankID &          out_bankID				///< Returned bank ID
	        );

		/// Loads a bank synchronously (from in-memory data, out-of-place).\n
		///
		/// NOTE: Banks loaded from in-memory with out-of-place data must be unloaded using the standard UnloadBank function
		/// (with no memory pointer). Make sure you are using the correct UnloadBank(...) overload
		///
		/// Use this overload when you want to manage I/O on your side. Load the bank file
		/// in a buffer and pass its address to the sound engine, the media section of the bank will be copied into the 
		/// specified memory pool.  
		/// In-memory loading is out-of-place: the buffer can be release as soon as the function returns. The advantage of using this
		/// over the in-place version is that there is no duplication of bank structures.
		/// A bank load request will be posted, and consumed by the Bank Manager thread.
		/// The function returns when the request has been completely processed.
		/// \return 
		/// The bank ID, which is stored in the first few bytes of the bank file. You may use this 
		/// ID with UnloadBank().
		///	- AK_Success: Load or unload successful.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The memory must be aligned on platform-specific AK_BANK_PLATFORM_DATA_ALIGNMENT bytes (see AkTypes.h).
		/// - (XboxOne only): If the bank may contain XMA in memory data, the memory must be allocated using the Physical memory allocator.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank function.
		/// - Avoid using this function for banks containing a lot of events or structure data.  
		///	  This data will be loaded in the Default Pool anyway thus duplicating memory (one copy in the Default Pool 
		///   and one in the block you provided). For event/structure-only banks, prefer the other versions of LoadBank().
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
			const void *		in_pInMemoryBankPtr,	///< Pointer to the in-memory bank to load (pointer is not stored in sound engine, memory can be released after return)
			AkUInt32			in_uInMemoryBankSize,	///< Size of the in-memory bank to load
			AkMemPoolId			in_uPoolForBankMedia,	///< Memory pool to copy the media section of the bank to (the pool is created if AK_DEFAULT_POOL_ID is passed).
			AkBankID &          out_bankID				///< Returned bank ID
			);

		/// Synchronously decode Vorbis-encoded and Opus-encoded(Software version) media in a SoundBank.
		AK_EXTERNAPIFUNC( AKRESULT, DecodeBank )(
			const void *		in_pInMemoryBankPtr,	///< Pointer to the in-memory bank to decode (pointer is not stored in sound engine, memory can be released after return)
			AkUInt32			in_uInMemoryBankSize,	///< Size of the in-memory bank to decode
			AkMemPoolId			in_uPoolForDecodedBank,	///< Memory pool to allocate decoded bank into. Specify AK_INVALID_POOL_ID and out_pDecodedBankPtr=NULL to obtain decoded bank size without performing the decode operation. Pass AK_INVALID_POOL_ID and out_pDecodedBankPtr!=NULL to decode bank into specified pointer.
			void * &			out_pDecodedBankPtr,	///< Decoded bank memory location.
			AkUInt32 &			out_uDecodedBankSize	///< Decoded bank memory size.
			);

#ifdef AK_SUPPORT_WCHAR
        /// Loads a bank asynchronously (by Unicode string).\n
		/// The bank name is passed to the Stream Manager.
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// You can specify a custom pool for storage of media, the engine will create a new pool if AK_DEFAULT_POOL_ID is passed.
		/// A bank load request will be posted to the Bank Manager consumer thread.
		/// The function returns immediately.
		/// \return 
		/// AK_Success if the scheduling was successful, AK_Fail otherwise.
		/// Use a callback to be notified when completed, and get the status of the request.
		/// The bank ID, which is obtained by hashing the bank name (see GetIDFromString()). 
		/// You may use this ID with UnloadBank().
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The sound engine internally calls GetIDFromString(in_pszString) to return the correct bank ID.
		/// Therefore, in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. The path should be resolved in 
		/// your implementation of the Stream Manager (<tt>AK::IAkStreamMgr::CreateStd()</tt>), or in the Low-Level I/O module 
		/// (<tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt>) if you use the default Stream Manager's implementation.
		/// - The cookie (in_pCookie) is passed to the Low-Level I/O module for your convenience, in <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> 
		// as AkFileSystemFlags::pCustomParam.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - <tt>AK::MemoryMgr::CreatePool()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref streamingdevicemanager
		/// - \ref streamingmanager_lowlevel
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
	        const wchar_t*      in_pszString,           ///< Name/path of the bank to load
			AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
			void *              in_pCookie,				///< Callback cookie (reserved to user, passed to the callback function, and also to  <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> as AkFileSystemFlags::pCustomParam)
            AkMemPoolId         in_memPoolId,			///< Memory pool ID (the pool is created if AK_DEFAULT_POOL_ID is passed)
			AkBankID &          out_bankID				///< Returned bank ID
	        );
#endif //AK_SUPPORT_WCHAR

        /// Loads a bank asynchronously.\n
		/// The bank name is passed to the Stream Manager.
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// You can specify a custom pool for storage of media, the engine will create a new pool if AK_DEFAULT_POOL_ID is passed.
		/// A bank load request will be posted to the Bank Manager consumer thread.
		/// The function returns immediately.
		/// \return 
		/// AK_Success if the scheduling was successful, AK_Fail otherwise.
		/// Use a callback to be notified when completed, and get the status of the request.
		/// The bank ID, which is obtained by hashing the bank name (see GetIDFromString()). 
		/// You may use this ID with UnloadBank().
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the Event will fail.
		/// - The sound engine internally calls GetIDFromString(in_pszString) to return the correct bank ID.
		/// Therefore, \c in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. The path should be resolved in 
		/// your implementation of the Stream Manager (<tt>AK::IAkStreamMgr::CreateStd()</tt>), or in the Low-Level I/O module 
		/// (<tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt>) if you use the default Stream Manager's implementation.
		/// - The cookie (in_pCookie) is passed to the Low-Level I/O module for your convenience, in <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> 
		// as <tt>AkFileSystemFlags::pCustomParam</tt>.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the \c UnloadBank function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - <tt>AK::MemoryMgr::CreatePool()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref streamingdevicemanager
		/// - \ref streamingmanager_lowlevel
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
	        const char*         in_pszString,			///< Name/path of the bank to load
			AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
			void *              in_pCookie,				///< Callback cookie (reserved to user, passed to the callback function, and also to  <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> as AkFileSystemFlags::pCustomParam)
            AkMemPoolId         in_memPoolId,			///< Memory pool ID (the pool is created if AK_DEFAULT_POOL_ID is passed)
			AkBankID &          out_bankID				///< Returned bank ID
	        );

        /// Loads a bank asynchronously (by ID).\n
		/// \aknote Requires that the "Use SoundBank names" option be unchecked in the Wwise Project Settings. \endaknote
		/// The bank ID is passed to the Stream Manager.
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// You can specify a custom pool for storage of media, the engine will create a new pool if AK_DEFAULT_POOL_ID is passed.
		/// A bank load request will be posted to the Bank Manager consumer thread.
		/// The function returns immediately.
		/// \return 
		/// AK_Success if the scheduling was successful, AK_Fail otherwise.
		/// Use a callback to be notified when completed, and get the status of the request.
		/// The bank ID, which is obtained by hashing the bank name (see GetIDFromString()). 
		/// You may use this ID with \c UnloadBank().
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The file path should be resolved in your implementation of the Stream Manager, or in the Low-Level I/O module if 
		/// you use the default Stream Manager's implementation. The ID overload of <tt>AK::IAkStreamMgr::CreateStd()</tt> and <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> are called.
		/// - The cookie (in_pCookie) is passed to the Low-Level I/O module for your convenience, in <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> 
		// as AkFileSystemFlags::pCustomParam.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - <tt>AK::MemoryMgr::CreatePool()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
	        AkBankID			in_bankID,				///< Bank ID of the bank to load
			AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
			void *              in_pCookie,				///< Callback cookie (reserved to user, passed to the callback function, and also to  <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt> as AkFileSystemFlags::pCustomParam)
            AkMemPoolId         in_memPoolId			///< Memory pool ID (the pool is created if AK_DEFAULT_POOL_ID is passed)
	        );

		/// Loads a bank asynchronously (from in-memory data, in-place).\n
		///
		/// IMPORTANT: Banks loaded from memory with in-place data MUST be unloaded using the UnloadBank function
		/// providing the same memory pointer. Make sure you are using the correct UnloadBank(...) overload
		///
		/// Use this overload when you want to manage I/O on your side. Load the bank file
		/// in a buffer and pass its address to the sound engine.
		/// In-memory loading is in-place: *** the memory must be valid until the bank is unloaded. ***
		/// A bank load request will be posted to the Bank Manager consumer thread.
		/// The function returns immediately.
		/// \return 
		/// AK_Success if the scheduling was successful, AK_Fail otherwise, or AK_InvalidParameter if memory alignment is not correct.
		/// Use a callback to be notified when completed, and get the status of the request.
		/// The bank ID, which is obtained by hashing the bank name (see GetIDFromString()). 
		/// You may use this ID with UnloadBank().
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The memory must be aligned on platform-specific AK_BANK_PLATFORM_DATA_ALIGNMENT bytes (see AkTypes.h).
		/// - (XboxOne only): If the bank may contain XMA in memory data, the memory must be allocated using the Physical memory allocator.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
			const void *		in_pInMemoryBankPtr,	///< Pointer to the in-memory bank to load (pointer is stored in sound engine, memory must remain valid)
			AkUInt32			in_uInMemoryBankSize,	///< Size of the in-memory bank to load
			AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
			void *              in_pCookie,				///< Callback cookie
			AkBankID &          out_bankID				///< Returned bank ID
	        );

		/// Loads a bank asynchronously (from in-memory data, out-of-place).\n
		///
		/// NOTE: Banks loaded from in-memory with out-of-place data must be unloaded using the standard UnloadBank function
		/// (with no memory pointer). Make sure you are using the correct UnloadBank(...) overload
		///
		/// Use this overload when you want to manage I/O on your side. Load the bank file
		/// in a buffer and pass its address to the sound engine, the media section of the bank will be copied into the 
		/// specified memory pool.  
		/// In-memory loading is out-of-place: the buffer can be released after the callback function is called. The advantage of using this
		/// over the in-place version is that there is no duplication of bank structures.
		/// A bank load request will be posted to the Bank Manager consumer thread.
		/// The function returns immediately.
		/// \return 
		/// AK_Success if the scheduling was successful, AK_Fail otherwise, or AK_InvalidParameter if memory alignment is not correct.
		/// Use a callback to be notified when completed, and get the status of the request.
		/// The bank ID, which is obtained by hashing the bank name (see GetIDFromString()). 
		/// You may use this ID with UnloadBank().
		/// \remarks
		/// - The initialization bank must be loaded first.
		/// - All SoundBanks subsequently loaded must come from the same Wwise project as the
		///   initialization bank. If you need to load SoundBanks from a different project, you
		///   must first unload ALL banks, including the initialization bank, then load the
		///   initialization bank from the other project, and finally load banks from that project.
		/// - Codecs and plug-ins must be registered before loading banks that use them.
		/// - Loading a bank referencing an unregistered plug-in or codec will result in a load bank success,
		/// but the plug-ins will not be used. More specifically, playing a sound that uses an unregistered effect plug-in 
		/// will result in audio playback without applying the said effect. If an unregistered source plug-in is used by an event's audio objects, 
		/// posting the event will fail.
		/// - The memory must be aligned on platform-specific AK_BANK_PLATFORM_DATA_ALIGNMENT bytes (see AkTypes.h).
		/// - (XboxOne only): If the bank may contain XMA in memory data, the memory must be allocated using the Physical memory allocator.
		/// - Requesting to load a bank in a different memory pool than where the bank was previously loaded must be done only
		/// after receiving confirmation by the callback that the bank was completely unloaded or by using synchronous versions
		/// of the UnloadBank function.
		/// \sa 
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref integrating_elements_plugins
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, LoadBank )(
			const void *		in_pInMemoryBankPtr,	///< Pointer to the in-memory bank to load (pointer is not stored in sound engine, memory can be released after callback)
			AkUInt32			in_uInMemoryBankSize,	///< Size of the in-memory bank to load
			AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
			void *              in_pCookie,				///< Callback cookie
			AkMemPoolId			in_uPoolForBankMedia,	///< Memory pool to copy the media section of the bank to (the pool is created if AK_DEFAULT_POOL_ID is passed).
			AkBankID &          out_bankID				///< Returned bank ID
			);

#ifdef AK_SUPPORT_WCHAR
        /// Unloads a bank synchronously (by Unicode string).\n
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// \return AK_Success if successful, AK_Fail otherwise. AK_Success is returned when the bank was not loaded.
		/// \remarks
		/// - If you provided a pool memory ID when loading this bank, it is returned as well. 
		/// Otherwise, the function returns AK_DEFAULT_POOL_ID.
		/// - The sound engine internally calls GetIDFromString(in_pszString) to retrieve the bank ID, 
		/// then it calls the synchronous version of UnloadBank() by ID.
		/// Therefore, in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. 
		/// - In order to force the memory deallocation of the bank, sounds that use media from this bank will be stopped. 
		/// This means that streamed sounds or generated sounds will not be stopped.
		/// \sa 
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - \ref soundengine_banks
        AK_EXTERNAPIFUNC( AKRESULT, UnloadBank )(
	        const wchar_t*      in_pszString,           ///< Name of the bank to unload
			const void *		in_pInMemoryBankPtr,	///< Memory pointer from where the bank was initially loaded from. (REQUIRED to determine which bank associated to a memory pointer must be unloaded). Pass NULL only if NULL was passed when loading the bank.
	        AkMemPoolId *       out_pMemPoolId = NULL   ///< Returned memory pool ID used with LoadBank() (can pass NULL)
	        );
#endif //AK_SUPPORT_WCHAR

        /// Unloads a bank synchronously.\n
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// \return AK_Success if successful, AK_Fail otherwise. AK_Success is returned when the bank was not loaded.
		/// \remarks
		/// - If you provided a pool memory ID when loading this bank, it is returned as well. 
		/// Otherwise, the function returns AK_DEFAULT_POOL_ID.
		/// - The sound engine internally calls GetIDFromString(in_pszString) to retrieve the bank ID, 
		/// then it calls the synchronous version of UnloadBank() by ID.
		/// Therefore, in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. 
		/// - In order to force the memory deallocation of the bank, sounds that use media from this bank will be stopped. 
		/// This means that streamed sounds or generated sounds will not be stopped.
		/// \sa 
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - \ref soundengine_banks
        AK_EXTERNAPIFUNC( AKRESULT, UnloadBank )(
	        const char*         in_pszString,           ///< Name of the bank to unload
			const void *		in_pInMemoryBankPtr,	///< Memory pointer from where the bank was initially loaded from. (REQUIRED to determine which bank associated to a memory pointer must be unloaded). Pass NULL only if NULL was passed when loading the bank.
	        AkMemPoolId *       out_pMemPoolId = NULL   ///< Returned memory pool ID used with LoadBank() (can pass NULL)
	        );

		/// Unloads a bank synchronously (by ID and memory pointer).\n
		/// \return AK_Success if successful, AK_Fail otherwise. AK_Success is returned when the bank was not loaded.
		/// \remarks
		/// If you provided a pool memory ID when loading this bank, it is returned as well. 
		/// Otherwise, the function returns AK_DEFAULT_POOL_ID.
		/// - In order to force the memory deallocation of the bank, sounds that use media from this bank will be stopped. 
		/// This means that streamed sounds or generated sounds will not be stopped.
		/// \sa 
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - \ref soundengine_banks
        AK_EXTERNAPIFUNC( AKRESULT, UnloadBank )(
	        AkBankID            in_bankID,              ///< ID of the bank to unload
			const void *		in_pInMemoryBankPtr,	///< Memory pointer from where the bank was initially loaded from. (REQUIRED to determine which bank associated to a memory pointer must be unloaded). Pass NULL only if NULL was passed when loading the bank.
            AkMemPoolId *       out_pMemPoolId = NULL   ///< Returned memory pool ID used with LoadBank() (can pass NULL)
	        );

#ifdef AK_SUPPORT_WCHAR
		/// Unloads a bank asynchronously (by Unicode string).\n
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// \return AK_Success if scheduling successful (use a callback to be notified when completed)
		/// \remarks
		/// The sound engine internally calls GetIDFromString(in_pszString) to retrieve the bank ID, 
		/// then it calls the synchronous version of UnloadBank() by ID.
		/// Therefore, in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. 
		/// - In order to force the memory deallocation of the bank, sounds that use media from this bank will be stopped. 
		/// This means that streamed sounds or generated sounds will not be stopped.
		/// \sa 
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		AK_EXTERNAPIFUNC( AKRESULT, UnloadBank )(
	        const wchar_t*      in_pszString,           ///< Name of the bank to unload
			const void *		in_pInMemoryBankPtr,	///< Memory pointer from where the bank was initially loaded from. (REQUIRED to determine which bank associated to a memory pointer must be unloaded). Pass NULL only if NULL was passed when loading the bank.
			AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
			void *              in_pCookie 				///< Callback cookie (reserved to user, passed to the callback function)
	        );
#endif //AK_SUPPORT_WCHAR

		/// Unloads a bank asynchronously.\n
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// \return AK_Success if scheduling successful (use a callback to be notified when completed)
		/// \remarks
		/// The sound engine internally calls GetIDFromString(in_pszString) to retrieve the bank ID, 
		/// then it calls the synchronous version of UnloadBank() by ID.
		/// Therefore, in_pszString should be the real name of the SoundBank (with or without the BNK extension - it is trimmed internally),
		/// not the name of the file (if you changed it), nor the full path of the file. 
		/// - In order to force the memory deallocation of the bank, sounds that use media from this bank will be stopped. 
		/// This means that streamed sounds or generated sounds will not be stopped.
		/// \sa 
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		AK_EXTERNAPIFUNC( AKRESULT, UnloadBank )(
	        const char*         in_pszString,           ///< Name of the bank to unload
			const void *		in_pInMemoryBankPtr,	///< Memory pointer from where the bank was initially loaded from. (REQUIRED to determine which bank associated to a memory pointer must be unloaded). Pass NULL only if NULL was passed when loading the bank.
			AkBankCallbackFunc  in_pfnBankCallback,	    ///< Callback function
			void *              in_pCookie 				///< Callback cookie (reserved to user, passed to the callback function)
	        );

		/// Unloads a bank asynchronously (by ID and memory pointer).\n
		/// Refer to \ref soundengine_banks_general for a discussion on using strings and IDs.
		/// \return AK_Success if scheduling successful (use a callback to be notified when completed)
		/// \remarks
		/// - In order to force the memory deallocation of the bank, sounds that use media from this bank will be stopped. 
		/// This means that streamed sounds or generated sounds will not be stopped.
		/// \sa 
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		AK_EXTERNAPIFUNC( AKRESULT, UnloadBank )(
	        AkBankID            in_bankID,				///< ID of the bank to unload
			const void *		in_pInMemoryBankPtr,	///< Memory pointer from where the bank was initially loaded from. (REQUIRED to determine which bank associated to a memory pointer must be unloaded). Pass NULL only if NULL was passed when loading the bank.
			AkBankCallbackFunc  in_pfnBankCallback,		///< Callback function
			void *              in_pCookie				///< Callback cookie (reserved to user, passed to the callback function)
	        );

		/// Cancels all Event callbacks associated with a specific callback cookie specified while loading Banks of preparing Events.\n
		/// \sa 
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::UnloadBank()</tt>
		/// - <tt>AK::SoundEngine::ClearBanks()</tt>
		/// - AkBankCallbackFunc
		AK_EXTERNAPIFUNC( void, CancelBankCallbackCookie )( 
			void * in_pCookie 							///< Callback cookie to be canceled
			);

		/// Preparation type.
		/// \sa
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::PrepareGameSyncs()</tt>
		/// - <tt>AK::SoundEngine::PrepareBank()</tt>
		enum PreparationType
		{
			Preparation_Load,	///< \c PrepareEvent() will load required information to play the specified event.
			Preparation_Unload,	///< \c PrepareEvent() will unload required information to play the specified event.
			Preparation_LoadAndDecode ///< Vorbis media is decoded when loading, and an uncompressed PCM version is used for playback.
		};

		/// Parameter to be passed to <tt>AK::SoundEngine::PrepareBank()</tt>.
		/// Use AkBankContent_All to load both the media and structural content from the bank. 
		/// Use AkBankContent_StructureOnly to load only the structural content, including events, from the bank and then later use the PrepareEvent() functions to load media on demand from loose files on the disk.
		/// \sa 
		/// - <tt>AK::SoundEngine::PrepareBank()</tt>
		/// - \ref soundengine_banks_preparingbanks
		enum AkBankContent
		{
			AkBankContent_StructureOnly,	///< Use AkBankContent_StructureOnly to load only the structural content, including Events, and then later use the PrepareEvent() functions to load media on demand from loose files on the disk.
			AkBankContent_All				///< Use AkBankContent_All to load both the media and structural content.
		};

#ifdef AK_SUPPORT_WCHAR
		/// This function will load the Events, structural content, and optionally, the media content from the SoundBank. If the flag AkBankContent_All is specified, PrepareBank() will load the media content from 
		/// the bank; but, as opposed to LoadBank(), the media will be loaded one media item at a time instead of in one contiguous memory block. Using PrepareBank(), alone or in combination with PrepareEvent(), 
		/// will prevent in-memory duplication of media at the cost of some memory fragmentation.
		/// Calling this function specifying the flag AkBankContent_StructureOnly will load only the structural part (including events) of this bank, 
		/// allowing using PrepareEvent() to load media on demand.  
		/// \sa 
		/// - \ref soundengine_banks_preparingbanks
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::PreparationType</tt>
		/// \remarks
		/// PrepareBank(), when called with the flag AkBankContent_StructureOnly, requires additional calls to PrepareEvent() to load the media for each event. PrepareEvent(), however, is unable to 
		///		access media content contained within SoundBanks and requires that the media be available as loose files in the file system. This flag may be useful to implement multiple loading configurations;
		///	for example, a game may have a tool mode that uses PrepareEvent() to load loose files on-demand and, also, a game mode that uses LoadBank() to load the bank in entirety.
		AK_EXTERNAPIFUNC( AKRESULT, PrepareBank )(
			AK::SoundEngine::PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			const wchar_t*        in_pszString,								///< Name of the bank to Prepare/Unprepare.
			AK::SoundEngine::AkBankContent	in_uFlags = AkBankContent_All	///< Structures only (including events) or all content.
			);
#endif //AK_SUPPORT_WCHAR

		/// This function will load the Events, structural content, and optionally, the media content from the SoundBank. If the flag AkBankContent_All is specified, PrepareBank() will load the media content from 
		/// the bank; but, as opposed to LoadBank(), the media will be loaded one media item at a time instead of in one contiguous memory block. Using PrepareBank(), alone or in combination with PrepareEvent(), 
		/// will prevent in-memory duplication of media at the cost of some memory fragmentation.
		/// Calling this function specifying the flag AkBankContent_StructureOnly will load only the structural part (including events) of this bank, 
		/// allowing using PrepareEvent() to load media on demand.  
		/// \sa 
		/// - \ref soundengine_banks_preparingbanks
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::PreparationType</tt>
		/// \remarks
		/// \c PrepareBank(), when called with the flag \c AkBankContent_StructureOnly, requires additional calls to \c PrepareEvent() to load the media for each event. \c PrepareEvent(), however, is unable to 
		///		access media content contained within SoundBanks and requires that the media be available as loose files in the file system. This flag may be useful to implement multiple loading configurations;
		///		for example, a game may have a tool mode that uses PrepareEvent() to load loose files on-demand and, also, a game mode that uses \c LoadBank() to load the bank in entirety.
		AK_EXTERNAPIFUNC( AKRESULT, PrepareBank )(
			AK::SoundEngine::PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			const char*           in_pszString,								///< Name of the bank to Prepare/Unprepare.
			AK::SoundEngine::AkBankContent	in_uFlags = AkBankContent_All	///< Structures only (including events) or all content.
			);

		/// \n\aknote Requires that the "Use SoundBank names" option be unchecked in the Wwise Project Settings. \endaknote
		/// This function will load the events, structural content, and optionally, the media content from the SoundBank. If the flag AkBankContent_All is specified, PrepareBank() will load the media content from 
		/// the bank, but as opposed to LoadBank(), the media will be loaded one media item at a time instead of in one contiguous memory block. Using PrepareBank(), alone or in combination with PrepareEvent(), 
		/// will prevent in-memory duplication of media at the cost of some memory fragmentation.
		/// Calling this function specifying the flag AkBankContent_StructureOnly will load only the structural part (including events) of this bank, 
		/// allowing using PrepareEvent() to load media on demand.  
		/// \sa 
		/// - \ref soundengine_banks_preparingbanks
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::PreparationType</tt>
		/// \remarks
		/// \c PrepareBank(), when called with the flag AkBankContent_StructureOnly, requires additional calls to PrepareEvent() to load the media for each event. PrepareEvent(), however, is unable to 
		///		access media content contained within SoundBanks and requires that the media be available as loose files in the file system. This flag may be useful to implement multiple loading configurations;
		///		for example, a game may have a tool mode that uses PrepareEvent() to load loose files on-demand and, also, a game mode that uses LoadBank() to load the bank in entirety.
		AK_EXTERNAPIFUNC( AKRESULT, PrepareBank )(
			AK::SoundEngine::PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkBankID            in_bankID,									///< ID of the bank to Prepare/Unprepare.
			AK::SoundEngine::AkBankContent	in_uFlags = AkBankContent_All	///< Structures only (including events) or all content.
			);

#ifdef AK_SUPPORT_WCHAR
		/// This function will load the Events, structural content, and optionally, the media content from the SoundBank. If the flag AkBankContent_All is specified, PrepareBank() will load the media content from 
		/// the bank, but as opposed to LoadBank(), the media will be loaded one media item at a time instead of in one contiguous memory block. Using PrepareBank(), alone or in combination with PrepareEvent(), 
		/// will prevent in-memory duplication of media at the cost of some memory fragmentation.
		/// Calling this function specifying the flag AkBankContent_StructureOnly will load only the structural part (including events) of this bank, 
		/// allowing using PrepareEvent() to load media on demand.  
		/// \sa 
		/// - \ref soundengine_banks_preparingbanks
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::PreparationType</tt>
		/// \remarks
		/// PrepareBank(), when called with the flag AkBankContent_StructureOnly, requires additional calls to PrepareEvent() to load the media for each event. PrepareEvent(), however, is unable to 
		///		access media content contained within SoundBanks and requires that the media be available as loose files in the file system. This flag may be useful to implement multiple loading configurations;
		///		for example, a game may have a tool mode that uses PrepareEvent() to load loose files on-demand and, also, a game mode that uses LoadBank() to load the bank in entirety.
		AK_EXTERNAPIFUNC( AKRESULT, PrepareBank )(
			AK::SoundEngine::PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			const wchar_t*      in_pszString,								///< Name of the bank to Prepare/Unprepare.
			AkBankCallbackFunc	in_pfnBankCallback,							///< Callback function
			void *              in_pCookie,									///< Callback cookie (reserved to user, passed to the callback function)
			AK::SoundEngine::AkBankContent	in_uFlags = AkBankContent_All	///< Structures only (including events) or all content.
			);
#endif //AK_SUPPORT_WCHAR

		/// This function will load the events, structural content, and optionally, the media content from the SoundBank. If the flag \c AkBankContent_All is specified, \c PrepareBank() will load the media content from 
		/// the bank, but as opposed to \c LoadBank(), the media will be loaded one media item at a time instead of in one contiguous memory block. Using \c PrepareBank(), alone or in combination with PrepareEvent(), 
		/// will prevent in-memory duplication of media at the cost of some memory fragmentation.
		/// Calling this function specifying the flag AkBankContent_StructureOnly will load only the structural part (including events) of this bank, 
		/// allowing using PrepareEvent() to load media on demand.  
		/// \sa 
		/// - \ref soundengine_banks_preparingbanks
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::PreparationType()</tt>
		/// \remarks
		/// PrepareBank(), when called with the flag AkBankContent_StructureOnly, requires additional calls to PrepareEvent() to load the media for each event. PrepareEvent(), however, is unable to 
		///		access media content contained within SoundBanks and requires that the media be available as loose files in the file system. This flag may be useful to implement multiple loading configurations;
		///		for example, a game may have a tool mode that uses PrepareEvent() to load loose files on-demand and, also, a game mode that uses LoadBank() to load the bank in entirety.
		AK_EXTERNAPIFUNC( AKRESULT, PrepareBank )(
			AK::SoundEngine::PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			const char*         in_pszString,								///< Name of the bank to Prepare/Unprepare.
			AkBankCallbackFunc	in_pfnBankCallback,							///< Callback function
			void *              in_pCookie,									///< Callback cookie (reserved to user, passed to the callback function)
			AK::SoundEngine::AkBankContent	in_uFlags = AkBankContent_All	///< Structures only (including events) or all content.
			);

		/// \n\aknote Requires that the "Use SoundBank names" option be unchecked in the Wwise Project Settings. \endaknote
		/// This function will load the events, structural content, and optionally, the media content from the SoundBank. If the flag AkBankContent_All is specified, \c PrepareBank() will load the media content from 
		/// the bank, but as opposed to \c LoadBank(), the media will be loaded one media item at a time instead of in one contiguous memory block. Using \c PrepareBank(), alone or in combination with \c PrepareEvent(), 
		/// will prevent in-memory duplication of media at the cost of some memory fragmentation.
		/// Calling this function specifying the flag AkBankContent_StructureOnly will load only the structural part (including events) of this bank, 
		/// allowing using PrepareEvent() to load media on demand.  
		/// \sa 
		/// - \ref soundengine_banks_preparingbanks
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - <tt>AK::SoundEngine::PreparationType()</tt>
		/// \remarks
		/// \c PrepareBank(), when called with the flag AkBankContent_StructureOnly, requires additional calls to PrepareEvent() to load the media for each event. \c PrepareEvent(), however, is unable to 
		///		access media content contained within SoundBanks and requires that the media be available as loose files in the file system. This flag may be useful to implement multiple loading configurations;
		///		for example, a game may have a tool mode that uses \c PrepareEvent() to load loose files on-demand and, also, a game mode that uses \c LoadBank() to load the bank in entirety.
		AK_EXTERNAPIFUNC( AKRESULT, PrepareBank )(
			AK::SoundEngine::PreparationType		in_PreparationType,				///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkBankID            in_bankID,						///< ID of the bank to Prepare/Unprepare.
			AkBankCallbackFunc	in_pfnBankCallback,				///< Callback function
			void *              in_pCookie,						///< Callback cookie (reserved to user, passed to the callback function)
			AK::SoundEngine::AkBankContent		in_uFlags = AkBankContent_All	///< Structures only (including events) or all content.
			);
		
		/// Clear all previously prepared events.\n
		/// \return
		/// - AK_Success if successful.
		///	- AK_Fail if the sound engine was not correctly initialized or if there is not enough memory to handle the command.
		/// \remarks
		/// The function \c ClearBanks() also clears all prepared events.
		/// \sa
		/// - \c <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - \c <tt>AK::SoundEngine::ClearBanks()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, ClearPreparedEvents )();

#ifdef AK_SUPPORT_WCHAR
		/// Prepares or unprepares Events synchronously (by Unicode string).\n
		/// The Events are identified by strings, and converted to IDs internally
		/// (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// Before invoking \c PrepareEvent(), use \c LoadBank() to explicitly load the SoundBank(s) 
		/// that contain the Events and structures. When a request is posted to the
		/// Bank Manager consumer thread, it will resolve all dependencies needed to 
		/// successfully post the specified Events and load the required loose media files. 
		/// \aknote Before version 2015.1, the required media files could be included
		/// in a separate media SoundBank. As described in \ref whatsnew_2015_1_migration,
		/// however,\c  PrepareEvent() now only looks for loose media files.
		/// \endaknote
		/// The function returns when the request is completely processed.
		/// \return 
		///	- AK_Success: Prepare/un-prepare successful.
		/// - AK_IDNotFound: At least one of the event/game sync identifiers passed to PrepareEvent() does not exist.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// Whenever at least one event fails to be resolved, the actions performed for all 
		/// other events are cancelled.
		/// \sa
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::ClearPreparedEvents()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareEvent )( 
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			const wchar_t**		in_ppszString,			///< Array of event names
			AkUInt32			in_uNumEvent			///< Number of events in the array
			);
#endif //AK_SUPPORT_WCHAR

		/// Prepares or unprepares events synchronously.\n
		/// The Events are identified by strings and converted to IDs internally
		/// (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// Before invoking PrepareEvent(), use LoadBank() to explicitly load the SoundBank(s) 
		/// that contain the Events and structures. When a request is posted to the
		/// Bank Manager consumer thread, it will resolve all dependencies needed to 
		/// successfully post the specified Events and load the required loose media files. 
		/// \aknote Before version 2015.1, the required media files could be included
		/// in a separate media SoundBank. As described in \ref whatsnew_2015_1_migration,
		/// however, PrepareEvent() now only looks for loose media files.
		/// \endaknote
		/// The function returns when the request is completely processed.
		/// \return 
		///	- AK_Success: Prepare/un-prepare successful.
		/// - AK_IDNotFound: At least one of the event/game sync identifiers passed to PrepareEvent() does not exist.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// Whenever at least one event fails to be resolved, the actions performed for all 
		/// other events are cancelled.
		/// \sa
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::ClearPreparedEvents()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareEvent )( 
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			const char**		in_ppszString,			///< Array of event names
			AkUInt32			in_uNumEvent			///< Number of events in the array
			);

		/// Prepares or unprepares events synchronously (by ID).
		/// The Events are identified by their ID (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// Before invoking PrepareEvent(), use LoadBank() to explicitly load the SoundBank(s) 
		/// that contain the Events and structures. When a request is posted to the
		/// Bank Manager consumer thread, it will resolve all dependencies needed to 
		/// successfully post the specified Events and load the required loose media files. 
		/// \aknote Before version 2015.1, the required media files could be included
		/// in a separate media SoundBank. As described in \ref whatsnew_2015_1_migration,
		/// however, PrepareEvent() now only looks for loose media files.
		/// \endaknote
		/// The function returns when the request is completely processed.
		/// \return 
		///	- AK_Success: Prepare/un-prepare successful.
		/// - AK_IDNotFound: At least one of the event/game sync identifiers passed to PrepareEvent() does not exist.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// Whenever at least one event fails to be resolved, the actions performed for all 
		/// other events are cancelled.
		/// \sa
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::ClearPreparedEvents()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareEvent )( 
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkUniqueID*			in_pEventID,			///< Array of event IDs
			AkUInt32			in_uNumEvent			///< Number of event IDs in the array
			);

#ifdef AK_SUPPORT_WCHAR
		/// Prepares or unprepares an event asynchronously (by Unicode string).
		/// The Events are identified by string (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// Before invoking PrepareEvent(), use LoadBank() to explicitly load the SoundBank(s) 
		/// that contain the Events and structures. When a request is posted to the
		/// Bank Manager consumer thread, it will resolve all dependencies needed to 
		/// successfully post the specified Events and load the required loose media files. 
		/// \aknote Before version 2015.1, the required media files could be included
		/// in a separate media SoundBank. As described in \ref whatsnew_2015_1_migration,
		/// however, \c PrepareEvent() now only looks for loose media files.
		/// \endaknote
		/// The function returns immediately. Use a callback to be notified when the request has finished being processed.
		/// \return AK_Success if scheduling is was successful, AK_Fail otherwise.
		/// \remarks
		/// Whenever at least one Event fails to be resolved, the actions performed for all 
		/// other Events are cancelled.
		/// \sa
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::ClearPreparedEvents()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareEvent )( 
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			const wchar_t**		in_ppszString,			///< Array of event names
			AkUInt32			in_uNumEvent,			///< Number of events in the array
			AkBankCallbackFunc	in_pfnBankCallback,		///< Callback function
			void *              in_pCookie				///< Callback cookie (reserved to user, passed to the callback function)
			);
#endif //AK_SUPPORT_WCHAR

		/// Prepares or unprepares an event asynchronously.
		/// The Events are identified by string (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// Before invoking PrepareEvent(), use LoadBank() to explicitly load the SoundBank(s) 
		/// that contain the Events and structures. When a request is posted to the
		/// Bank Manager consumer thread, it will resolve all dependencies needed to 
		/// successfully post the specified Events and load the required loose media files. 
		/// \aknote Before version 2015.1, the required media files could be included
		/// in a separate media SoundBank. As described in \ref whatsnew_2015_1_migration,
		/// however, PrepareEvent() now only looks for loose media files.
		/// \endaknote
		/// The function returns immediately. Use a callback to be notified when the request has finished being processed.
		/// \return AK_Success if scheduling is was successful, AK_Fail otherwise.
		/// \remarks
		/// Whenever at least one event fails to be resolved, the actions performed for all 
		/// other events are cancelled.
		/// \sa
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::ClearPreparedEvents()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareEvent )( 
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			const char**		in_ppszString,			///< Array of event names 
			AkUInt32			in_uNumEvent,			///< Number of events in the array
			AkBankCallbackFunc	in_pfnBankCallback,		///< Callback function
			void *              in_pCookie				///< Callback cookie (reserved to user, passed to the callback function)
			);

		/// Prepares or unprepares events asynchronously (by ID).\n
		/// The Events are identified by their ID (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// Before invoking PrepareEvent(), use LoadBank() to explicitly load the SoundBank(s) 
		/// that contain the Events and structures. When a request is posted to the
		/// Bank Manager consumer thread, it will resolve all dependencies needed to 
		/// successfully post the specified Events and load the required loose media files. 
		/// \aknote Before version 2015.1, the required media files could be included
		/// in a separate media SoundBank. As described in \ref whatsnew_2015_1_migration,
		/// however, PrepareEvent() now only looks for loose media files.
		/// \endaknote
		/// The function returns immediately. Use a callback to be notified when the request has finished being processed.
		/// \return AK_Success if scheduling is was successful, AK_Fail otherwise.
		/// \remarks
		/// Whenever at least one event fails to be resolved, the actions performed for all 
		/// other events are cancelled.
		/// \sa
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::ClearPreparedEvents()</tt>
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareEvent )( 
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkUniqueID*			in_pEventID,			///< Array of event IDs
			AkUInt32			in_uNumEvent,			///< Number of event IDs in the array
			AkBankCallbackFunc	in_pfnBankCallback,		///< Callback function
			void *              in_pCookie				///< Callback cookie (reserved to user, passed to the callback function)
			);

		/// Indicates the location of a specific Media ID in memory
		/// The sources are identified by their ID (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// \return AK_Success if operation was successful, AK_InvalidParameter if in_pSourceSettings is invalid, and AK_Fail otherwise.
		AK_EXTERNAPIFUNC( AKRESULT, SetMedia )( 
			AkSourceSettings *	in_pSourceSettings,		///< Array of Source Settings
			AkUInt32			in_uNumSourceSettings	///< Number of Source Settings in the array
			);

		/// Removes the specified source from the list of loaded media.
		/// The sources are identified by their ID (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// \return AK_Success if operation was successful, AK_InvalidParameter if in_pSourceSettings is invalid, and AK_Fail otherwise.
		AK_EXTERNAPIFUNC( AKRESULT, UnsetMedia )( 
			AkSourceSettings *	in_pSourceSettings,		///< Array of Source Settings
			AkUInt32			in_uNumSourceSettings	///< Number of Source Settings in the array
			);

#ifdef AK_SUPPORT_WCHAR
		/// Prepares or unprepares game syncs synchronously (by Unicode string).\n
		/// The group and game syncs are specified by string (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// The game syncs definitions must already exist in the sound engine by having
		/// explicitly loaded the bank(s) that contain them (with LoadBank()).
		/// A request is posted to the Bank Manager consumer thread. It will resolve all 
		/// dependencies needed to successfully set this game sync group to one of the
		/// game sync values specified, and load the required banks, if applicable. 
		/// The function returns when the request has been completely processed. 
		/// \return 
		///	- AK_Success: Prepare/un-prepare successful.
		/// - AK_IDNotFound: At least one of the event/game sync identifiers passed to PrepareGameSyncs() does not exist.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// You need to call PrepareGameSyncs() if the sound engine was initialized with AkInitSettings::bEnableGameSyncPreparation 
		/// set to true. When set to false, the sound engine automatically prepares all game syncs when preparing events,
		/// so you never need to call this function.
		/// \sa 
		/// - \c <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - \c <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - \c <tt>AK::SoundEngine::LoadBank()</tt>
		/// - \c AkInitSettings
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareGameSyncs )(
			PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkGroupType		in_eGameSyncType,			///< The type of game sync.
			const wchar_t*	in_pszGroupName,			///< The State Group Name or the Switch Group Name.
			const wchar_t**	in_ppszGameSyncName,		///< The specific ID of the state to either support or not support.
			AkUInt32		in_uNumGameSyncs			///< The number of game sync in the string array.
			);
#endif //AK_SUPPORT_WCHAR

		/// Prepares or unprepares game syncs synchronously.\n
		/// The group and game syncs are specified by string (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// The game syncs definitions must already exist in the sound engine by having
		/// explicitly loaded the bank(s) that contain them (with LoadBank()).
		/// A request is posted to the Bank Manager consumer thread. It will resolve all 
		/// dependencies needed to successfully set this game sync group to one of the
		/// game sync values specified, and load the required banks, if applicable. 
		/// The function returns when the request has been completely processed. 
		/// \return 
		///	- AK_Success: Prepare/un-prepare successful.
		/// - AK_IDNotFound: At least one of the event/game sync identifiers passed to PrepareGameSyncs() does not exist.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// You need to call PrepareGameSyncs() if the sound engine was initialized with AkInitSettings::bEnableGameSyncPreparation 
		/// set to true. When set to false, the sound engine automatically prepares all game syncs when preparing events,
		/// so you never need to call this function.
		/// \sa 
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - AkInitSettings
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareGameSyncs )(
			PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkGroupType		in_eGameSyncType,			///< The type of game sync.
			const char*		in_pszGroupName,			///< The State Group Name or the Switch Group Name.
			const char**	in_ppszGameSyncName,		///< The specific ID of the state to either support or not support.
			AkUInt32		in_uNumGameSyncs			///< The number of game sync in the string array.
			);

		/// Prepares or unprepares game syncs synchronously (by ID).\n
		/// The group and game syncs are specified by ID (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// The game syncs definitions must already exist in the sound engine by having
		/// explicitly loaded the bank(s) that contain them (with LoadBank()).
		/// A request is posted to the Bank Manager consumer thread. It will resolve all 
		/// dependencies needed to successfully set this game sync group to one of the
		/// game sync values specified, and load the required banks, if applicable. 
		/// The function returns when the request has been completely processed. 
		/// \return 
		///	- AK_Success: Prepare/un-prepare successful.
		/// - AK_IDNotFound: At least one of the event/game sync identifiers passed to PrepareGameSyncs() does not exist.
		/// - AK_InsufficientMemory: Insufficient memory to store bank data.
		/// - AK_BankReadError: I/O error.
		/// - AK_WrongBankVersion: Invalid bank version: make sure the version of Wwise that 
		/// you used to generate the SoundBanks matches that of the SDK you are currently using.
		/// - AK_InvalidFile: File specified could not be opened.
		/// - AK_InvalidParameter: Invalid parameter, invalid memory alignment.		
		/// - AK_Fail: Load or unload failed for any other reason. (Most likely small allocation failure)
		/// \remarks
		/// You need to call \c PrepareGameSyncs() if the sound engine was initialized with \c AkInitSettings::bEnableGameSyncPreparation 
		/// set to \c true. When set to \c false, the sound engine automatically prepares all game syncs when preparing Events,
		/// so you never need to call this function.
		/// \sa 
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - AkInitSettings
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareGameSyncs )(
			PreparationType	in_PreparationType,			///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkGroupType		in_eGameSyncType,			///< The type of game sync.
			AkUInt32		in_GroupID,					///< The State Group ID or the Switch Group ID.
			AkUInt32*		in_paGameSyncID,			///< Array of ID of the game syncs to either support or not support.
			AkUInt32		in_uNumGameSyncs			///< The number of game sync ID in the array.
			);

#ifdef AK_SUPPORT_WCHAR
		/// Prepares or unprepares game syncs asynchronously (by Unicode string).\n
		/// The group and game syncs are specified by string (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// The game syncs definitions must already exist in the sound engine by having
		/// explicitly loaded the bank(s) that contain them (with <tt>LoadBank()</tt>).
		/// A request is posted to the Bank Manager consumer thread. It will resolve all 
		/// dependencies needed to successfully set this game sync group to one of the
		/// game sync values specified, and load the required banks, if applicable. 
		/// The function returns immediately. Use a callback to be notified when the request has finished being processed.
		/// \return AK_Success if scheduling is was successful, AK_Fail otherwise.
		/// \remarks
		/// You need to call \c PrepareGameSyncs() if the sound engine was initialized with \c AkInitSettings::bEnableGameSyncPreparation 
		/// set to \c true. When set to \c false, the sound engine automatically prepares all game syncs when preparing Events,
		/// so you never need to call this function.
		/// \sa 
		/// - \c <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - \c <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - \c <tt>AK::SoundEngine::LoadBank()</tt>
		/// - \c AkInitSettings
		/// - \c AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareGameSyncs )(
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkGroupType			in_eGameSyncType,		///< The type of game sync.
			const wchar_t*		in_pszGroupName,		///< The State Group Name or the Switch Group Name.
			const wchar_t**		in_ppszGameSyncName,	///< The specific ID of the state to either support or not support.
			AkUInt32			in_uNumGameSyncs,		///< The number of game sync in the string array.
			AkBankCallbackFunc	in_pfnBankCallback,		///< Callback function
			void *				in_pCookie				///< Callback cookie (reserved to user, passed to the callback function)
			);
#endif //AK_SUPPORT_WCHAR

		/// Prepares or unprepares game syncs asynchronously.\n
		/// The group and game syncs are specified by string (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// The game syncs definitions must already exist in the sound engine by having
		/// explicitly loaded the bank(s) that contain them (with LoadBank()).
		/// A request is posted to the Bank Manager consumer thread. It will resolve all 
		/// dependencies needed to successfully set this game sync group to one of the
		/// game sync values specified, and load the required banks, if applicable. 
		/// The function returns immediately. Use a callback to be notified when the request has finished being processed.
		/// \return AK_Success if scheduling is was successful, AK_Fail otherwise.
		/// \remarks
		/// You need to call PrepareGameSyncs() if the sound engine was initialized with AkInitSettings::bEnableGameSyncPreparation 
		/// set to true. When set to false, the sound engine automatically prepares all game syncs when preparing events,
		/// so you never need to call this function.
		/// \sa 
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - AkInitSettings
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareGameSyncs )(
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkGroupType			in_eGameSyncType,		///< The type of game sync.
			const char*			in_pszGroupName,		///< The State Group Name or the Switch Group Name.
			const char**		in_ppszGameSyncName,	///< The specific ID of the state to either support or not support.
			AkUInt32			in_uNumGameSyncs,		///< The number of game sync in the string array.
			AkBankCallbackFunc	in_pfnBankCallback,		///< Callback function
			void *				in_pCookie				///< Callback cookie (reserved to user, passed to the callback function)
			);

		/// Prepares or un-prepare game syncs asynchronously (by ID).\n
		/// The group and game syncs are specified by ID (refer to \ref soundengine_banks_general for a discussion on using strings and IDs).
		/// The game syncs definitions must already exist in the sound engine by having
		/// explicitly loaded the bank(s) that contain them (with LoadBank()).
		/// A request is posted to the Bank Manager consumer thread. It will resolve all 
		/// dependencies needed to successfully set this game sync group to one of the
		/// game sync values specified, and load the required banks, if applicable. 
		/// The function returns immediately. Use a callback to be notified when the request has finished being processed.
		/// \return AK_Success if scheduling is was successful, AK_Fail otherwise.
		/// \remarks
		/// You need to call PrepareGameSyncs() if the sound engine was initialized with AkInitSettings::bEnableGameSyncPreparation 
		/// set to true. When set to false, the sound engine automatically prepares all Game Syncs when preparing Events,
		/// so you never need to call this function.
		/// \sa 
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::PrepareEvent()</tt>
		/// - <tt>AK::SoundEngine::LoadBank()</tt>
		/// - AkInitSettings
		/// - AkBankCallbackFunc
		/// - \ref soundengine_banks
		/// - \ref sdk_bank_training
		AK_EXTERNAPIFUNC( AKRESULT, PrepareGameSyncs )(
			PreparationType		in_PreparationType,		///< Preparation type ( Preparation_Load or Preparation_Unload )
			AkGroupType			in_eGameSyncType,		///< The type of game sync.
			AkUInt32			in_GroupID,				///< The State Group ID or the Switch Group ID.
			AkUInt32*			in_paGameSyncID,		///< Array of ID of the Game Syncs to either support or not support.
			AkUInt32			in_uNumGameSyncs,		///< The number of game sync ID in the array.
			AkBankCallbackFunc	in_pfnBankCallback,		///< Callback function
			void *				in_pCookie				///< Callback cookie (reserved to user, passed to the callback function)
			);

	    //@}


		////////////////////////////////////////////////////////////////////////
		/// @name Listeners
		//@{

		/// Sets a game object's associated listeners.
		/// All listeners that have previously been added via AddListener or set via SetListeners will be removed and replaced with the listeners in the array in_pListenerGameObjs.
		/// Calling this function will override the default set of listeners and in_emitterGameObj will now reference its own, unique set of listeners.
		/// \return Always returns AK_Success
		/// \sa 
		/// - <tt>AK::SoundEngine::AddListener</tt>
		/// - <tt>AK::SoundEngine::RemoveListener</tt>
		/// - <tt>AK::SoundEngine::SetDefaultListeners</tt>
		/// - \ref soundengine_listeners
		AK_EXTERNAPIFUNC( AKRESULT, SetListeners )(
			AkGameObjectID in_emitterGameObj,			///< Emitter game object.  Must have been previously registered via RegisterGameObj.
			const AkGameObjectID* in_pListenerGameObjs,	///< Array of listener game object IDs that will be activated for in_emitterGameObj. 
			AkUInt32 in_uNumListeners					///< Length of array
			);

		/// Add a single listener to a game object's set of associated listeners.
		/// Any listeners that have previously been added or set via AddListener or SetListeners will remain as listeners and in_listenerGameObj will be added as an additional listener.
		/// Calling this function will override the default set of listeners and in_emitterGameObj will now reference its own, unique set of listeners.
		/// \sa 
		/// - <tt>AK::SoundEngine::SetListeners</tt>
		/// - <tt>AK::SoundEngine::RemoveListener</tt>
		/// - <tt>AK::SoundEngine::SetDefaultListeners</tt>
		/// - \ref soundengine_listeners
		AK_EXTERNAPIFUNC(AKRESULT, AddListener)(
			AkGameObjectID in_emitterGameObj,			///< Emitter game object.  Must have been previously registered via RegisterGameObj.
			AkGameObjectID in_listenerGameObj			///< Listener game object IDs that will be activated for in_emitterGameObj. 
			);

		/// Remove a single listener from a game object's set of active listeners.
		/// Calling this function will override the default set of listeners and in_emitterGameObj will now reference its own, unique set of listeners.
		/// \sa 
		/// - <tt>AK::SoundEngine::SetListeners</tt>
		/// - <tt>AK::SoundEngine::AddListener</tt>
		/// - <tt>AK::SoundEngine::SetDefaultListeners</tt>
		/// - \ref soundengine_listeners
		AK_EXTERNAPIFUNC(AKRESULT, RemoveListener)(
			AkGameObjectID in_emitterGameObj,			///< Emitter game object.
			AkGameObjectID in_listenerGameObj			///< Listener game object IDs that will be deactivated for in_emitterGameObj. Game objects must have been previously registered.
			);

		/// Sets the default set of associated listeners for game objects that have not explicitly overridden their listener sets.  Upon registration, all game objects reference the default listener set, until
		/// a call to AddListener, RemoveListener, SetListeners or SetGameObjectOutputBusVolume is made on that game object.
		/// All default listeners that have previously been added via AddDefaultListener or set via SetDefaultListeners will be removed and replaced with the listeners in the array in_pListenerGameObjs.
		/// \return Always returns AK_Success
		/// \sa 
		/// - \ref soundengine_listeners
		AK_EXTERNAPIFUNC(AKRESULT, SetDefaultListeners)(
			const AkGameObjectID* in_pListenerObjs,	///< Array of listener game object IDs that will be activated for subsequent registrations. Game objects must have been previously registered.
			AkUInt32 in_uNumListeners				///< Length of array
			);

		/// Add a single listener to the default set of listeners.  Upon registration, all game objects reference the default listener set, until
		/// a call to AddListener, RemoveListener, SetListeners or SetGameObjectOutputBusVolume is made on that game object.
		/// \sa 
		/// - <tt>AK::SoundEngine::SetDefaultListeners</tt>
		/// - <tt>AK::SoundEngine::RemoveDefaultListener</tt>
		/// - \ref soundengine_listeners
		AK_EXTERNAPIFUNC(AKRESULT, AddDefaultListener)(
			AkGameObjectID in_listenerGameObj			///< Listener game object IDs that will be added to the default set of listeners.
			);

		/// Remove a single listener from the default set of listeners.  Upon registration, all game objects reference the default listener set, until
		/// a call to AddListener, RemoveListener, SetListeners or SetGameObjectOutputBusVolume is made on that game object.
		/// \sa 
		/// - <tt>AK::SoundEngine::SetDefaultListeners</tt>
		/// - <tt>AK::SoundEngine::AddDefaultListener</tt>
		/// - \ref soundengine_listeners
		AK_EXTERNAPIFUNC(AKRESULT, RemoveDefaultListener)(
			AkGameObjectID in_listenerGameObj			///< Listener game object IDs that will be removed from the default set of listeners.
			);

		/// Resets the listener associations to the default listener(s), as set by <tt>SetDefaultListeners</tt>. This will also reset per-listener gains that have been set using <tt>SetGameObjectOutputBusVolume</tt>.
		/// \return Always returns AK_Success
		/// \sa 
		/// - <tt>AK::SoundEngine::SetListeners</tt>
		/// - <tt>AK::SoundEngine::SetDefaultListeners</tt>
		/// - <tt>AK::SoundEngine::SetGameObjectOutputBusVolume</tt>
		/// - \ref soundengine_listeners
		AK_EXTERNAPIFUNC(AKRESULT, ResetListenersToDefault)(
			AkGameObjectID in_emitterGameObj	///< Emitter game object.
			);

		/// Sets a listener's spatialization parameters. This let you define listener-specific 
		/// volume offsets for each audio channel.
		/// If \c in_bSpatialized is false, only \c in_pVolumeOffsets is used for this listener (3D positions 
		/// have no effect on the speaker distribution). Otherwise, \c in_pVolumeOffsets is added to the speaker
		/// distribution computed for this listener.
		/// Use helper functions of \c AK::SpeakerVolumes to manipulate the vector of volume offsets in_pVolumeOffsets.
		/// 
		/// If a sound is mixed into a bus that has a different speaker configuration than in_channelConfig,
		/// standard up/downmix rules apply.
		/// \return \c AK_Success if message was successfully posted to sound engine queue, \c AK_Fail otherwise.
		/// \sa 
		/// - \ref soundengine_listeners_spatial
		AK_EXTERNAPIFUNC( AKRESULT, SetListenerSpatialization )(
			AkGameObjectID in_uListenerID,				///< Listener game object ID
			bool in_bSpatialized,						///< Spatialization toggle (True : enable spatialization, False : disable spatialization)
			AkChannelConfig in_channelConfig,			///< Channel configuration associated with volumes in_pVolumeOffsets. Ignored if in_pVolumeOffsets is NULL.
			AK::SpeakerVolumes::VectorPtr in_pVolumeOffsets = NULL	///< Per-speaker volume offset, in dB. See AkSpeakerVolumes.h for how to manipulate this vector.
			);

		//@}


		////////////////////////////////////////////////////////////////////////
		/// @name Game Syncs
		//@{

		/// Sets the value of a real-time parameter control (by ID).
		/// With this function, you may set a game parameter value with global scope or with game object scope. 
		/// Game object scope supersedes global scope. (Once a value is set for the game object scope, it will not be affected by changes to the global scope value.) Game parameter values set with a global scope are applied to all 
		/// game objects that not yet registered, or already registered but not overridden with a value with game object scope.
		/// To set a game parameter value with global scope, pass \c AK_INVALID_GAME_OBJECT as the game object. 
		/// With this function, you may also change the value of a game parameter over time. To do so, specify a non-zero 
		/// value for \c in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call \c <tt><tt>SetRTPCValue()</tt></tt> with <tt>in_uValueChangeDuration = 0</tt> in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. Thus, if you call this 
		/// function at every game frame, you should not use \c in_uValueChangeDuration, as it would have no effect and it is less efficient.
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return Always \c AK_Success
		/// \sa 
		/// - \ref soundengine_rtpc
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
        AK_EXTERNAPIFUNC( AKRESULT, SetRTPCValue )( 
			AkRtpcID in_rtpcID, 									///< ID of the game parameter
			AkRtpcValue in_value, 									///< Value to set
			AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,///< Associated game object ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards in_value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when for example loading a level and you dont want the values to interpolate.
		    );

#ifdef AK_SUPPORT_WCHAR
		/// Sets the value of a real-time parameter control (by Unicode string name).
		/// With this function, you may set a game parameter value to global scope or to game object scope. 
		/// Game object scope supersedes global scope. (Once a value is set for the game object scope, it will not be affected by changes to the global scope value.) Game parameter values set with global scope are applied to all 
		/// game objects that not yet registered, or already registered but not overridden with a value with game object scope.
		/// To set a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
		/// With this function, you may also change the value of a game parameter over time. To do so, specify a non-zero 
		/// value for in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValue() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. Thus, if you call this 
		/// function at every game frame, you should not use in_uValueChangeDuration, as it would have no effect and it is less efficient.
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if in_pszRtpcName is NULL.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_rtpc
        AK_EXTERNAPIFUNC( AKRESULT, SetRTPCValue )( 
			const wchar_t* in_pszRtpcName,							///< Name of the game parameter
			AkRtpcValue in_value, 									///< Value to set
			AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,///< Associated game object ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards in_value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when for example loading a level and you dont want the values to interpolate.
		    );
#endif //AK_SUPPORT_WCHAR

		/// Sets the value of a real-time parameter control.
		/// With this function, you may set a game parameter value with global scope or with game object scope. 
		/// Game object scope supersedes global scope. (Once a value is set for the game object scope, it will not be affected by changes to the global scope value.) Game parameter values set with global scope are applied to all 
		/// game objects that not yet registered, or already registered but not overridden with a value with game object scope.
		/// To set a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
		/// With this function, you may also change the value of a game parameter over time. To do so, specify a non-zero 
		/// value for \c in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValue() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. Thus, if you call this 
		/// function at every game frame, you should not use in_uValueChangeDuration, as it would have no effect and it is less efficient.
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if in_pszRtpcName is NULL.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_rtpc
        AK_EXTERNAPIFUNC( AKRESULT, SetRTPCValue )( 
			const char* in_pszRtpcName,								///< Name of the game parameter
			AkRtpcValue in_value, 									///< Value to set
			AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,///< Associated game object ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards in_value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when for example loading a level and you dont want the values to interpolate.
		    );

		/// Sets the value of a real-time parameter control (by ID).
		/// With this function, you may set a game parameter value on playing id scope. 
		/// Playing id scope supersedes both game object scope and global scope. 
		/// With this function, you may also change the value of a game parameter over time. To do so, specify a non-zero 
		/// value for in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValueByPlayingID() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. Thus, if you call this 
		/// function at every game frame, you should not use in_uValueChangeDuration, as it would have no effect and it is less efficient.
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return Always AK_Success
		/// \sa 
		/// - \ref soundengine_rtpc
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SetRTPCValueByPlayingID )( 
			AkRtpcID in_rtpcID, 									///< ID of the game parameter
			AkRtpcValue in_value, 									///< Value to set
			AkPlayingID in_playingID,								///< Associated playing ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards in_value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when, for example, loading a level and you don't want the values to interpolate.
			);

#ifdef AK_SUPPORT_WCHAR
		/// Sets the value of a real-time parameter control (by Unicode string name).
		/// With this function, you may set a game parameter value on playing ID scope. 
		/// Playing id scope supersedes both game object scope and global scope. 
		/// With this function, you may also change the value of a game parameter over time. To do so, specify a non-zero 
		/// value for in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValueByPlayingID() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. Thus, if you call this 
		/// function at every game frame, you should not use in_uValueChangeDuration, as it would have no effect and it is less efficient.
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return Always AK_Success
		/// \sa 
		/// - \ref soundengine_rtpc
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SetRTPCValueByPlayingID )( 
			const wchar_t* in_pszRtpcName,							///< Name of the game parameter
			AkRtpcValue in_value, 									///< Value to set
			AkPlayingID in_playingID,								///< Associated playing ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards in_value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when, for example, loading a level and you don't want the values to interpolate.
			);
#endif //AK_SUPPORT_WCHAR
		
		/// Sets the value of a real-time parameter control (by string name).
		/// With this function, you may set a game parameter value on playing id scope. 
		/// Playing id scope supersedes both game object scope and global scope. 
		/// With this function, you may also change the value of a game parameter over time. To do so, specify a non-zero 
		/// value for in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValueByPlayingID() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. Thus, if you call this 
		/// function at every game frame, you should not use in_uValueChangeDuration, as it would have no effect and it is less efficient.
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return Always AK_Success
		/// \sa 
		/// - \ref soundengine_rtpc
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, SetRTPCValueByPlayingID )( 
			const char* in_pszRtpcName,								///< Name of the game parameter
			AkRtpcValue in_value, 									///< Value to set
			AkPlayingID in_playingID,								///< Associated playing ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards in_value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when for example loading a level and you dont want the values to interpolate.
			);

		/// Resets the value of the game parameter to its default value, as specified in the Wwise project.
		/// With this function, you may reset a game parameter to its default value with global scope or with game object scope. 
		/// Game object scope supersedes global scope. Game parameter values reset with global scope are applied to all 
		/// game objects that were not overridden with a value with game object scope.
		/// To reset a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
		/// With this function, you may also reset the value of a game parameter over time. To do so, specify a non-zero 
		/// value for in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValue() or ResetRTPCValue() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. 
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return Always AK_Success
		/// \sa 
		/// - \ref soundengine_rtpc
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// - <tt>AK::SoundEngine::SetRTPCValue()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, ResetRTPCValue )(
			AkRtpcID in_rtpcID, 									///< ID of the game parameter
			AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,///< Associated game object ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards its default value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when for example loading a level and you dont want the values to interpolate.
			);

#ifdef AK_SUPPORT_WCHAR
		/// Resets the value of the game parameter to its default value, as specified in the Wwise project.
		/// With this function, you may reset a game parameter to its default value with global scope or with game object scope. 
		/// Game object scope supersedes global scope. Game parameter values reset with global scope are applied to all 
		/// game objects that were not overridden with a value with game object scope.
		/// To reset a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
		/// With this function, you may also reset the value of a game parameter over time. To do so, specify a non-zero 
		/// value for in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValue() or ResetRTPCValue() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. 
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if in_pszParamName is NULL.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_rtpc
		/// - <tt>AK::SoundEngine::SetRTPCValue()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, ResetRTPCValue )(
			const wchar_t* in_pszRtpcName,							///< Name of the game parameter
			AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,///< Associated game object ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards its default value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when for example loading a level and you dont want the values to interpolate.
			);
#endif //AK_SUPPORT_WCHAR

		/// Resets the value of the game parameter to its default value, as specified in the Wwise project.
		/// With this function, you may reset a game parameter to its default value with global scope or with game object scope. 
		/// Game object scope supersedes global scope. Game parameter values reset with global scope are applied to all 
		/// game objects that were not overridden with a value with game object scope.
		/// To reset a game parameter value with global scope, pass AK_INVALID_GAME_OBJECT as the game object. 
		/// With this function, you may also reset the value of a game parameter over time. To do so, specify a non-zero 
		/// value for in_uValueChangeDuration. At each audio frame, the game parameter value will be updated internally 
		/// according to the interpolation curve. If you call SetRTPCValue() or ResetRTPCValue() with in_uValueChangeDuration = 0 in the 
		/// middle of an interpolation, the interpolation stops and the new value is set directly. 
		/// Refer to \ref soundengine_rtpc_pergameobject, \ref soundengine_rtpc_buses and 
		/// \ref soundengine_rtpc_effects for more details on RTPC scope.
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if in_pszParamName is NULL.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_rtpc
		/// - <tt>AK::SoundEngine::SetRTPCValue()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, ResetRTPCValue )(
			const char* in_pszRtpcName,								///< Name of the game parameter
			AkGameObjectID in_gameObjectID = AK_INVALID_GAME_OBJECT,///< Associated game object ID
			AkTimeMs in_uValueChangeDuration = 0,					///< Duration during which the game parameter is interpolated towards its default value
			AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear,	///< Curve type to be used for the game parameter interpolation
			bool in_bBypassInternalValueInterpolation = false		///< True if you want to bypass the internal "slew rate" or "over time filtering" specified by the sound designer. This is meant to be used when for example loading a level and you dont want the values to interpolate.
			);

		/// Sets the State of a Switch Group (by IDs).
		/// \return Always returns AK_Success
		/// \sa 
		/// - \ref soundengine_switch
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
        AK_EXTERNAPIFUNC( AKRESULT, SetSwitch )( 
			AkSwitchGroupID in_switchGroup, 			///< ID of the Switch Group
			AkSwitchStateID in_switchState, 			///< ID of the Switch
			AkGameObjectID in_gameObjectID				///< Associated game object ID
		    );

#ifdef AK_SUPPORT_WCHAR
		/// Sets the State of a Switch Group (by Unicode string names).
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if the switch or Switch Group name was not resolved to an existing ID\n
		/// Make sure that the banks were generated with the "include string" option.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_switch
        AK_EXTERNAPIFUNC( AKRESULT, SetSwitch )( 
			const wchar_t* in_pszSwitchGroup,			///< Name of the Switch Group
			const wchar_t* in_pszSwitchState, 			///< Name of the Switch
			AkGameObjectID in_gameObjectID				///< Associated game object ID
			);
#endif //AK_SUPPORT_WCHAR

		/// Sets the state of a Switch Group.
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if the switch or Switch Group name was not resolved to an existing ID\n
		/// Make sure that the banks were generated with the "include string" option.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_switch
        AK_EXTERNAPIFUNC( AKRESULT, SetSwitch )( 
			const char* in_pszSwitchGroup,				///< Name of the Switch Group
			const char* in_pszSwitchState, 				///< Name of the Switch
			AkGameObjectID in_gameObjectID				///< Associated game object ID
			);

		/// Post the specified trigger (by IDs).
		/// \return Always returns AK_Success
		/// \sa 
		/// - \ref soundengine_triggers
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, PostTrigger )( 
			AkTriggerID 	in_triggerID, 				///< ID of the trigger
			AkGameObjectID 	in_gameObjectID				///< Associated game object ID
		    );

#ifdef AK_SUPPORT_WCHAR
		/// Posts the specified trigger (by Unicode string name).
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if the trigger name was not resolved to an existing ID\n
		/// Make sure that the banks were generated with the "include string" option.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_triggers
        AK_EXTERNAPIFUNC( AKRESULT, PostTrigger )( 
			const wchar_t* in_pszTrigger,				///< Name of the trigger
			AkGameObjectID in_gameObjectID				///< Associated game object ID
			);
#endif //AK_SUPPORT_WCHAR

		/// Posts the specified trigger.
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if the trigger name was not resolved to an existing ID\n
		/// Make sure that the banks were generated with the "include string" option.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_triggers
        AK_EXTERNAPIFUNC( AKRESULT, PostTrigger )( 
			const char* in_pszTrigger,			 	    ///< Name of the trigger
			AkGameObjectID in_gameObjectID				///< Associated game object ID
			);

		/// Sets the state of a State Group (by IDs).
		/// \return Always returns AK_Success
		/// \sa 
		/// - \ref soundengine_states
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
        AK_EXTERNAPIFUNC( AKRESULT, SetState )( 
			AkStateGroupID in_stateGroup, 				///< ID of the State Group
			AkStateID in_state 							///< ID of the state
		    );

#ifdef AK_SUPPORT_WCHAR
		/// Sets the state of a State Group (by Unicode string names).
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if the state or State Group name was not resolved to an existing ID\n
		/// Make sure that the banks were generated with the "include string" option.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_states
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
        AK_EXTERNAPIFUNC( AKRESULT, SetState )( 
			const wchar_t* in_pszStateGroup,				///< Name of the State Group
			const wchar_t* in_pszState 						///< Name of the state
			);
#endif //AK_SUPPORT_WCHAR

		/// Sets the state of a State Group.
		/// \return 
		/// - AK_Success if successful
		/// - AK_IDNotFound if the state or State Group name was not resolved to an existing ID\n
		/// Make sure that the banks were generated with the "include string" option.
		/// \aknote Strings are case-insensitive. \endaknote
		/// \sa 
		/// - \ref soundengine_states
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
        AK_EXTERNAPIFUNC( AKRESULT, SetState )( 
			const char* in_pszStateGroup,					///< Name of the State Group
			const char* in_pszState 						///< Name of the state
			);

		//@}

		////////////////////////////////////////////////////////////////////////
		/// @name Environments
		//@{

		/// Sets the Auxiliary Busses to route the specified game object
		/// To clear the game object's auxiliary sends, \c in_uNumSendValues must be 0.
		/// \sa 
		/// - \ref soundengine_environments
		/// - \ref soundengine_environments_dynamic_aux_bus_routing
		/// - \ref soundengine_environments_id_vs_string
		/// - <tt>AK::SoundEngine::GetIDFromString()</tt>
		/// \return 
		/// - AK_Success if successful
		///	- AK_InvalidParameter if a duplicated environment is found in the array
		AK_EXTERNAPIFUNC( AKRESULT, SetGameObjectAuxSendValues )( 
			AkGameObjectID		in_gameObjectID,		///< Associated game object ID
			AkAuxSendValue*		in_aAuxSendValues,		///< Variable-size array of AkAuxSendValue structures
														///< (it may be NULL if no environment must be set)
			AkUInt32			in_uNumSendValues		///< The number of auxiliary busses at the pointer's address
														///< (it must be 0 if no environment is set)
			);

		/// Registers a callback to allow the game to modify or override the volume to be applied at the output of an audio bus.
		/// The callback must be registered once per bus ID.
		/// Call with <tt>in_pfnCallback = NULL</tt> to unregister.
		/// \aknote The bus in_busID needs to be a mixing bus.\endaknote
		/// \aknote Beware when using this callback on the Master Audio Bus: since the output of this bus is not a bus, but is instead a system end-point, <tt>AkSpeakerVolumeMatrixCallbackInfo::pMixerContext</tt> will be \c NULL. You cannot modify panning at the output of the Master Audio Bus.\endaknote
		/// \sa 
		/// - \ref goingfurther_speakermatrixcallback
		/// - \ref soundengine_environments
		/// - AkSpeakerVolumeMatrixCallbackInfo
		/// - <tt>AK::IAkMixerInputContext</tt>
		/// - <tt>AK::IAkMixerPluginContext</tt>
		/// \return 
		/// - AK_Success if successful
		AK_EXTERNAPIFUNC( AKRESULT, RegisterBusVolumeCallback )( 
			AkUniqueID in_busID,						///< Bus ID, as obtained by GetIDFromString( bus_name ).
			AkBusCallbackFunc in_pfnCallback			///< Callback function.
			);

		/// Registers a callback to be called to allow the game to access metering data from any mixing bus. You may use this to monitor loudness at any point of the mixing hierarchy 
		/// by querying the peak, RMS, True Peak and K-weighted power (according to loudness standard ITU BS.1770). See \ref goingfurther_speakermatrixcallback for an example.
		/// The callback must be registered once per bus ID.
		/// Call with in_pfnCallback = NULL to unregister.
		/// \aknote The bus in_busID needs to be a mixing bus.\endaknote
		/// \sa 
		/// - \ref goingfurther_speakermatrixcallback
		/// - AkBusMeteringCallbackFunc
		/// - <tt>AK::IAkMetering</tt>
		/// \return 
		/// - AK_Success if successful
		AK_EXTERNAPIFUNC( AKRESULT, RegisterBusMeteringCallback )( 
			AkUniqueID in_busID,						///< Bus ID, as obtained by GetIDFromString( bus_name ).
			AkBusMeteringCallbackFunc in_pfnCallback,	///< Callback function.
			AkMeteringFlags in_eMeteringFlags			///< Metering flags.
			);

		/// Sets the output bus volume (direct) to be used for the specified game object.
		/// The control value is a number ranging from 0.0f to 1.0f.
		/// Output bus volumes are stored per listener association, so calling this function will override the default set of listeners. The game object in_emitterObjID will now reference its own set of listeners which will 
		/// be the same as the old set of listeners, but with the new associated gain.  Future changes to the default listener set will not be picked up by this game object unless ResetListenersToDefault() is called.
		/// \sa 
		/// - \ref AK::SoundEngine::ResetListenersToDefault
		/// - \ref soundengine_environments
		/// - \ref soundengine_environments_setting_dry_environment
		/// - \ref soundengine_environments_id_vs_string
		/// \return Always returns AK_Success
		AK_EXTERNAPIFUNC( AKRESULT, SetGameObjectOutputBusVolume )( 
			AkGameObjectID		in_emitterObjID,		///< Associated emitter game object ID
			AkGameObjectID		in_listenerObjID,		///< Associated listener game object ID
			AkReal32			in_fControlValue		///< A multiplier where 0 means silence and 1 means no change. 
														///< (Therefore, values between 0 and 1 will attenuate the sound, and values greater than 1 will amplify it.)
			);

		/// Sets an Effect ShareSet at the specified audio node and Effect slot index.
		/// The target node cannot be a Bus, to set effects on a bus, use SetBusEffect() instead.
		/// \aknote The option "Override Parent" in 
		/// the Effect section in Wwise must be enabled for this node, otherwise the parent's effect will 
		/// still be the one in use and the call to SetActorMixerEffect will have no impact.
		/// \endaknote
		/// \return Always returns AK_Success
		AK_EXTERNAPIFUNC( AKRESULT, SetActorMixerEffect )( 
			AkUniqueID in_audioNodeID,					///< Can be a member of the Actor-Mixer or Interactive Music Hierarchy (not a bus).
			AkUInt32 in_uFXIndex,						///< Effect slot index (0-3)
			AkUniqueID in_shareSetID					///< ShareSets ID; pass AK_INVALID_UNIQUE_ID to clear the effect slot
			);

		/// Sets an Effect ShareSet at the specified bus and Effect slot index.
		/// The Bus can either be an Audio Bus or an Auxiliary Bus.
		/// This adds a reference on the audio node to an existing ShareSet.
		/// \aknote This function has unspecified behavior when adding an Effect to a currently playing
		/// Bus which does not have any Effects, or removing the last Effect on a currently playing bus.
		/// \endaknote
		/// \aknote This function will replace existing Effects on the node. If the target node is not at 
		/// the top of the hierarchy and is in the actor-mixer hierarchy, the option "Override Parent" in 
		/// the Effect section in Wwise must be enabled for this node, otherwise the parent's Effect will 
		/// still be the one in use and the call to SetBusEffect will have no impact.
		/// \endaknote
		/// \return Always returns AK_Success
		AK_EXTERNAPIFUNC( AKRESULT, SetBusEffect )( 
			AkUniqueID in_audioNodeID,					///< Bus Short ID.
			AkUInt32 in_uFXIndex,						///< Effect slot index (0-3)
			AkUniqueID in_shareSetID					///< ShareSets ID; pass AK_INVALID_UNIQUE_ID to clear the Effect slot
			);

#ifdef AK_SUPPORT_WCHAR
		/// Sets an Effect ShareSet at the specified Bus and Effect slot index.
		/// The Bus can either be an Audio Bus or an Auxiliary Bus.
		/// This adds a reference on the audio node to an existing ShareSet.
		/// \aknote This function has unspecified behavior when adding an Effect to a currently playing
		/// bus which does not have any Effects, or removing the last Effect on a currently playing Bus.
		/// \endaknote
		/// \aknote This function will replace existing Effects on the node. If the target node is not at 
		/// the top of the hierarchy and is in the Actor-Mixer Hierarchy, the option "Override Parent" in 
		/// the Effect section in Wwise must be enabled for this node, otherwise the parent's Effect will 
		/// still be the one in use and the call to \c SetBusEffect will have no impact.
		/// \endaknote
		/// \returns AK_IDNotFound is name not resolved, returns AK_Success otherwise.
		AK_EXTERNAPIFUNC( AKRESULT, SetBusEffect )( 
			const wchar_t* in_pszBusName,				///< Bus name
			AkUInt32 in_uFXIndex,						///< Effect slot index (0-3)
			AkUniqueID in_shareSetID					///< ShareSets ID; pass AK_INVALID_UNIQUE_ID to clear the effect slot
			);
#endif //AK_SUPPORT_WCHAR

		/// Sets an Effect ShareSet at the specified Bus and Effect slot index.
		/// The Bus can either be an Audio Bus or an Auxiliary Bus.
		/// This adds a reference on the audio node to an existing ShareSet.
		/// \aknote This function has unspecified behavior when adding an Effect to a currently playing
		/// Bus which does not have any effects, or removing the last Effect on a currently playing bus.
		/// \endaknote
		/// \aknote This function will replace existing Effects on the node. If the target node is not at 
		/// the top of the hierarchy and is in the Actor-Mixer Hierarchy, the option "Override Parent" in 
		/// the Effect section in Wwise must be enabled for this node, otherwise the parent's Effect will 
		/// still be the one in use and the call to SetBusEffect will have no impact.
		/// \endaknote
		/// \returns AK_IDNotFound is name not resolved, returns AK_Success otherwise.
		AK_EXTERNAPIFUNC( AKRESULT, SetBusEffect )( 
			const char* in_pszBusName,		///< Bus name
			AkUInt32 in_uFXIndex,			///< Effect slot index (0-3)
			AkUniqueID in_shareSetID		///< ShareSets ID; pass AK_INVALID_UNIQUE_ID to clear the effect slot
			);

		/// Sets a Mixer ShareSet at the specified bus.
		/// \aknote This function has unspecified behavior when adding a mixer to a currently playing
		/// Bus which does not have any Effects or mixer, or removing the last mixer on a currently playing Bus.
		/// \endaknote
		/// \aknote This function will replace existing mixers on the node. 
		/// \endaknote
		/// \return Always returns AK_Success
		AK_EXTERNAPIFUNC( AKRESULT, SetMixer )( 
			AkUniqueID in_audioNodeID,					///< Bus Short ID.
			AkUniqueID in_shareSetID					///< ShareSets ID; pass AK_INVALID_UNIQUE_ID to remove.
			);

#ifdef AK_SUPPORT_WCHAR
		/// Sets a Mixer ShareSet at the specified bus.
		/// \aknote This function has unspecified behavior when adding a mixer to a currently playing
		/// bus which does not have any effects nor mixer, or removing the last mixer on a currently playing bus.
		/// \endaknote
		/// \aknote This function will replace existing mixers on the node. 
		/// \endaknote
		/// \returns AK_IDNotFound is name not resolved, returns AK_Success otherwise.
		AK_EXTERNAPIFUNC( AKRESULT, SetMixer )( 
			const wchar_t* in_pszBusName,				///< Bus name
			AkUniqueID in_shareSetID					///< ShareSets ID; pass AK_INVALID_UNIQUE_ID to remove.
			);
#endif //AK_SUPPORT_WCHAR

		/// Sets a Mixer ShareSet at the specified bus.
		/// \aknote This function has unspecified behavior when adding a mixer to a currently playing
		/// bus which does not have any effects nor mixer, or removing the last mixer on a currently playing bus.
		/// \endaknote
		/// \aknote This function will replace existing mixers on the node.
		/// \endaknote
		/// \returns AK_IDNotFound is name not resolved, returns AK_Success otherwise.
		AK_EXTERNAPIFUNC( AKRESULT, SetMixer )( 
			const char* in_pszBusName,		///< Bus name
			AkUniqueID in_shareSetID		///< ShareSets ID; pass AK_INVALID_UNIQUE_ID to remove.
			);

		/// Forces channel configuration for the specified bus.
		/// \aknote You cannot change the configuration of the master bus.\endaknote
		/// 
		/// \return Always returns AK_Success
		AK_EXTERNAPIFUNC(AKRESULT, SetBusConfig)(
			AkUniqueID in_audioNodeID,					///< Bus Short ID.
			AkChannelConfig in_channelConfig			///< Desired channel configuration. An invalid configuration (from default constructor) means "as parent".
			);

#ifdef AK_SUPPORT_WCHAR
		/// Forces channel configuration for the specified bus.
		/// \aknote You cannot change the configuration of the master bus.\endaknote
		/// 
		/// \returns AK_IDNotFound is name not resolved, returns AK_Success otherwise.
		AK_EXTERNAPIFUNC(AKRESULT, SetBusConfig)(
			const wchar_t* in_pszBusName,				///< Bus name
			AkChannelConfig in_channelConfig			///< Desired channel configuration. An invalid configuration (from default constructor) means "as parent".
			);
#endif //AK_SUPPORT_WCHAR

		/// Forces channel configuration for the specified bus.
		/// \aknote You cannot change the configuration of the master bus.\endaknote
		/// 
		/// \returns AK_IDNotFound is name not resolved, returns AK_Success otherwise.
		AK_EXTERNAPIFUNC(AKRESULT, SetBusConfig)(
			const char* in_pszBusName,					///< Bus name
			AkChannelConfig in_channelConfig			///< Desired channel configuration. An invalid configuration (from default constructor) means "as parent".
			);

		/// Sets a game object's obstruction and occlusion levels. If SetMultiplePositions were used, values are set for all positions.
		/// This function is used to affect how an object should be heard by a specific listener.
		/// \sa 
		/// - \ref soundengine_obsocc
		/// - \ref soundengine_environments
		/// \return Always returns AK_Success
		AK_EXTERNAPIFUNC( AKRESULT, SetObjectObstructionAndOcclusion )(  
			AkGameObjectID in_EmitterID,		///< Emitter game object ID
			AkGameObjectID in_ListenerID,		///< Listener game object ID
			AkReal32 in_fObstructionLevel,		///< ObstructionLevel: [0.0f..1.0f]
			AkReal32 in_fOcclusionLevel			///< OcclusionLevel: [0.0f..1.0f]
			);

		/// Sets a game object's obstruction and occlusion level for each positions defined by SetMultiplePositions.
		/// This function differs from SetObjectObstructionAndOcclusion as a list of obstruction/occlusion pair is provided
		/// and each obstruction/occlusion pair will affect the corresponding position defined at the same index.
		/// \aknote In the case the number of obstruction/occlusion pairs is smaller than the number of positions, remaining positions' 
		/// obstrucion/occlusion values are set to 0.0. \endaknote
		/// \sa
		/// - \ref soundengine_obsocc
		/// - \ref soundengine_environments
		/// \return AK_Success if occlusion/obstruction values are successfully stored for this emitter
		AK_EXTERNAPIFUNC( AKRESULT, SetMultipleObstructionAndOcclusion )(
			AkGameObjectID in_EmitterID,										///< Emitter game object ID
			AkGameObjectID in_uListenerID,										///< Listener game object ID
			AkObstructionOcclusionValues* in_fObstructionOcclusionValues,		///< Array of obstruction/occlusion pairs to apply
																				///< ObstructionLevel: [0.0f..1.0f]
																				///< OcclusionLevel: [0.0f..1.0f]
			AkUInt32 in_uNumOcclusionObstruction								///< Number of obstruction/occlusion pairs specified in the provided array
			);

		/// Saves the playback history of container structures.
		/// This function will write history data for all currently loaded containers and instantiated game
		/// objects (for example, current position in Sequence Containers and previously played elements in
		/// Random Containers). 
		/// \remarks
		/// This function acquires the main audio lock, and may block the caller for several milliseconds.
		/// \sa 
		/// - <tt>AK::SoundEngine::SetContainerHistory()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, GetContainerHistory)(
			AK::IWriteBytes * in_pBytes			///< Pointer to IWriteBytes interface used to save the history.
			);

		/// Restores the playback history of container structures.
		/// This function will read history data from the passed-in stream reader interface, and apply it to all
		/// currently loaded containers and instantiated game objects. Game objects are matched by
		/// ID. History for unloaded structures and unknown game objects will be skipped.
		/// \remarks
		/// This function acquires the main audio lock, and may block the caller for several milliseconds.
		/// \sa 
		/// - <tt>AK::SoundEngine::GetContainerHistory()</tt>
		AK_EXTERNAPIFUNC(AKRESULT, SetContainerHistory)(
			AK::IReadBytes * in_pBytes 			///< Pointer to IReadBytes interface used to load the history.
			);

		//@}
        
        ////////////////////////////////////////////////////////////////////////
		/// @name Capture
		//@{

		/// Starts recording the sound engine audio output. 
		/// StartOutputCapture outputs a wav file per current output device of the sound engine.
		/// If more than one device is active, the system will create multiple files in the same output 
		/// directory and will append numbers at the end of the provided filename.
		///
		/// If no device is running yet, the system will return success AK_Success despite doing nothing.
		/// Use \ref RegisterAudioDeviceStatusCallback to get notified when devices are created/destructed.
		///
		/// \return AK_Success if successful, AK_Fail if there was a problem starting the output capture.
		/// \remark
		///		- The sound engine opens a stream for writing using <tt>AK::IAkStreamMgr::CreateStd()</tt>. If you are using the
		///			default implementation of the Stream Manager, file opening is executed in your implementation of 
		///			the Low-Level IO interface <tt>AK::StreamMgr::IAkFileLocationResolver::Open()</tt>. The following 
		///			AkFileSystemFlags are passed: uCompanyID = AKCOMPANYID_AUDIOKINETIC and uCodecID = AKCODECID_PCM,
		///			and the AkOpenMode is AK_OpenModeWriteOvrwr. Refer to \ref streamingmanager_lowlevel_location for
		///			more details on managing the deployment of your Wwise generated data.
		/// \sa 
		/// - <tt>AK::SoundEngine::StopOutputCapture()</tt>
		/// - <tt>AK::StreamMgr::SetFileLocationResolver()</tt>
		/// - \ref streamingdevicemanager
		/// - \ref streamingmanager_lowlevel_location
		AK_EXTERNAPIFUNC( AKRESULT, StartOutputCapture )( 
			const AkOSChar* in_CaptureFileName				///< Name of the output capture file
			);

		/// Stops recording the sound engine audio output. 
		/// \return AK_Success if successful, AK_Fail if there was a problem stopping the output capture.
		/// \sa 
		/// - <tt>AK::SoundEngine::StartOutputCapture()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, StopOutputCapture )();

		/// Adds text marker in audio output file. 
		/// \return AK_Success if successful, AK_Fail if there was a problem adding the output marker.
		/// \sa 
		/// - <tt>AK::SoundEngine::StartOutputCapture()</tt>
		AK_EXTERNAPIFUNC( AKRESULT, AddOutputCaptureMarker )(
			const char* in_MarkerText					///< Text of the marker
			);
			
		/// Starts recording the sound engine profiling information into a file. This file can be read
		/// by Wwise Authoring.
		/// \remark This function is provided as a utility tool only. It does nothing if it is 
		///			called in the release configuration and returns AK_NotCompatible.
		AK_EXTERNAPIFUNC( AKRESULT, StartProfilerCapture )( 
			const AkOSChar* in_CaptureFileName				///< Name of the output profiler file (.prof extension recommended)
			);

		/// Stops recording the sound engine profiling information. 
		/// \remark This function is provided as a utility tool only. It does nothing if it is 
		///			called in the release configuration and returns AK_NotCompatible.
		AK_EXTERNAPIFUNC( AKRESULT, StopProfilerCapture )();

		//@}

		////////////////////////////////////////////////////////////////////////
		/// @name Secondary Outputs
		//@{

		/// Adds an output to the sound engine. Use this to add controller-attached headphones, controller speakers, DVR output, etc.  
		/// The in_Settings parameter contains an Audio Device shareset to specify the output plugin to use and a device ID to specify the instance, if necessary (e.g. which game controller).
		/// \sa AkOutputSettings for more details.
		/// \sa \ref integrating_secondary_outputs
		/// \sa \ref default_audio_devices
		/// \return 
		/// - AK_NotImplemented: Feature not supported
		/// - AK_InvalidParameter: Out of range parameters or unsupported parameter combinations (see parameter list below).
		/// - AK_IDNotFound: Shareset ID doesn't exist.  Possibly, the Init bank isn't loaded yet or was not updated with latest changes.
		/// - AK_DeviceNotReady: The in_iDeviceID doesn't match with a valid hardware device.  Either the device doesn't exist or is disabled.  Disconnected devices (headphones) are not considered "not ready" therefore won't cause this error.
		/// - AK_Success: Parameters are valid.		
		AK_EXTERNAPIFUNC(AKRESULT, AddOutput)(			
			const AkOutputSettings & in_Settings,	///< Creation parameters for this output.  \ref AkOutputSettings							
			AkOutputDeviceID *out_pDeviceID = NULL,	///< (Optional) Output ID to use with all other Output management functions.  Leave to NULL if not required. \ref AK::SoundEngine::GetOutputID
			const AkGameObjectID* in_pListenerIDs = NULL,	///< Specific listener(s) to attach to this device. 
															///< If specified, only the sounds routed to game objects linked to those listeners will play in this device.  
															///< It is necessary to have separate listeners if multiple devices of the same type can coexist (e.g. controller speakers)
															///< If not specified, sound routing simply obey the associations between Master Busses and Audio Devices setup in the Wwise Project.
			AkUInt32 in_uNumListeners = 0					///< The number of elements in the in_pListenerIDs array.
			);

		/// Removes one output added through AK::SoundEngine::AddOutput
		/// If a listener was associated with the device, you should consider unregistering the listener prior to call RemoveOutput 
		/// so that Game Object/Listener routing is properly updated according to your game scenario.
		/// \sa \ref integrating_secondary_outputs
		/// \sa AK::SoundEngine::AddOutput
		/// \return AK_Success: Parameters are valid.
		AK_EXTERNAPIFUNC(AKRESULT, RemoveOutput)(
			AkOutputDeviceID in_idOutput	///< ID of the output to remove.  Use the returned ID from AddOutput or GetOutputID
			);

		/// Gets the compounded output ID from shareset and device id.
		/// Outputs are defined by their type (Audio Device shareset) and their specific system ID.  A system ID could be reused for other device types on some OS or platforms, hence the compounded ID.
		/// \return The id of the output					
		AK_EXTERNAPIFUNC(AkOutputDeviceID, GetOutputID)(
			AkUniqueID in_idShareset,		///< Audio Device ShareSet ID, as defined in the Wwise Project.  If needed, use AK::SoundEngine::GetIDFromString() to convert from a string.  Set to AK_INVALID_UNIQUE_ID to use the default.
			AkUInt32 in_idDevice			///< Device specific identifier, when multiple devices of the same type are possible.  If only one device is possible, leave to 0.
											///< - PS4 Controller-Speakers: UserID as returned from sceUserServiceGetLoginUserIdList
											///< - XBoxOne Controller-Headphones: Use the AK::GetDeviceID function to get the ID from an IMMDevice. Find the player's device with the WASAPI API (IMMDeviceEnumerator, see Microsoft documentation) or use AK::GetDeviceIDFromName.
											///< - Windows: Use AK::GetDeviceID or AK::GetDeviceIDFromName to get the correct ID.		
											///< - All others output: use 0 to select the default device for that type.
			);

		AK_EXTERNAPIFUNC(AkOutputDeviceID, GetOutputID)(
			const char* in_szShareSet,			///< Audio Device ShareSet Name, as defined in the Wwise Project.  If Null, will select the Default Output shareset (always available)
			AkUInt32 in_idDevice				///< Device specific identifier, when multiple devices of the same type are possible.  If only one device is possible, leave to 0.
			///< - PS4 Controller-Speakers: UserID as returned from sceUserServiceGetLoginUserIdList
			///< - XBoxOne Controller-Headphones: Use the AK::GetDeviceID function to get the ID from an IMMDevice. Find the player's device with the WASAPI API (IMMDeviceEnumerator, see Microsoft documentation) or use AK::GetDeviceIDFromName.
			///< - Windows: Use AK::GetDeviceID or AK::GetDeviceIDFromName to get the correct ID.		
			///< - All others output: use 0 to select the default device for that type.
			);

		#ifdef AK_SUPPORT_WCHAR
		AK_EXTERNAPIFUNC(AkOutputDeviceID, GetOutputID)(
			const wchar_t* in_szShareSet,			///< Audio Device ShareSet Name, as defined in the Wwise Project.  If Null, will select the Default Output shareset (always available)
			AkUInt32 in_idDevice			///< Device specific identifier, when multiple devices of the same type are possible.  If only one device is possible, leave to 0.
			///< - PS4 Controller-Speakers: UserID as returned from sceUserServiceGetLoginUserIdList
			///< - XBoxOne Controller-Headphones: Use the AK::GetDeviceID function to get the ID from an IMMDevice. Find the player's device with the WASAPI API (IMMDeviceEnumerator, see Microsoft documentation) or use AK::GetDeviceIDFromName.
			///< - Windows: Use AK::GetDeviceID or AK::GetDeviceIDFromName to get the correct ID.		
			///< - All others output: use 0 to select the default device for that type.
			);
		#endif

		/// Sets the Audio Device to which a master bus outputs.  This overrides the setting in the Wwise project.	
		/// Can only be set on top-level busses. The Init bank should be successfully loaded prior to this call.
		/// \return 
		/// AK_IDNotFound when either the Bus ID or the Device ID are not present in the Init bank or the bank was not loaded
		/// AK_InvalidParameter when the specified bus is not a Master Bus.  This function can be called only on busses that have no parent bus.
		AK_EXTERNAPIFUNC(AKRESULT, SetBusDevice)(
			AkUniqueID in_idBus,			///< Id of the master bus
			AkUniqueID in_idNewDevice		///< New device shareset to replace with.
			);

		/// Sets the Audio Device to which a master bus outputs.  This overrides the setting in the Wwise project.	
		/// Can only be set on top-level busses. The Init bank should be successfully loaded prior to this call.
		/// \return 
		/// AK_IDNotFound when either the Bus ID or the Device ID are not present in the Init bank or the bank was not loaded
		/// AK_InvalidParameter when the specified bus is not a Master Bus.  This function can be called only on busses that have no parent bus.
		AK_EXTERNAPIFUNC(AKRESULT, SetBusDevice)(
			const char*  in_BusName,		///< Name of the master bus
			const char*  in_DeviceName		///< New device shareset to replace with.
			);

		#ifdef AK_SUPPORT_WCHAR
		/// Sets the Audio Device to which a master bus outputs.  This overrides the setting in the Wwise project.	
		/// Can only be set on top-level busses. The Init bank should be successfully loaded prior to this call.
		/// \return 
		/// AK_IDNotFound when either the Bus ID or the Device ID are not present in the Init bank or the bank was not loaded
		/// AK_InvalidParameter when the specified bus is not a Master Bus.  This function can be called only on busses that have no parent bus.
		AK_EXTERNAPIFUNC(AKRESULT, SetBusDevice)(
			const wchar_t*  in_BusName,		///< Name of the master bus
			const wchar_t*  in_DeviceName	///< New device shareset to replace with.
			);
		#endif
		
		/// Sets the volume of a output device.		
		AK_EXTERNAPIFUNC( AKRESULT, SetOutputVolume )(
			AkOutputDeviceID in_idOutput,	///< Output ID to set the volume on.  As returned from AddOutput or GetOutputID
			AkReal32 in_fVolume				///< Volume (0.0 = Muted, 1.0 = Volume max)
			);

		//@}

		/// This function should be called to put the sound engine in background mode, where audio isn't processed anymore. This needs to be called if the console has a background mode or some suspended state.
		/// Call \c WakeupFromSuspend when your application receives the message from the OS that the process is back in foreground.
		/// When suspended, the sound engine will process API messages (like PostEvent and SetSwitch) only when \ref RenderAudio() is called. 
		/// It is recommended to match the <b>in_bRenderAnyway</b> parameter with the behavior of the rest of your game: 
		/// if your game still runs in background and you must keep some kind of coherent state between the audio engine and game, then allow rendering.
		/// If you want to minimize CPU when in background, then don't allow rendering and never call RenderAudio from the game.
		///
		/// - Android: Call for APP_CMD_PAUSE
		/// - iOS: Don't call. All audio interrupts are handled internally.
		/// - XBoxOne: Use when entering constrained mode or suspended mode (see ResourceAvailability in Xbox One documentation).
		/// \sa \ref WakeupFromSuspend
		AK_EXTERNAPIFUNC( AKRESULT, Suspend )(
			bool in_bRenderAnyway = false /// If set to true, audio processing will still occur, but not outputted. When set to false, no audio will be processed at all, even upon reception of RenderAudio().
			);

		/// This function should be called to wake up the sound engine and start processing audio again. This needs to be called if the console has a background mode or some suspended state.
		/// - Android: Call for APP_CMD_RESUME
		/// - iOS: Don't call. All audio interrupts are handled internally.		
		/// - XBoxOne: Use when the game is back to Full resources (see ResourceAvailability in Xbox One documentation).
		AK_EXTERNAPIFUNC( AKRESULT, WakeupFromSuspend )(); 

		/// Obtains the current audio output buffer tick. This corresponds to the number of buffers produced by
		/// the sound engine since initialization. 
		/// \return Tick count.
		AK_EXTERNAPIFUNC(AkUInt32, GetBufferTick)();
	}
}

#endif // _AK_SOUNDENGINE_H_
