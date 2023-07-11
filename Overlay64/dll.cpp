#include <atlbase.h>
#include <atlcom.h>
#include "Overlay64.h"

// Global:
CComModule g_Module;

BEGIN_OBJECT_MAP( g_ObjectMap )
	OBJECT_ENTRY( __uuidof( COverlay64 ), COverlay64 )
END_OBJECT_MAP()


STDAPI_(BOOL) DllMain( HINSTANCE hInstance, DWORD dwReason, void* )
{
	switch( dwReason )
	{
		case DLL_PROCESS_ATTACH:
			g_Module.Init( g_ObjectMap, hInstance );
			::DisableThreadLibraryCalls( hInstance );
			break;

		case DLL_PROCESS_DETACH:
			g_Module.Term();
			break;

		default:
			break;
	}
	return TRUE;
}

STDAPI DllCanUnloadNow()
{
	return ( g_Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppv )
{
	return g_Module.GetClassObject( rclsid, riid, ppv );
}

STDAPI DllRegisterServer()
{
	return g_Module.RegisterServer( FALSE, &__uuidof( COverlay64 ) );
}

STDAPI DllUnregisterServer()
{
	return g_Module.UnregisterServer( FALSE, &__uuidof( COverlay64 ) );
}
