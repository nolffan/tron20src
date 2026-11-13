/****************************************************************************
;
;	 MODULE:		TROND3D9 (.H)
;
;	PURPOSE:		Direct3D 9's interface, as far as the launcher uses it
;
;	COMMENT:		tron20src (the file and its contents are ours).
;
;	Tron 2.0's launcher enumerates the display modes through Direct3D 9
;	(CRenderer::Init: "d3d9.dll", "Direct3DCreate9", D3D_SDK_VERSION 31,
;	the DirectX 9.0b SDK's; RETAIL: 0x4053b0, and the developer build of
;	2003-08-05 already does). The DirectX 9 SDK is not in ext/toolchains/
;	yet (build/build.mk), so this declares what the launcher needs of its
;	d3d9.h, laid out as the SDK lays it out: IDirect3D9's methods in its
;	order, D3DADAPTER_IDENTIFIER9. The types Direct3D 9 shares with
;	Direct3D 8 (D3DFORMAT, D3DDISPLAYMODE) are the DirectX 8.1 SDK's.
;	Once the DirectX 9 SDK is in the toolchains, DisplayMgr.cpp includes
;	its d3d9.h instead.
;
****************************************************************************/

#ifndef TRON_D3D9_H
#define TRON_D3D9_H

#include <objbase.h>
#include "d3d8types.h"

// (as the SDKs' d3d8.h and d3d9.h declare it, for WINVER below 0x0500)
#if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
	#define HMONITOR_DECLARED
	DECLARE_HANDLE(HMONITOR);
#endif

// The DirectX 9.0b SDK's
#define D3D9_SDK_VERSION 31

#define MAX_DEVICE_IDENTIFIER_STRING 512

// (d3d9types.h's structures are 4-byte packed)
#pragma pack(4)

typedef struct _D3DADAPTER_IDENTIFIER9
{
	char            Driver[MAX_DEVICE_IDENTIFIER_STRING];
	char            Description[MAX_DEVICE_IDENTIFIER_STRING];
	char            DeviceName[32];
	LARGE_INTEGER   DriverVersion;
	DWORD           VendorId;
	DWORD           DeviceId;
	DWORD           SubSysId;
	DWORD           Revision;
	GUID            DeviceIdentifier;
	DWORD           WHQLLevel;
} D3DADAPTER_IDENTIFIER9;

#pragma pack()

// The arguments of the methods the launcher does not call
struct _D3DCAPS9;
struct _D3DPRESENT_PARAMETERS_;
struct IDirect3DDevice9;

#undef INTERFACE
#define INTERFACE IDirect3D9

DECLARE_INTERFACE_(IDirect3D9, IUnknown)
{
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	/*** IDirect3D9 methods ***/
	STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction) PURE;
	STDMETHOD_(UINT, GetAdapterCount)(THIS) PURE;
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier) PURE;
	STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter,D3DFORMAT Format) PURE;
	STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode) PURE;
	STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter,D3DDISPLAYMODE* pMode) PURE;
	STDMETHOD(CheckDeviceType)(THIS_ UINT iAdapter,D3DDEVTYPE DevType,D3DFORMAT DisplayFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed) PURE;
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat) PURE;
	STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels) PURE;
	STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat) PURE;
	STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat) PURE;
	STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,struct _D3DCAPS9* pCaps) PURE;
	STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter) PURE;
	STDMETHOD(CreateDevice)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,struct _D3DPRESENT_PARAMETERS_* pPresentationParameters,struct IDirect3DDevice9** ppReturnedDeviceInterface) PURE;
};

#undef INTERFACE

#endif // TRON_D3D9_H
