#pragma once

// Predelceration incase we do not have access here (we do not need specific access)
struct WMLib_FileInfoEx;

// Write video and audio assets
bool FileManagementDLL WriteVideoFileAssets( const char *pFileName, const WMLib_FileInfoEx *pFormat );
bool FileManagementDLL WriteVideoFileAssets( FileProperties *pFileProperties, const WMLib_FileInfoEx *pFormat );