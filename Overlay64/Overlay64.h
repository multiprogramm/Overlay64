#pragma once
#include <atlbase.h>
#include <atlcom.h>
#include <Shlobj.h>

#include <string>
#include <algorithm>
#include <fstream>

#include "resource.h"

extern CComModule g_Module;

using namespace ATL;

class DECLSPEC_UUID( "5747094E-84FB-47B4-BC0C-F89FB583895F" ) COverlay64 :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<COverlay64, &__uuidof( COverlay64 )>,
	public IShellIconOverlayIdentifier
{
public:
DECLARE_NOT_AGGREGATABLE( COverlay64 )

BEGIN_COM_MAP( COverlay64 )
	COM_INTERFACE_ENTRY( IShellIconOverlayIdentifier )
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

	COverlay64() = default;

	static HRESULT WINAPI UpdateRegistry( BOOL bRegister )
	{
		static const std::wstring clsid( ToString( __uuidof( COverlay64 ) ) );
		static struct _ATL_REGMAP_ENTRY map[] = {
			{ L"CLSID", clsid.c_str() },
			{ L"DESCRIPTION", L"Overlay64" },
			{ L"PROGID", L"Overlay64.multiprogramm" },
			{ NULL, NULL }
		};

		return g_Module.UpdateRegistryFromResource( static_cast<UINT>( IDR_REG_OVERLAY64 ), bRegister,
			reinterpret_cast<_ATL_REGMAP_ENTRY*>( &map ) );
	}



	// IShellIconOverlayIdentifier
	IFACEMETHODIMP IsMemberOf( PCWSTR pwszPath, DWORD dwAttrib ) override
	{
		try
		{
			if( dwAttrib & ( SFGAO_LINK | SFGAO_FOLDER ) )
				return S_FALSE;
			static const std::wstring dll_ext = L".dll";
			static const std::wstring exe_ext = L".exe";

			std::wstring ext( ::PathFindExtension( pwszPath ) );
			std::transform( ext.begin(), ext.end(), ext.begin(), ::tolower );
			if( ext == dll_ext || ext == exe_ext )
			{
				if( Is64Bit( pwszPath ) )
					return S_OK;
			}
		}
		catch( ... )
		{
			return E_FAIL;
		}
		
		return S_FALSE;
	}

	IFACEMETHODIMP GetOverlayInfo( PWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags ) override
	{
		*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
		HRESULT hr = S_OK;
		const int cbCopied = GetModuleFileName( g_Module.m_hInst, pwszIconFile, cchMax );
		if( cbCopied <= 0 )
			hr = HRESULT_FROM_WIN32( GetLastError() );
		*pIndex = 0;
		return S_OK;
	}

	IFACEMETHODIMP GetPriority( int* pIPriority ) override
	{
		*pIPriority = 0;
		return S_OK;
	}

private:

	static std::wstring ToString( const GUID& guid )
	{
		std::wstring result( 40, '\0' );
		::StringFromGUID2( guid, &result[0], static_cast<int>( result.size() ) );
		return std::wstring( result.c_str() );
	}

	static_assert( sizeof( uint32_t ) == 4, "Incorrect uint32_t size." );
	uint32_t ReadUInt32( std::ifstream& str )
	{
		uint32_t result;
		str.read( reinterpret_cast<char*>( &result ) , sizeof( result ) );
		return result;
	}

	static_assert( sizeof( uint16_t ) == 2, "Incorrect uint16_t size." );
	uint16_t ReadUInt16( std::ifstream& str )
	{
		uint16_t result;
		str.read( reinterpret_cast<char*>( &result ), sizeof( result ) );
		return result;
	}

	bool Is64Bit( const wchar_t* const file_path )
	{
		std::ifstream stream( file_path, std::ios::binary );
		if( !stream.is_open() )
			return false;
	
		const uint16_t first = ReadUInt16( stream );
		if( !stream.good() )
			return false;
		if( first != IMAGE_DOS_SIGNATURE )
			return false; // Not a valid executable

		// Offset to PE header is always at 0x3C
		stream.seekg( 0x3c, std::ios::beg );
		const uint32_t pe_offset = ReadUInt32( stream );
		if( !stream.good() )
			return false;

		// Apply offset
		stream.seekg( pe_offset, std::ios::beg );
		const uint32_t pe_head = ReadUInt32( stream );
		if( !stream.good() )
			return false;
		if( pe_head != IMAGE_NT_SIGNATURE )
			return false; // Not a PE header
	
		// Next: 2-byte machine type field
		const uint16_t machine_type = ReadUInt16( stream );
		if( !stream.good() )
			return false;
		return machine_type == IMAGE_FILE_MACHINE_AMD64
			|| machine_type == IMAGE_FILE_MACHINE_IA64;
	}
};
