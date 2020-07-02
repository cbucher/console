#include "StdAfx.h"
#include "ImageHandler.h"

//////////////////////////////////////////////////////////////////////////////

bool ImageHandler::m_win8 = ImageHandler::CheckWin8();

bool ImageHandler::CheckWin8()
{
  OSVERSIONINFOEX osvi;
  DWORDLONG dwlConditionMask = 0;
  BYTE op = VER_GREATER_EQUAL;

  // Initialize the OSVERSIONINFOEX structure.

  ::ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  osvi.dwMajorVersion = 6;
  osvi.dwMinorVersion = 2;

  // Initialize the condition mask.

  VER_SET_CONDITION( dwlConditionMask, VER_MAJORVERSION, op );
  VER_SET_CONDITION( dwlConditionMask, VER_MINORVERSION, op );

  // Perform the test.

  if( ::VerifyVersionInfo(
    &osvi, 
    VER_MAJORVERSION | VER_MINORVERSION | 
    VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
    dwlConditionMask) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

ImageHandler::ImageHandler()
: m_images()
, m_hWnd(nullptr)
, m_bAnimationStarted(false)
{
}

ImageHandler::~ImageHandler()
{
}

#ifdef _USE_AERO
void ImageHandler::StartAnimation(HWND hWnd)
{
	if( m_bAnimationStarted == true ) return;

	m_hWnd = hWnd;
	for( auto i : m_images )
	{
		i->originalImage->nextFrame(m_hWnd);
	}

	m_bAnimationStarted = true;
}

void ImageHandler::StopAnimation()
{
	if( m_bAnimationStarted == false ) return;

	for( auto i : m_images )
	{
		i->originalImage->stop(m_hWnd);
	}

	m_bAnimationStarted = false;
}

void ImageHandler::NextFrame(UINT_PTR nIDTimerEvent)
{
	if( m_bAnimationStarted == false ) return;

	for( auto i : m_images )
	{
		if( i->originalImage->getIDTimerEvent() == nIDTimerEvent )
		{
			i->originalImage->nextFrame(m_hWnd);
			break;
		}
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::CalcRescale(DWORD& dwNewWidth, DWORD& dwNewHeight, std::shared_ptr<BackgroundImage>& bkImage)
{
  switch( bkImage->imageData.imagePosition )
  {
  case imagePositionFit:
    {
      double dXRatio = (double)dwNewWidth  / (double)bkImage->originalImage->getWidth ();
      double dYRatio = (double)dwNewHeight / (double)bkImage->originalImage->getHeight();

      if( dXRatio < dYRatio )
      {
        dwNewHeight = ::MulDiv(bkImage->originalImage->getHeight(), dwNewWidth, bkImage->originalImage->getWidth());
      }
      else
      {
        dwNewWidth  = ::MulDiv(bkImage->originalImage->getWidth(), dwNewHeight, bkImage->originalImage->getHeight());
      }
    }
    break;

  case imagePositionFill:
    {
      double dXRatio = (double)dwNewWidth  / (double)bkImage->originalImage->getWidth ();
      double dYRatio = (double)dwNewHeight / (double)bkImage->originalImage->getHeight();

      if( dXRatio > dYRatio )
      {
        dwNewHeight = ::MulDiv(bkImage->originalImage->getHeight(), dwNewWidth, bkImage->originalImage->getWidth());
      }
      else
      {
        dwNewWidth  = ::MulDiv(bkImage->originalImage->getWidth(), dwNewHeight, bkImage->originalImage->getHeight());
      }
    }
    break;

	case imagePositionCenter:
		{
			dwNewWidth  = bkImage->originalImage->getWidth();
			dwNewHeight = bkImage->originalImage->getHeight();
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackgroundImage> ImageHandler::GetImage(const ImageData& imageData)
{
	std::shared_ptr<BackgroundImage>	bkImage(new BackgroundImage(imageData));

	Images::iterator		itImage = m_images.begin();

	for (; itImage != m_images.end(); ++itImage) if ((*itImage)->imageData == bkImage->imageData) break;

	// found image, return
	if (itImage != m_images.end()) return *itImage;

	// else, try to load image
	if (!LoadImage(bkImage)) return std::shared_ptr<BackgroundImage>();

	m_images.push_back(bkImage);

#ifdef _USE_AERO
	if( m_bAnimationStarted ) bkImage->originalImage->nextFrame(m_hWnd);
#endif

	return bkImage;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackgroundImage> ImageHandler::GetDesktopImage(ImageData& imageData)
{
	if (!GetDesktopImageData(imageData)) return std::shared_ptr<BackgroundImage>();

	// now, find the image
	Images::iterator itImage = m_images.begin();

	for (; itImage != m_images.end(); ++itImage) if ((*itImage)->imageData == imageData) break;

	// found image, return
	if (itImage != m_images.end()) return *itImage;

	// else, try to load image
	std::shared_ptr<BackgroundImage>	bkImage(new BackgroundImage(imageData));

	bkImage->bWallpaper = true;

//	if (!LoadImage(bkImage)) return std::shared_ptr<BackgroundImage>();
	// we always return background image, even if there's no wallpaper selected
	LoadImage(bkImage);

	m_images.push_back(bkImage);

#ifdef _USE_AERO
	if( m_bAnimationStarted ) bkImage->originalImage->nextFrame(m_hWnd);
#endif

	return bkImage;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BackgroundImage> ImageHandler::GetBingImage(ImageData& imageData)
{
	if (!GetBingImageData(imageData)) return std::shared_ptr<BackgroundImage>();

	// now, find the image
	Images::iterator itImage = m_images.begin();

	for (; itImage != m_images.end(); ++itImage) if ((*itImage)->imageData == imageData) break;

	// found image, return
	if (itImage != m_images.end()) return *itImage;

	// else, try to load image
	std::shared_ptr<BackgroundImage> bkImage(new BackgroundImage(imageData));

	try
	{
		std::vector<char> content;
		Helpers::GetUrlContent(imageData.strFilename, content);

		LoadImageFromContent(bkImage, content);
	}
	catch( std::exception& )
	{
	}

	m_images.push_back(bkImage);

#ifdef _USE_AERO
	if( m_bAnimationStarted ) bkImage->originalImage->nextFrame(m_hWnd);
#endif

	return bkImage;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::ReloadDesktopImages()
{
	Images::iterator itImage = m_images.begin();

	for (; itImage != m_images.end(); ++itImage)
	{
		if (!(*itImage)->bWallpaper) continue;

		// TODO: how to handle these two?
		if (!GetDesktopImageData((*itImage)->imageData)) continue;
		if (!LoadImage(*itImage)) continue;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::UpdateImageBitmap(const CDC& dc, const CRect& clientRect, std::shared_ptr<BackgroundImage>& bkImage, bool force)
{
	if (bkImage->imageData.bRelative)
	{
		if( !force
		    &&
		    !bkImage->image.IsNull() ) // first access to relative image
			return;

		CreateRelativeImage(dc, bkImage);
	}
	else
	{
		if( !force
		    &&
		    bkImage->dwImageWidth == static_cast<DWORD>(clientRect.Width())
		    &&
		    bkImage->dwImageHeight == static_cast<DWORD>(clientRect.Height()) )
		{
			// no client size change, nothing to do
			return;
		}

		CreateImage(dc, clientRect, bkImage);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ImageHandler::GetDesktopImageData(ImageData& imageData)
{
	CRegKey keyColors;

	if (keyColors.Open(HKEY_CURRENT_USER, L"Control Panel\\Colors", KEY_READ) != ERROR_SUCCESS) return false;

	CString	strBackground;
	DWORD	dwDataSize = 32;

	if (keyColors.QueryStringValue(L"Background", strBackground.GetBuffer(dwDataSize), &dwDataSize) != ERROR_SUCCESS)
	{
		strBackground.ReleaseBuffer();
		return false;
	}
	strBackground.ReleaseBuffer();

	wstringstream	streamBk(wstring(strBackground.operator LPCTSTR()));
	DWORD			r = 0;
	DWORD			g = 0;
	DWORD			b = 0;

	streamBk >> r;
	streamBk >> g;
	streamBk >> b;

	CRegKey keyDesktop;
	if (keyDesktop.Open(HKEY_CURRENT_USER, L"Control Panel\\Desktop", KEY_READ) != ERROR_SUCCESS) return false;

	DWORD	dwChars = 0;
	CString	strWallpaperFile;
	CString	strWallpaperStyle;
	CString	strWallpaperTile;

	dwChars = 32;
	keyDesktop.QueryStringValue(L"WallpaperStyle", strWallpaperStyle.GetBuffer(dwChars), &dwChars);
	dwChars = 32;
	keyDesktop.QueryStringValue(L"TileWallpaper", strWallpaperTile.GetBuffer(dwChars), &dwChars);
	dwChars = MAX_PATH;
	keyDesktop.QueryStringValue(L"Wallpaper", strWallpaperFile.GetBuffer(dwChars), &dwChars);

	strWallpaperStyle.ReleaseBuffer();
	strWallpaperFile.ReleaseBuffer();
	strWallpaperTile.ReleaseBuffer();

	// set background data
	imageData.strFilename  = strWallpaperFile;
	imageData.bRelative    = true;
	imageData.bExtend      = false;
	imageData.crBackground = RGB(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b));

	if (strWallpaperTile == L"1")
	{
		imageData.imagePosition= imagePositionTile;
	}
	else
	{
		if (strWallpaperStyle == L"0")
		{
			imageData.imagePosition= imagePositionCenter;
		}
		else if (strWallpaperStyle == L"6")
		{
			imageData.imagePosition= imagePositionFit;
		}
		else if (strWallpaperStyle == L"10")
		{
			imageData.imagePosition= imagePositionFill;
		}
		else if (strWallpaperStyle == L"22")
		{
			imageData.imagePosition= imagePositionFill;
			imageData.bExtend = true;
		}
		else
		{
			imageData.imagePosition= imagePositionStretch;
		}
	}

#if _WIN32_WINNT >= 0x0602
	if( ImageHandler::IsWin8() )
	{
		CComPtr<IDesktopWallpaper> desktopWallpaper;
		HRESULT hr = desktopWallpaper.CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL);
		if( SUCCEEDED(hr) )
		{
			DESKTOP_WALLPAPER_POSITION position;
			hr = desktopWallpaper->GetPosition(&position);

			UINT count = 0;
			hr = desktopWallpaper->GetMonitorDevicePathCount(&count);
			if( hr == S_OK && position == DWPOS_SPAN && count > 1 )
			{
				CComHeapPtr<wchar_t> spszWallpaper;
				hr = desktopWallpaper->GetWallpaper(NULL, &spszWallpaper);
				if( hr == S_OK )
					imageData.bExtend = true;
			}
		}
	}
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ImageHandler::GetBingImageData(ImageData& imageData)
{
	try
	{
		std::vector<char> content;
		Helpers::GetUrlContent(L"http://www.bing.com/HPImageArchive.aspx?format=xml&idx=0&n=1&mkt=en-US", content);

		CComPtr<IXMLDOMDocument> pBingDocument;
		CComPtr<IXMLDOMElement>  pBingRoot;

		if( SUCCEEDED(XmlHelper::OpenXmlDocumentFromContent(content, pBingDocument, pBingRoot)) )
		{
			/*
			CComPtr<IXMLDOMNodeList> pUrlBaseNodes;
			if( FAILED(pBingRoot->selectNodes(CComBSTR(L"/images/image/urlBase"), &pUrlBaseNodes)) ) return false;

			long lListLength;
			if( FAILED(pUrlBaseNodes->get_length(&lListLength)) ) return false;

			for( long i = 0; i < lListLength; ++i )
			{
				CComPtr<IXMLDOMNode>    pUrlBaseNode;
				CComPtr<IXMLDOMElement> pUrlBaseElement;

				if( FAILED(pUrlBaseNodes->get_item(i, &pUrlBaseNode)) ) return false;
				if( FAILED(pUrlBaseNode.QueryInterface(&pUrlBaseElement)) ) return false;

				CComBSTR bstrUrlBase;
				if( FAILED(pUrlBaseElement->get_text(&bstrUrlBase)) ) return false;
			}
			*/

			CComPtr<IXMLDOMNode> pUrlBaseNode;
			if( FAILED(pBingRoot->selectSingleNode(CComBSTR(L"/images/image/urlBase"), &pUrlBaseNode)) ) return false;

			CComPtr<IXMLDOMElement> pUrlBaseElement;
			if( FAILED(pUrlBaseNode.QueryInterface(&pUrlBaseElement)) ) return false;

			CComBSTR bstrUrlBase;
			if( FAILED(pUrlBaseElement->get_text(&bstrUrlBase)) ) return false;

			imageData.strFilename = std::wstring(L"http://www.bing.com");
			imageData.strFilename += bstrUrlBase;
			imageData.strFilename += L"_1920x1080.jpg";

			CComPtr<IXMLDOMNode> pCopyrightNode;
			if( FAILED(pBingRoot->selectSingleNode(CComBSTR(L"/images/image/copyright"), &pCopyrightNode)) ) return true;

			CComPtr<IXMLDOMElement> pCopyrightElement;
			if( FAILED(pCopyrightNode.QueryInterface(&pCopyrightElement)) ) return false;

			CComBSTR bstrCopyright;
			if( SUCCEEDED(pCopyrightElement->get_text(&bstrCopyright)) )
				imageData.strCopyright = bstrCopyright;

			CComPtr<IXMLDOMNode> pCopyrightLinkNode;
			if( FAILED(pBingRoot->selectSingleNode(CComBSTR(L"/images/image/copyrightlink"), &pCopyrightLinkNode)) ) return true;

			CComPtr<IXMLDOMElement> pCopyrightLinkElement;
			if( FAILED(pCopyrightLinkNode.QueryInterface(&pCopyrightLinkElement)) ) return false;

			CComBSTR bstrCopyrightLink;
			if( SUCCEEDED(pCopyrightLinkElement->get_text(&bstrCopyrightLink)) )
				imageData.strCopyrightLink = bstrCopyrightLink;
		}
	}
	catch( std::exception& )
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ImageHandler::LoadImage(std::shared_ptr<BackgroundImage>& bkImage)
{
	CriticalSectionLock lock(bkImage->updateCritSec);

	if (!bkImage) return false;

		// if we're reloading, delete old bitmap and DC
	if (!bkImage->dcImage.IsNull())
	{
		bkImage->dcImage.SelectBitmap(NULL);
		bkImage->dcImage.DeleteDC();
	}

	if (!bkImage->image.IsNull()) bkImage->image.DeleteObject();

	// create new original image
	bkImage->originalImage.reset(new internalImage());

	// load background image
	if (!bkImage->originalImage->loadU(Helpers::ExpandEnvironmentStrings(bkImage->imageData.strFilename).c_str()))
	{
		bkImage->originalImage.reset();
		return false;
	}

	bkImage->dwOriginalImageWidth	= bkImage->originalImage->getWidth();
	bkImage->dwOriginalImageHeight	= bkImage->originalImage->getHeight();

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

bool ImageHandler::LoadImageFromContent(std::shared_ptr<BackgroundImage>& bkImage, const std::vector<char>& content)
{
	CriticalSectionLock lock(bkImage->updateCritSec);

	if (!bkImage) return false;

		// if we're reloading, delete old bitmap and DC
	if (!bkImage->dcImage.IsNull())
	{
		bkImage->dcImage.SelectBitmap(NULL);
		bkImage->dcImage.DeleteDC();
	}

	if (!bkImage->image.IsNull()) bkImage->image.DeleteObject();

	// create new original image
	bkImage->originalImage.reset(new internalImage());

	// load background image
	if (!bkImage->originalImage->loadFromMemory(reinterpret_cast<const BYTE*>(content.data()), static_cast<DWORD>(content.size())))
	{
		bkImage->originalImage.reset();
		return false;
	}

	bkImage->dwOriginalImageWidth	= bkImage->originalImage->getWidth();
	bkImage->dwOriginalImageHeight	= bkImage->originalImage->getHeight();

	return true;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::PaintRelativeImage(const CDC& dc, CBitmap&	bmpTemplate, std::shared_ptr<BackgroundImage>& bkImage, DWORD& dwDisplayWidth, DWORD& dwDisplayHeight)
{
  // set template bitmap dimensions
  DWORD	dwTemplateWidth  = bkImage->originalImage->getWidth();
  DWORD	dwTemplateHeight = bkImage->originalImage->getHeight();

  if (bkImage->imageData.imagePosition == imagePositionStretch ||
      bkImage->imageData.imagePosition == imagePositionFit     ||
      bkImage->imageData.imagePosition == imagePositionFill)
  {
    if (bkImage->imageData.bExtend)
    {
      dwTemplateWidth  = bkImage->dwImageWidth;
      dwTemplateHeight = bkImage->dwImageHeight;
    }
    else
    {
      dwTemplateWidth  = dwDisplayWidth;
      dwTemplateHeight = dwDisplayHeight;
    }
  }

  DWORD dwNewWidth  = dwTemplateWidth;
  DWORD dwNewHeight = dwTemplateHeight;

  if ( bkImage->originalImage->getWidth()  != dwNewWidth ||
       bkImage->originalImage->getHeight() != dwNewHeight )
  {
    // resize background image
    ImageHandler::CalcRescale(dwNewWidth, dwNewHeight, bkImage);
    internalImage tempImage(*(bkImage->originalImage));

#ifdef _DEBUG
    DWORD dwGetTickCount = ::GetTickCount();
#endif
    tempImage.rescale(dwNewWidth, dwNewHeight);
#ifdef _DEBUG
    TRACE(L"rescale in %lu ms\n", ::GetTickCount() - dwGetTickCount);
#endif

		tempImage.CreateDIBitmap(dc, bmpTemplate);
  }
  else
  {
		bkImage->originalImage->CreateDIBitmap(dc, bmpTemplate);
  }

  dwDisplayWidth  = dwNewWidth;
  dwDisplayHeight = dwNewHeight;
}

void ImageHandler::CreateRelativeImage(const CDC& dc, std::shared_ptr<BackgroundImage>& bkImage)
{
  CriticalSectionLock	lock(bkImage->updateCritSec);

  bkImage->dwImageWidth	= ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
  bkImage->dwImageHeight	= ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

  // create background DC
	if( bkImage->dcImage.IsNull() ) bkImage->dcImage.CreateCompatibleDC(NULL);

  // create background bitmap
	if( !bkImage->image.IsNull() ) bkImage->image.DeleteObject();
  Helpers::CreateBitmap(dc, bkImage->dwImageWidth, bkImage->dwImageHeight, bkImage->image);
  bkImage->dcImage.SelectBitmap(bkImage->image);

  // paint background
  CBrush	backgroundBrush(::CreateSolidBrush(bkImage->imageData.crBackground));
  CRect	rect(0, 0, bkImage->dwImageWidth, bkImage->dwImageHeight);
  bkImage->dcImage.FillRect(&rect, backgroundBrush);

  // this can be false for desktop backgrounds with no wallpaper image
  if (bkImage->originalImage.get() != NULL)
  {
    if ( bkImage->bWallpaper && ImageHandler::IsWin8()           // Win8 wallpaper
      && !bkImage->imageData.bExtend                             // not extended
      && bkImage->imageData.imagePosition != imagePositionTile ) // and not tiled
    {
      // Windows 8 wallpaper (Stretch, Fit & Fill) is handled separately for each monitor
      MonitorEnumData enumData(dc, bkImage);
#if _WIN32_WINNT >= 0x0602
      ImageHandler::LoadDesktopWallpaperWin8(&enumData);
#else
      ::EnumDisplayMonitors(NULL, NULL, ImageHandler::MonitorEnumProcWin8, reinterpret_cast<LPARAM>(&enumData));
#endif
    }
    else
    {
      // create template image
      CDC     dcTemplate;
      CBitmap	bmpTemplate;
      dcTemplate.CreateCompatibleDC(NULL);

      // Windows 7 or older wallpaper (Stretch, Fit & Fill) use the primary monitor to rescale picture for each monitor
      DWORD dwNewWidth  = ::GetSystemMetrics(SM_CXSCREEN);
      DWORD dwNewHeight = ::GetSystemMetrics(SM_CYSCREEN);
      PaintRelativeImage(dc, bmpTemplate, bkImage, dwNewWidth, dwNewHeight);

      dcTemplate.SelectBitmap(bmpTemplate);

      if (bkImage->imageData.imagePosition == imagePositionTile)
      {
        ImageHandler::TileTemplateImage(
          dcTemplate,
          ImageHandler::IsWin8() ? 0 : ::GetSystemMetrics(SM_XVIRTUALSCREEN), // Windows 8 wallpaper tiles starts
          ImageHandler::IsWin8() ? 0 : ::GetSystemMetrics(SM_YVIRTUALSCREEN), // in the top left corner of virtual screen
          bkImage);
      }
      else if (bkImage->imageData.bExtend)
      {
        ImageHandler::PaintTemplateImage(
          dcTemplate,
          0,
          0,
          dwNewWidth,
          dwNewHeight,
          bkImage->dwImageWidth,
          bkImage->dwImageHeight,
          bkImage);
      }
      else
      {
        MonitorEnumData	enumData(dcTemplate, bkImage);
        ::EnumDisplayMonitors(NULL, NULL, ImageHandler::MonitorEnumProc, reinterpret_cast<LPARAM>(&enumData));
      }
    }
  }

  if (bkImage->imageData.byTintOpacity > 0) TintImage(dc, bkImage);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::CreateImage(const CDC& dc, const CRect& clientRect, std::shared_ptr<BackgroundImage>& bkImage)
{
	CriticalSectionLock	lock(bkImage->updateCritSec);

	bkImage->dwImageWidth = clientRect.Width();
	bkImage->dwImageHeight= clientRect.Height();

	// create background DC
	if (bkImage->dcImage.IsNull()) bkImage->dcImage.CreateCompatibleDC(NULL);

	// create background bitmap
	if (!bkImage->image.IsNull()) bkImage->image.DeleteObject();
	Helpers::CreateBitmap(dc, bkImage->dwImageWidth, bkImage->dwImageHeight, bkImage->image);
	bkImage->dcImage.SelectBitmap(bkImage->image);

	// paint background
	CBrush backgroundBrush(::CreateSolidBrush(bkImage->imageData.crBackground));
	bkImage->dcImage.FillRect(&clientRect, backgroundBrush);

	// this can be false only for desktop backgrounds with no wallpaper image
	if (bkImage->originalImage.get() != NULL)
	{
		// create template image
		CDC		dcTemplate;
		CBitmap	bmpTemplate;

		dcTemplate.CreateCompatibleDC(NULL);

    DWORD dwNewWidth  = bkImage->dwImageWidth;
    DWORD dwNewHeight = bkImage->dwImageHeight;

		if ( (bkImage->imageData.imagePosition == imagePositionStretch ||
          bkImage->imageData.imagePosition == imagePositionFit     ||
          bkImage->imageData.imagePosition == imagePositionFill)
         &&
         (bkImage->originalImage->getWidth()  != dwNewWidth        ||
          bkImage->originalImage->getHeight() != dwNewHeight) )
		{
			// resize background image
			ImageHandler::CalcRescale(dwNewWidth, dwNewHeight, bkImage);
			internalImage tempImage(*(bkImage->originalImage));
			tempImage.rescale(dwNewWidth, dwNewHeight);

			tempImage.CreateDIBitmap(dc, bmpTemplate);
		}
		else
		{
			dwNewWidth  = bkImage->dwOriginalImageWidth;
			dwNewHeight = bkImage->dwOriginalImageHeight;

			bkImage->originalImage->CreateDIBitmap(dc, bmpTemplate);
		}

		dcTemplate.SelectBitmap(bmpTemplate);

		if (bkImage->imageData.imagePosition == imagePositionTile)
		{
			TileTemplateImage(
				dcTemplate,
				0,
				0,
				bkImage);
		}
		else
		{
			PaintTemplateImage(
				dcTemplate,
				0,
				0,
				dwNewWidth,
				dwNewHeight,
				bkImage->dwImageWidth,
				bkImage->dwImageHeight,
				bkImage);
		}
	}

	if (bkImage->imageData.byTintOpacity > 0) TintImage(dc, bkImage);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::PaintTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, DWORD dwSrcWidth, DWORD dwSrcHeight, DWORD dwDstWidth, DWORD dwDstHeight, std::shared_ptr<BackgroundImage>& bkImage)
{
	if( bkImage->imageData.imagePosition == imagePositionFill && (!bkImage->imageData.bExtend) && ImageHandler::IsWin8() )
	{
		// Windows 8 filled wallpaper:
		// when image height is greater than screen height
		// top is not shifted with half but 1/3
		bkImage->dcImage.BitBlt(
					(dwDstWidth <= dwSrcWidth) ? nOffsetX : nOffsetX + (dwDstWidth - dwSrcWidth)/2,
					(dwDstHeight <= dwSrcHeight) ? nOffsetY : nOffsetY + (dwDstHeight - dwSrcHeight)/3,
					dwDstWidth, 
					dwDstHeight,
					dcTemplate,
					(dwDstWidth < dwSrcWidth) ? (dwSrcWidth - dwDstWidth)/2 : 0,
					(dwDstHeight < dwSrcHeight) ? (dwSrcHeight - dwDstHeight)/3 : 0,
					SRCCOPY);
  }
  else
	{
		bkImage->dcImage.BitBlt(
					(dwDstWidth <= dwSrcWidth) ? nOffsetX : nOffsetX + (dwDstWidth - dwSrcWidth)/2,
					(dwDstHeight <= dwSrcHeight) ? nOffsetY : nOffsetY + (dwDstHeight - dwSrcHeight)/2,
					dwDstWidth, 
					dwDstHeight,
					dcTemplate,
					(dwDstWidth < dwSrcWidth) ? (dwSrcWidth - dwDstWidth)/2 : 0,
					(dwDstHeight < dwSrcHeight) ? (dwSrcHeight - dwDstHeight)/2 : 0,
					SRCCOPY);
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::TileTemplateImage(const CDC& dcTemplate, int nOffsetX, int nOffsetY, std::shared_ptr<BackgroundImage>& bkImage)
{
	// we're tiling the image, starting at coordinates (0, 0)
	DWORD dwY = 0;
	DWORD dwImageOffsetY = bkImage->originalImage->getHeight() + (nOffsetY - (int)bkImage->originalImage->getHeight()*(nOffsetY/(int)bkImage->originalImage->getHeight()));

	while (dwY < bkImage->dwImageHeight)
	{
		DWORD dwX				= 0;
		DWORD dwImageOffsetX	= bkImage->originalImage->getWidth() + (nOffsetX - (int)bkImage->originalImage->getWidth()*(nOffsetX/(int)bkImage->originalImage->getWidth()));

		while (dwX < bkImage->dwImageWidth)
		{
			bkImage->dcImage.BitBlt(
						dwX, 
						dwY, 
						bkImage->originalImage->getWidth(), 
						bkImage->originalImage->getHeight(),
						dcTemplate,
						dwImageOffsetX,
						dwImageOffsetY,
						SRCCOPY);

			dwX += bkImage->originalImage->getWidth() - dwImageOffsetX;
			dwImageOffsetX = 0;
		}

		dwY += bkImage->originalImage->getHeight() - dwImageOffsetY;
		dwImageOffsetY = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

void ImageHandler::TintImage(const CDC& dc, std::shared_ptr<BackgroundImage>& bkImage)
{
	CDC				dcTint;
	CBitmap			bmpTint;
	CBrush			tintBrush(::CreateSolidBrush(bkImage->imageData.crTint));
	BLENDFUNCTION	bf;
	CRect			rect(0, 0, bkImage->dwImageWidth, bkImage->dwImageHeight);

	bf.BlendOp				= AC_SRC_OVER;
	bf.BlendFlags			= 0;
	bf.SourceConstantAlpha	= bkImage->imageData.byTintOpacity;
	bf.AlphaFormat			= 0;

	if (dcTint.CreateCompatibleDC(dc) == NULL)
	{
		// this can sometimes fail with 'not enough storage available' error
		// when resizing the window fast
		return;
	}
//	bmpTint.CreateCompatibleBitmap(dc, bkImage->dwImageWidth, bkImage->dwImageHeight);
	Helpers::CreateBitmap(dc, bkImage->dwImageWidth, bkImage->dwImageHeight, bmpTint);
	dcTint.SelectBitmap(bmpTint);
	dcTint.FillRect(&rect, tintBrush);

	bkImage->dcImage.AlphaBlend(
						0,
						0,
						bkImage->dwImageWidth,
						bkImage->dwImageHeight,
						dcTint,
						0,
						0,
						bkImage->dwImageWidth,
						bkImage->dwImageHeight,
						bf);
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK ImageHandler::MonitorEnumProc(HMONITOR /*hMonitor*/, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData)
{
	MonitorEnumData* pEnumData = reinterpret_cast<MonitorEnumData*>(lpData);

	CRect rectMonitor(lprcMonitor);

	DWORD dwNewWidth  = ::GetSystemMetrics(SM_CXSCREEN);
	DWORD dwNewHeight = ::GetSystemMetrics(SM_CYSCREEN);

	ImageHandler::CalcRescale(dwNewWidth, dwNewHeight, pEnumData->bkImage);

	ImageHandler::PaintTemplateImage(
					pEnumData->dcTemplate, 
					rectMonitor.left - ::GetSystemMetrics(SM_XVIRTUALSCREEN), 
					rectMonitor.top  - ::GetSystemMetrics(SM_YVIRTUALSCREEN), 
					dwNewWidth,
					dwNewHeight,
					rectMonitor.Width(),
					rectMonitor.Height(),
					pEnumData->bkImage);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#if _WIN32_WINNT >= 0x0602
void ImageHandler::LoadDesktopWallpaperWin8(MonitorEnumData* pEnumData)
{
	CComPtr<IDesktopWallpaper> desktopWallpaper;
	HRESULT hr = desktopWallpaper.CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL);
	if(FAILED(hr)) return;

	UINT count;
	hr = desktopWallpaper->GetMonitorDevicePathCount(&count);
	if(FAILED(hr)) return;

	for(UINT i = 0; i < count; i++)
	{
		// Get the device path for the monitor.
		CComHeapPtr<wchar_t> spszId;
		hr = desktopWallpaper->GetMonitorDevicePathAt(i, &spszId);
		if(FAILED(hr)) continue;

		// Get the monitor location.
		CRect rectMonitor;
		hr = desktopWallpaper->GetMonitorRECT(spszId, (LPRECT)rectMonitor);
		if(FAILED(hr)) continue;

		// Get the wallpaper on that monitor.
		CComHeapPtr<wchar_t> spszWallpaper;
		hr = desktopWallpaper->GetWallpaper(spszId, &spszWallpaper);
		if(FAILED(hr)) continue;

		TRACE(
			L"wallpaper picture on '%s' (%ix%i)-(%ix%i) is '%s'\n",
			static_cast<wchar_t*>(spszId),
			rectMonitor.left, rectMonitor.top,
			rectMonitor.right, rectMonitor.bottom,
			static_cast<wchar_t*>(spszWallpaper));

		std::shared_ptr<BackgroundImage> bkImage;

		if(spszWallpaper.m_pData && *spszWallpaper.m_pData)
		{
			std::filesystem::path imagePath(spszWallpaper.m_pData);

			// c:\dir1\dir2\dir3\photo.jpg
			// we check if following files exist (in that order)
			// c:\dir1\dir2\dir3.ConsoleZ\photo.jpg
			// c:\dir1\dir2.ConsoleZ\dir3\photo.jpg
			// c:\dir1.ConsoleZ\dir2\dir3\photo.jpg
			auto iter = imagePath.end();
			if( iter != imagePath.begin() ) --iter;
			if( iter != imagePath.begin() ) --iter;
			for(; iter != imagePath.end() && iter != imagePath.begin() && !iter->has_root_path(); --iter )
			{
				std::filesystem::path imagePath2;
				for( auto iter2 = imagePath.begin(); iter2 != imagePath.end(); ++iter2 )
				{
					imagePath2 += *iter2;
					if( iter2 == iter ) imagePath2.concat(L".ConsoleZ");
				}
				if( std::filesystem::exists(imagePath2) )
				{
					imagePath.swap(imagePath2);
					break;
				}
			}

			bkImage.reset(new BackgroundImage (pEnumData->bkImage->imageData));
			bkImage->imageData.strFilename = imagePath;
			bkImage->bWallpaper = true;
			ImageHandler::LoadImageW(bkImage);
		}
		else
		{
			bkImage = pEnumData->bkImage;
		}

		if( bkImage->originalImage.get() )
		{
			// create template image
			CDC     dcTemplate;
			CBitmap	bmpTemplate;
			dcTemplate.CreateCompatibleDC(NULL);

			DWORD dwNewWidth = rectMonitor.Width();
			DWORD dwNewHeight = rectMonitor.Height();
			ImageHandler::PaintRelativeImage(pEnumData->dcTemplate, bmpTemplate, bkImage, dwNewWidth, dwNewHeight);

			dcTemplate.SelectBitmap(bmpTemplate);

			ImageHandler::PaintTemplateImage(
				dcTemplate,
				rectMonitor.left - ::GetSystemMetrics(SM_XVIRTUALSCREEN),
				rectMonitor.top - ::GetSystemMetrics(SM_YVIRTUALSCREEN),
				dwNewWidth,
				dwNewHeight,
				rectMonitor.Width(),
				rectMonitor.Height(),
				pEnumData->bkImage);
		}
	}
}
#endif

BOOL CALLBACK ImageHandler::MonitorEnumProcWin8(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM lpData)
{
  MONITORINFOEX miex;
  miex.cbSize = sizeof(miex);
  ::GetMonitorInfo(hMonitor, &miex);

  DISPLAY_DEVICE dd;
  dd.cb = sizeof(dd);
  ::EnumDisplayDevices(miex.szDevice, 0, &dd, EDD_GET_DEVICE_INTERFACE_NAME);

#if 0
  TRACE(L"name %s String %s DeviceID %s Key %s StateFlags %lu\n",
    dd.DeviceName,
    dd.DeviceString,
    dd.DeviceID,
    dd.DeviceKey,
    dd.StateFlags);
#endif

  wchar_t szTranscodedImage [_MAX_PATH] = L"";

  TRACE(
    L"searching for %s ('%s') ...\n",
    miex.szDevice,
    dd.DeviceID);

  try
  {
    DWORD dwTranscodedImageCount = 0;
    DWORD dwType;
    DWORD dwValueSize;
    LSTATUS rc;

    HKEY hkey;
    rc = ::RegOpenKeyEx(
      HKEY_CURRENT_USER,
      L"Control Panel\\Desktop",
      0,
      KEY_READ,
      &hkey);

    if( rc != ERROR_SUCCESS )
      Win32Exception::Throw("RegOpenKeyEx", rc);

    unique_ptr<HKEY__, RegCloseKeyHelper>hkeyPtr(hkey);

    rc = ::RegQueryValueEx(
      hkeyPtr.get(),
      L"TranscodedImageCount",
      nullptr,
      &dwType,
      reinterpret_cast<LPBYTE>(&dwTranscodedImageCount),
      &(dwValueSize = static_cast<DWORD>(sizeof(dwTranscodedImageCount))));

    if( rc != ERROR_SUCCESS )
      Win32Exception::Throw("RegQueryValueEx", rc);

    for(DWORD i = 0; i < dwTranscodedImageCount; ++i)
    {
      wchar_t szValueName [32];
      _snwprintf_s(
        szValueName, ARRAYSIZE(szValueName),
        _TRUNCATE,
        L"TranscodedImageCache_%03lu",
        i);

      BYTE binTranscodedImageCache [0x320];
      rc = ::RegQueryValueEx(
        hkeyPtr.get(),
        szValueName,
        nullptr,
        &dwType,
        binTranscodedImageCache,
        &(dwValueSize = static_cast<DWORD>(sizeof(binTranscodedImageCache))));

      if( rc != ERROR_SUCCESS )
        Win32Exception::Throw("RegQueryValueEx", rc);

      TRACE(
        L"\twallpaper picture on ('%s') is '%s'\n",
        reinterpret_cast<wchar_t*>(binTranscodedImageCache + 0x220),
        reinterpret_cast<wchar_t*>(binTranscodedImageCache + 0x18));

      wchar_t* szDeviceID1 = dd.DeviceID;
      wchar_t* szDeviceID2 = reinterpret_cast<wchar_t*>(binTranscodedImageCache + 0x220);

      // when monitor device id is "Default_Monitor"
      // \\?\DISPLAY#Default_Monitor#...
      // there is no device id in TranscodedImageCache_xxx file

      if( ( *szDeviceID2 == 0 && 
            wcsncmp(szDeviceID1, L"\\\\?\\DISPLAY#Default_Monitor#", 28) == 0 ) ||
          wcsncmp(szDeviceID1, szDeviceID2, 128) == 0 )
      {
        wcsncpy_s(
          szTranscodedImage, _MAX_PATH,
          reinterpret_cast<wchar_t*>(binTranscodedImageCache + 0x18),
          _TRUNCATE);

        break;
      }
    }
  }
  catch(Win32Exception&)
  {
  }

  MonitorEnumData* pEnumData = reinterpret_cast<MonitorEnumData*>(lpData);

  std::shared_ptr<BackgroundImage> bkImage;

  if( szTranscodedImage[0] )
  {
    TRACE(
      L"wallpaper picture on %s ('%s') is '%s'\n",
      miex.szDevice,
      dd.DeviceID,
      szTranscodedImage);

    bkImage.reset(new BackgroundImage (pEnumData->bkImage->imageData));
    bkImage->imageData.strFilename = szTranscodedImage;
    bkImage->bWallpaper = true;
    ImageHandler::LoadImageW(bkImage);
  }
  else
  {
    bkImage = pEnumData->bkImage;
  }

	if( bkImage.get() )
	{
		CRect   rectMonitor(lprcMonitor);

		// create template image
		CDC     dcTemplate;
		CBitmap	bmpTemplate;
		dcTemplate.CreateCompatibleDC(NULL);

		DWORD dwNewWidth = rectMonitor.Width();
		DWORD dwNewHeight = rectMonitor.Height();
		ImageHandler::PaintRelativeImage(pEnumData->dcTemplate, bmpTemplate, bkImage, dwNewWidth, dwNewHeight);

		dcTemplate.SelectBitmap(bmpTemplate);

		ImageHandler::PaintTemplateImage(
			dcTemplate,
			rectMonitor.left - ::GetSystemMetrics(SM_XVIRTUALSCREEN),
			rectMonitor.top - ::GetSystemMetrics(SM_YVIRTUALSCREEN),
			dwNewWidth,
			dwNewHeight,
			rectMonitor.Width(),
			rectMonitor.Height(),
			pEnumData->bkImage);
	}

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
