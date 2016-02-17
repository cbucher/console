// code found in http://win7shell.googlecode.com/svn/trunk/jumplist.cpp

#include "StdAfx.h"
#include "JumpList.h"
#include "Console.h"

#include <propkey.h>
#include <propvarutil.h>

// Creates a CLSID_ShellLink to insert into the Tasks section of the Jump List.  This type of Jump
// List item allows the specification of an explicit command line to execute the task.
static HRESULT _CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl)
{
  CComPtr<IShellLink> psl;
  HRESULT hr = psl.CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER);
  if (FAILED(hr))
    return hr;

  // path
  wchar_t szModulePath[MAX_PATH];
  ::ZeroMemory(szModulePath, sizeof(szModulePath));
  if( !::GetModuleFileName(NULL, szModulePath, MAX_PATH) )
    return HRESULT_FROM_WIN32(::GetLastError());

  hr = psl->SetPath(szModulePath);
  if (FAILED(hr))
    return hr;

  // arguments
  hr = psl->SetArguments(pszArguments);
  if (FAILED(hr))
    return hr;

  // The title property is required on Jump List items provided as an IShellLink
  // instance.  This value is used as the display name in the Jump List.
  CComPtr<IPropertyStore> pps;
  hr = psl.QueryInterface(&pps);
  if (FAILED(hr))
    return hr;

  PROPVARIANT propvar;
  hr = ::InitPropVariantFromString(pszTitle, &propvar);
  if (FAILED(hr))
    return hr;

  hr = pps->SetValue(PKEY_Title, propvar);
  ::PropVariantClear(&propvar);
  if (FAILED(hr))
    return hr;

  hr = pps->Commit();
  if (FAILED(hr))
    return hr;

  hr = psl->SetWorkingDirectory(g_settingsHandler->GetSettingsPath().c_str());
  if (FAILED(hr))
    return hr;

  *ppsl = psl.Detach();

  return hr;
}

static HRESULT SetIconLocation(IShellLink *psl, LPCWSTR szIconLocation)
{
  wchar_t szIconLocationFullName[_MAX_PATH];
  if( !::GetFullPathName(szIconLocation, ARRAYSIZE(szIconLocationFullName), szIconLocationFullName, 0) )
    return HRESULT_FROM_WIN32(::GetLastError());

  return psl->SetIconLocation(szIconLocationFullName, 0);
}

static HRESULT SetIcon(IShellLink *psl, std::shared_ptr<TabData> tab)
{
  if (tab->bUseDefaultIcon)
  {
    if ( !tab->strShell.empty() )
    {
      wstring strCommandLine = Helpers::ExpandEnvironmentStrings(tab->strShell);
      int argc = 0;
      std::unique_ptr<LPWSTR[], LocalFreeHelper> argv(::CommandLineToArgvW(strCommandLine.c_str(), &argc));

      if ( argv && argc > 0 )
      {
        return ::SetIconLocation(psl, argv[0]);
      }
    }
  }
  else
  {
    if (!tab->strIcon.empty())
    {
      return ::SetIconLocation(psl, Helpers::ExpandEnvironmentStrings(tab->strIcon).c_str());
    }
  }

  wchar_t szModulePath[MAX_PATH];
  ::ZeroMemory(szModulePath, sizeof(szModulePath));
  ::GetModuleFileName(NULL, szModulePath, MAX_PATH);

  return ::SetIconLocation(psl, szModulePath);
}

void JumpList::CreateList(TabDataVector& tabDataVector)
{
  if( !g_settingsHandler->GetAppearanceSettings().stylesSettings.bJumplist )
    return;

  CComPtr<ICustomDestinationList> pcdl;
  HRESULT hr = pcdl.CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER);
  if (FAILED(hr))
    return;

  UINT cMinSlots;
  CComPtr<IObjectArray> poaRemoved;
  hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
  if (FAILED(hr))
    return;

  CComPtr<IObjectCollection> poc;
  hr = poc.CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC);
  if (FAILED(hr))
    return;

  for (TabDataVector::iterator it = tabDataVector.begin(); it != tabDataVector.end(); ++it)
  {
    CComPtr<IShellLink> psl;
    wstring quotedName(L"-t ");
    quotedName.append(Helpers::EscapeCommandLineArg((*it)->strTitle));
    quotedName.append(L" -c ");
    quotedName.append(Helpers::EscapeCommandLineArg(g_settingsHandler->GetSettingsFileName()));
    if (SUCCEEDED(_CreateShellLink(quotedName.c_str(), (*it)->strTitle.c_str(), &psl)))
    {
      SetIcon(psl, *it);
      poc->AddObject(psl);
    }
  }

  CComPtr<IObjectArray> poa;
  hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
  if (FAILED(hr))
    return;

  // Add the tasks to the Jump List. Tasks always appear in the canonical "Tasks"
  // category that is displayed at the bottom of the Jump List, after all other
  // categories.
  //hr = pcdl->AddUserTasks(poa);
  hr = pcdl->AppendCategory(g_settingsHandler->GetSettingsTitle().c_str(),poa);
  if (FAILED(hr))
    return;

  pcdl->CommitList();
}
