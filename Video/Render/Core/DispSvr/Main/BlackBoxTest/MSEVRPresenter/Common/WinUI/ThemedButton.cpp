//------------------------------------------------------------------------------
//
// File: ThemedButton.cpp
// Description: Implements a theme-aware button control.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#include "wincontrol.h"
#include "button.h"
#include "ThemedButton.h"

BOOL AlphaDrawBitmap(HDC hdc, const Rect& rcDest, BitmapStrip& bitmap, UINT index);

//------------------------------------------------------------------------------
// ThemeHandle destructor
//------------------------------------------------------------------------------

ThemeHandle::~ThemeHandle()
{
    Close();
}


//------------------------------------------------------------------------------
// Open
// Opens a theme handle for a window (and associated theme class)
//
// For buttons, the theme class is "Button"
//------------------------------------------------------------------------------

void ThemeHandle::Open(HWND hwnd, const WCHAR *pszClassList)
{
    if (m_hTheme == NULL && m_bActive)
    {
        if (IsThemeActive())
        {
            m_hTheme = OpenThemeData(hwnd, pszClassList);
        }
        else
        {
            m_bActive = FALSE;
        }
    }
}

//------------------------------------------------------------------------------
// Close
// Closes the theme handle.
//------------------------------------------------------------------------------

void ThemeHandle::Close()
{
    if (m_hTheme)
    {
        CloseThemeData(m_hTheme);
        m_hTheme = NULL;
    }
}

//------------------------------------------------------------------------------
// Reset
// Re-opens the theme handle. 
//
// Call this method in response to WM_THEMECHANGED messages.
//------------------------------------------------------------------------------

void ThemeHandle::Reset(HWND hwnd, const WCHAR *pszClassList)
{
    Close();
    m_bActive = TRUE;
    Open(hwnd, pszClassList);
}

/////////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------------
// BitmapStrip destructor
//------------------------------------------------------------------------------

BitmapStrip::~BitmapStrip()
{
    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
    }
}

//------------------------------------------------------------------------------
// Load 
// Loads the bitmap that contains the image strip.
// 
// nID: Resource ID of the bitmap
// cImages: Count of images in the bitmap.
//------------------------------------------------------------------------------

BOOL BitmapStrip::Load(int nID, int cImages)
{
    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
    }
    m_cImages = 0;

    m_hBitmap = LoadBitmap(GetInstance(), MAKEINTRESOURCE(nID));
    if (m_hBitmap == NULL)
    {
        return FALSE;
    }

    BITMAP bm;
    ZeroMemory(&bm, sizeof(bm));
    GetObject(m_hBitmap, sizeof(BITMAP), &bm);

    // Calculate the size of each image.
    if (cImages == 0)
    {
        cImages = 1;
    }

    m_size.cx = bm.bmWidth / cImages;
    m_size.cy = bm.bmHeight;
    m_cImages = cImages;

    return TRUE;
}

//------------------------------------------------------------------------------
// GetImage
//
// Returns the following information for one image in the list:
// - Handle to the bitmap
// - Subrectangle within the bitmap that contains the image.
//
// i: Index of the image to get information about.
//------------------------------------------------------------------------------

