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
/// Wwise source control plug-in utilities interface, used to create custom dialogs, display the progress dialog, and get
/// the registry path needed to save the plug-in configuration.

#ifndef _AK_WWISE_ISOURCECONTROLUTILITIES_H
#define _AK_WWISE_ISOURCECONTROLUTILITIES_H

#include <AK/SoundEngine/Common/AkTypes.h>

#include "ISourceControlDialogBase.h"
#include "ISourceControlOperationProgress.h"

// Audiokinetic namespace
namespace AK
{
	// Audiokinetic Wwise namespace
	namespace Wwise
	{
		/// Wwise source control utilities interface. This interface is provided when the plug-in is initialized.
		/// With this interface, you can display a progress dialog, create custom dialogs, display message boxes, and
		/// save the plug-in configuration to the registry.
		class ISourceControlUtilities
		{
		public:
			/// Get a pointer to an AK::Wwise::ISourceControlOperationProgress interface, so you can display a simple progress dialog for the operation.
			/// \warning This function is not thread-safe.
			/// \return A pointer to an AK::Wwise::ISourceControlOperationProgress interface.
			virtual ISourceControlOperationProgress* GetProgressDialog() = 0;

			/// This function does the same thing as the standard ::MessageBox function, except that this one will
			/// be displayed with the Wwise UI look and feel.
			/// \warning This function is not thread-safe.
			/// \return The window results of the dialog
			virtual int MessageBox( 
				HWND in_hWnd,					///< The window handle of the dialog
				LPCWSTR in_pszText,				///< The text to be displayed in the message box
				LPCWSTR in_pszCaption,			///< The caption of the message box
				UINT in_uiType					///< The window message box type (e.g. MB_OK)
				) = 0;

			/// This function show a dialog with a edit field and allow the user enter input string
			/// \warning This function is not thread-safe.
			/// \return The window results of the dialog: IDOK or IDCANCEL
			virtual int PromptMessage( 
				HWND in_hWnd,					///< The window handle of the dialog
				LPCWSTR in_pszText,				///< The text to be displayed in the message box
				LPCWSTR in_pszCaption,			///< The caption of the message box
				LPWSTR out_pszInput,			///< The buffer to receive the user input
				UINT in_uiInputSize,			///< The size of the buffer to receive input
				bool in_bIsPassword				///< True to hide text; used for passwords
				) = 0;
	
			/// Show a browse for folder dialog.  
			/// \warning This function is not thread-safe.
			/// \return The resulting path is set in out_pszChoosenPath
			/// \return True if user clicked OK, false if user clicked Cancel
			virtual bool ShowBrowseForFolderDialog(
				LPCWSTR in_pszDialogTitle,			///< The dialog title
				LPWSTR out_pszChoosenPath,			///< The choosen path
				UINT in_uiChoosenPathSize,			///< The size of the buffer to receive path (out_pszChoosenPath)
				LPCWSTR in_pszRootPath = NULL		///< The root path for the browse for folder dialog
				) = 0;

			/// This function does the same thing as the CDialog::DoModal function.
			/// \warning This function is not thread-safe.
			/// \return The window results of the dialog (e.g. IDOK)
			virtual INT_PTR CreateModalCustomDialog( 
				ISourceControlDialogBase* in_pDialog	///< A pointer to a dialog class that implements 
														///< AK::Wwise::ISourceControlDialogBase functions.
				) = 0;

			/// Get the path to the registry for the current project. This path is to be used with
			/// the HKEY_CURRENT_USER registry key.
			/// \warning This function is not thread-safe.
			/// \return A string containing the registry path.
			virtual LPCWSTR GetRegistryPath() = 0;

			/// Set DWORD value in user preferences.
			/// \warning This function is not thread-safe.
			virtual void SetUserPreferenceDword(
				LPCWSTR in_pszPreference,		///< Name of preference
				DWORD in_dwValue				///< Value to set in user preferences.
				) = 0;

			/// Get DWORD value in user preferences.
			/// \warning This function is not thread-safe.
			virtual void GetUserPreferenceDword(
				LPCWSTR in_pszPreference,		///< Name of preference
				DWORD& io_dwValue				///< in: value to return if preference is not set; out: changed to user preference value if set
				) = 0;

			/// Set string value in user preferences.
			/// \warning This function is not thread-safe.
			virtual void SetUserPreferenceString(
				LPCWSTR in_pszPreference,		///< Name of preference
				LPCWSTR in_pszValue				///< Value to set in user preferences.
				) = 0;

			/// Get string value from user preferences.
			/// \warning This function is not thread-safe.
			virtual void GetUserPreferenceString(
				LPCWSTR in_pszPreference,		///< Name of preference
				LPWSTR io_pszValue,				///< in: value to return if preference is not set; out: changed to user preference value if set
				DWORD in_dwSize					///< Size of out_pszValue buffer.
				) = 0;

			/// Get the root path for a move operation.  
			/// The input file can either be a work unit or a source file
			/// \warning This function is not thread-safe.
			/// \return Nothing as return value.  The out_pszRootPath will contain the path.
			virtual void GetMoveRootPath( 
				LPCWSTR in_pszFullPath,			///< The full path of an audio source or work unit file
				LPWSTR out_pszRootPath,			///< A pointer to the array that receives the root path
				UINT in_uiRootPathSize			///< The size of the array that receives the root path
				) = 0;

			/// Create a AK style list control with 3 columns:
			/// - Filename
			/// - Status
			/// - Owner
			/// You must create a static control in the resources delimitating the region of the list control and
			/// pass the control ID of it.
			/// \note DestroyFileStatusListControl must be called when handling WM_DESTROY in WindowProc
			/// \warning This function is not thread-safe.
			virtual void CreateFileStatusListControl( 
				HWND in_hWndParent,					///< The parent dialog to create the list control
				UINT in_idStatic,					///< The ID of the placeholder static control, which will also be the ID of the list control after the creation
				const WCHAR** in_ppFilenameList,	///< The list of files to show in the list
				unsigned int in_uiFilenameListCount	///< the number of files in the in_ppFilenameList array
				) = 0;
		};							  
	}
}

#endif // _AK_WWISE_ISOURCECONTROLUTILITIES_H