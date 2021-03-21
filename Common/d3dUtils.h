#ifndef D3D_UTILS_H
#define D3D_UTILS_H

// d3d12 common header.
#include <dxgi1_4.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <d3dcompiler.h>
#include "d3dx12.h"
#include <comdef.h> /// For _com_ptr
#include <wrl.h> /// For ComPtr

// for _ASSERT in crt.
#if defined(_DEBUG) || defined(DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "Common.h"

#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           if(FAILED(hr = (x)) && (DebugBreak(), 1)) { DXTraceW(__FILEW__, __LINE__, hr, L#x, true); }
#define V2(x, ...)     if(FAILED(hr = ((x), ##__VA_ARGS__)) && (DebugBreak(), 1)) { DXTraceW(__FILEW__, __LINE__, hr, L#x, true); }
#endif

#ifndef V_RETURN
#define V_RETURN(x)    if(FAILED(hr = (x)) && (DebugBreak(), 1)) { DXTraceW(__FILEW__, __LINE__, hr, L#x, true);  return hr; }
#define V_RETURN2(x, ...) if(FAILED(hr = ((x), ##__VA_ARGS__)) && (DebugBreak(), 1)) { DXTraceW(__FILEW__, __LINE__, hr, L#x, true);  return hr; }
#endif /* V_RETURN2 */
#else
#ifndef V
#define V(x)                (hr = (x));
#define V2(x, ...)           (hr = (x, ##__VA_ARGS));
#endif
#ifndef V_RETURN
#define V_RETURN(x)         if(FAILED(hr = (x))) {return hr; }
#define V_RETURN2(x, ...)   if(FAILED(hr=(x, ##__VA_ARGS__))) { return hr; }
#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p) = nullptr; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p) = nullptr; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = nullptr; } }
#endif

#ifndef SAFE_ADDREF
#define SAFE_ADDREF(p) { (p) ? (p)->AddRef() : (ULONG)0; }
#endif

// Use DXUT_SetDebugName() to attach names to D3D objects for use by 
// SDKDebugLayer, PIX's object table, etc.
#if defined(_DEBUG) || defined(DEBUG)
inline void DX_SetDebugName(IDXGIObject* pObj, const CHAR* pstrName) {
    if (pObj)
        pObj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pstrName), pstrName);
}
inline void DX_SetDebugName(ID3D12Device *pObj, const CHAR *pstrName) {
    if (pObj) {
        pObj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pstrName), pstrName);
    }
}
inline void DX_SetDebugName(ID3D12DeviceChild *pObj, const CHAR *pstrName) {
    if (pObj) {
        pObj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pstrName), pstrName);
    }
}

#else
#define DX_SetDebugName( pObj, pstrName )
#endif

struct Unknown12
{
    Unknown12();
    virtual ~Unknown12();
    ULONG AddRef();
    ULONG Release();

private:
    Unknown12(const Unknown12 &) = delete;
    Unknown12(Unknown12 &&) = delete;
    Unknown12 &operator= (const Unknown12 &) = delete;
    Unknown12 &operator= (Unknown12&&) = delete;

    volatile ULONG m_uRefcnt;
};

struct NonCopyable {
  NonCopyable() = default;
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable(NonCopyable &&) = delete;
  NonCopyable operator = (const NonCopyable &) = delete;
  NonCopyable& operator = (NonCopyable &&) = delete;
};

extern HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr,
    _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox);

extern void DXOutputDebugStringA(LPCSTR fmt, ...);
extern void DXOutputDebugStringW(LPCWSTR fmt, ...);

namespace d3dUtils {

  class DxcCompilerWrapper: public Unknown12 {
  public:
    DxcCompilerWrapper();
    ~DxcCompilerWrapper();

    HRESULT CreateCompiler();
    HRESULT CreateIncludeHandler(
      _COM_Outptr_ IDxcIncludeHandler **pIncludeHandler
    );

    HRESULT CompileFromFile(
      _In_ LPCWSTR pFileNname,                      // Source text to compile
      _In_ LPCWSTR pEntryPoint,                     // entry point name
      _In_ LPCWSTR pTargetProfile,                  // shader profile to compile
      _In_count_(argCount) LPCWSTR *pArguments,     // Array of pointers to arguments
      _In_ UINT32 argCount,                         // Number of arguments
      _In_count_(defineCount) const DxcDefine *pDefines,  // Array of defines
      _In_ UINT32 defineCount,                      // Number of defines
      _In_opt_ IDxcIncludeHandler *pIncludeHandler, // user-provided interface to handle #include directives (optional)
      _COM_Outptr_ IDxcBlob **ppResult,              // Result buffer
      _COM_Outptr_opt_ IDxcBlob **ppErrorBlob            // Store possible error.
    );

  private:
    IDxcCompiler *m_pDxcCompiler;
    IDxcLibrary *m_pDxcLibrary;
  };

  extern
  HRESULT CompileShaderFromFile(
    _In_ LPCWSTR                pFileName,
    _In_ const D3D_SHADER_MACRO *pDefines,
    _In_ ID3DInclude            *pInclude,
    _In_ LPCSTR                 pEntrypoint,
    _In_ LPCSTR                 pTarget,
    _In_ UINT                   Flags1,
    _In_ UINT                   Flags2,
    _Out_ ID3DBlob               **ppCode,
    _Out_ ID3DBlob               **ppErrorMsgs
  );

  extern HRESULT CreateDefaultBuffer(
      ID3D12Device *pd3dDevice,
      ID3D12GraphicsCommandList *pCmdList,
      const void *pInitData,
      UINT_PTR uByteSize,
      ID3D12Resource **ppUploadBuffer,
      ID3D12Resource **ppDefaultBuffer
  );

  extern UINT CalcConstantBufferByteSize(UINT uByteSize);

  ///
/// Shader binding table for binding resource between pipeline and resource views,
/// for example root constants, CBV, SRV, UAV.
///
  struct ROOT_CONSTANT_BINDING_ENTRY {
    UINT Num32BitValues;
    const void *p32BitValues;
    UINT Dest32BitOffset;
  };

  union DESCRIPTOR_BINDING_ENTRY {
    D3D12_GPU_VIRTUAL_ADDRESS   Address;
    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHandle;
  };

  typedef D3D12_ROOT_PARAMETER_TYPE SHADER_BINDING_ENTRY_TYPE;

  struct SHADER_BINDING_ENTRY {
    UINT RootParameterIndex;
    D3D12_ROOT_PARAMETER_TYPE Type;
    union {
      ROOT_CONSTANT_BINDING_ENTRY RootConstants;
      DESCRIPTOR_BINDING_ENTRY    RootDescriptor;
      DESCRIPTOR_BINDING_ENTRY    DescriptorTableEntry;
    };
  };


  struct SHADER_BINDING_TABLE {
    UINT NumBindingEntry;
    const SHADER_BINDING_ENTRY *pBindingEntries;
  };

};

#endif /* D3D_UTILS_H */