BOOL BitmapStrip::GetImage(UINT i, BitmapStripInfo *pInfo)
{
    if (i >= m_cImages)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pInfo->rc.Set( m_size.cx * i, 0, m_size.cx * (i + 1), m_size.cy );
    pInfo->hBitmap = m_hBitmap;
    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------------
// ThemedButton constructor
//------------------------------------------------------------------------------

ThemedButton::ThemedButton() : m_bDrawBackground(TRUE)
{
    ZeroMemory(&m_stateMap, sizeof(m_stateMap));
}

//------------------------------------------------------------------------------
// ResetTheme
// Resets the theme. 
// 
// Call this method in response to WM_THEMECHANGED messages.
//------------------------------------------------------------------------------

void ThemedButton::ResetTheme()
{
    m_hTheme.Reset(Window(), L"Button");
}

//------------------------------------------------------------------------------
// LoadBitmap
// Loads the bitmap that contains the image strip.
//
// nID: Resource ID of the bitmap
// cImages: Count of images in the bitmap.
//------------------------------------------------------------------------------

BOOL ThemedButton::LoadBitmap(int nID, int cImages)
{
    // Clear the mapping of states to images.
    ZeroMemory(&m_stateMap, sizeof(m_stateMap));

    BOOL bResult = m_bitmap.Load(nID, cImages);

    if (bResult)
    {
        // By default, assume that the image strip is ordered by button state.
        // In other words, for button_state == i, use image[i]
        DWORD count = min(cImages,NUM_THEME_STATES);
        for (DWORD i = 0; i < count; i++)
        {
            m_stateMap[i] = i;
        }
    }
    return bResult;
}

//------------------------------------------------------------------------------
// SetButtonImage
// Associate button state "iState" with image "iImageIndex"
//
// In other words, when the button state equals iState, the button draws the
// image located at index = iImageIndex in the image strip.
//
// The default ordering is iState == iImageIndex (see LoadBitmap)
//------------------------------------------------------------------------------

BOOL ThemedButton::SetButtonImage(UINT iState, UINT iImageIndex)
{
    if (iImageIndex >= m_bitmap.NumImages())
    {
        return FALSE;
    }

    if (iState == (UINT)-1)
    {
        for (DWORD i = 0; i < NUM_THEME_STATES; i++)
        {
            m_stateMap[i] = iImageIndex;
        }
        return TRUE;
    }

    if (iState < 1 || iState > NUM_THEME_STATES)
    {
        return FALSE;
    }

    m_stateMap[iState] = iImageIndex;
    return TRUE;
}

//------------------------------------------------------------------------------
// Draw
// Draws the themed button. 
// 
// Call this method in response to NM_CUSTOMDRAW messages.
//------------------------------------------------------------------------------

LRESULT ThemedButton::Draw(const NMCUSTOMDRAW *pDraw)
{
    m_hTheme.Open(Window(), L"Button");

    int iPartId = BP_PUSHBUTTON;
    int iStateId = PBS_NORMAL; 

    // Translate the button state into the theme state.
    if (HasStyle(WS_DISABLED))
    {
        iStateId = PBS_DISABLED;
    }
    else if (pDraw->uItemState & CDIS_SELECTED)
    {
        iStateId = PBS_PRESSED; // Button is pressed.
    }
    else if (pDraw->uItemState & CDIS_HOT)
    {
        iStateId = PBS_HOT;  // Button is hot but not pressed.
    }
    else if (HasStyle(BS_DEFPUSHBUTTON))
    {
        iStateId = PBS_DEFAULTED; // Default button, but not hot or pressed.
    }

    // Note: This class does not support PBS_DEFAULTED_ANIMATING.

    // Now draw the different parts.

    Rect rcClient;      // Entire client rect for the button.
    Rect rcContent;     // Content rect is a subrect of the client rect, when themed.

    GetClientRect(Window(), &rcClient);

    if ((HTHEME)m_hTheme != NULL)
    {
        // Themes are active. Draw the background using themes.

        if (!m_bDrawBackground || IsThemeBackgroundPartiallyTransparent(m_hTheme, iPartId, iStateId))
        {
            DrawThemeParentBackground(Window(), pDraw->hdc, &pDraw->rc);
        }

        if (m_bDrawBackground)
        {
            DrawThemeBackground(m_hTheme, pDraw->hdc, iPartId, iStateId, &pDraw->rc, &pDraw->rc);
        }

        // Find the content rect.
        GetThemeBackgroundContentRect(m_hTheme, pDraw->hdc, iPartId, iStateId, &rcClient, &rcContent);
    }
    else
    {
        // Themes are not active. Draw the normal button color.
        FillRect(pDraw->hdc, &rcClient, (HBRUSH)(COLOR_BTNFACE+1));

        rcContent = rcClient;
    }

    // Draw the bitmap image.
    AlphaDrawBitmap(pDraw->hdc, rcContent, m_bitmap, m_stateMap[iStateId]);

    // This return value tells Windows not to draw any part of the button.
    return CDRF_SKIPDEFAULT;
}


//------------------------------------------------------------------------------
// AlphaDrawBitmap
// Alpha blends an image from a BitmapStrip to a DC.
//
// hdc: Device context
// rcDest: Destination rectangle
// bitmap: Bitmap strip
// index:  Index of the image in the strip.
//------------------------------------------------------------------------------

BOOL AlphaDrawBitmap(HDC hdc, const Rect& rcDest, BitmapStrip& bitmap, UINT index)
{
    BitmapStripInfo info;

    BOOL bResult = bitmap.GetImage(index, &info);
    if (!bResult)
    {
        return FALSE;
    }

    HDC hdcMem = CreateCompatibleDC(hdc);
    if (hdcMem == NULL)
    {
        return FALSE;
    }

    SelectObject(hdcMem, info.hBitmap);

    BLENDFUNCTION BlendFn;
    ZeroMemory(&BlendFn, sizeof(&BlendFn));
    BlendFn.BlendOp = AC_SRC_OVER;
    BlendFn.SourceConstantAlpha = 0xFF;
    BlendFn.AlphaFormat = AC_SRC_ALPHA;

    LONG src_w = info.rc.Width();
    LONG src_h = info.rc.Height();
    LONG dest_w = rcDest.Width();
    LONG dest_h = rcDest.Height();

    LONG padX = max(0, dest_w - src_w) / 2;
    LONG padY = max(0, dest_h - src_h) / 2;

    LONG width = min(src_w, src_w);
    LONG height = min(dest_h, src_h);

    bResult = AlphaBlend(
      hdc,                  // handle to destination DC
      rcDest.left + padX,   // x-coord of upper-left corner
      rcDest.top + padY,    // y-coord of upper-left corner
      width,                // destination width
      height,               // destination height
      hdcMem,               // handle to source DC
      info.rc.left,         // x-coord of upper-left corner
      info.rc.top,          // y-coord of upper-left corner
      src_w,                // source width
      src_h,                // source height
      BlendFn               // alpha-blending function
    );
    
    DeleteDC(hdcMem);

    return TRUE;
}

