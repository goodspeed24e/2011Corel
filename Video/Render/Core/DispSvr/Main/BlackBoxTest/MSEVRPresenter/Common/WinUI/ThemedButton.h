//------------------------------------------------------------------------------
//
// File: ThemedButton.h
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

#pragma once

#include <uxtheme.h>
#include <vsstyle.h> // Defines PBS_xxxx

#pragma comment(lib, "UxTheme")
#pragma comment(lib, "Msimg32")		// links AlphaDrawBitmap()

const DWORD NUM_THEME_STATES = 6;

//------------------------------------------------------------------------------
// ThemeHandle class
// Thin wrapper around a theme handle
//------------------------------------------------------------------------------

class ThemeHandle
{
private:
    HTHEME  m_hTheme;   // Handle to the current theme.
    BOOL    m_bActive;  // Are themes active?

    // Copy and assignment are private. 
    // (Copying theme handles is not required anywhere in this sample.)
    ThemeHandle(const ThemeHandle&);
    ThemeHandle& operator=(const ThemeHandle&);

public:

    ThemeHandle(): m_hTheme(NULL), m_bActive(TRUE) 
    {
    }

    ~ThemeHandle();

    operator HTHEME () const
    {
        return m_hTheme;
    }

    void Open(HWND hwnd, const WCHAR *pszClassList);
    void Close();
    void Reset(HWND hwnd, const WCHAR *pszClassList);
};


//------------------------------------------------------------------------------
// BitmapStripInfo
// Holds information about one image in a bitmap strip.
//------------------------------------------------------------------------------

struct BitmapStripInfo
{
    Rect rc;            // Rectangle within the bitmap, containing the image.
    HBITMAP hBitmap;    // Handle to the bitmap.
};


//------------------------------------------------------------------------------
// BitmapStrip class
// Holds a bitmap that contains a strip of images.
//
// This class is used to manage the various images the themed button draws
// in different states.
//
// Possibly the standard image-list control could be used instead, but this 
// class is more straightforward for the requirements of this sample.
//------------------------------------------------------------------------------

class BitmapStrip
{
protected:
    HBITMAP m_hBitmap;      // Handle to the bitmap that contains the images.
    Size    m_size;         // Size of each image in the strip.
    UINT    m_cImages;      // Number of images. (m_size.cx * m_cImages <= width of bitmap)
public:
    BitmapStrip(): m_hBitmap(NULL), m_cImages(0)
    {
    }

    ~BitmapStrip();

    BOOL    Load(int nID, int cImages);
    UINT    NumImages() const { return m_cImages; }
    BOOL    GetImage(UINT i, BitmapStripInfo *pInfo);
};


//------------------------------------------------------------------------------
// ThemedButton class
// Implements a theme-aware button.
//------------------------------------------------------------------------------

class ThemedButton : public Button
{
protected:
    ThemeHandle  m_hTheme;
    BitmapStrip  m_bitmap;
    DWORD        m_stateMap[ NUM_THEME_STATES ];  // Maps button states to images
    BOOL         m_bDrawBackground;
public:
    ThemedButton();
    void    ResetTheme();
    LRESULT Draw(const NMCUSTOMDRAW *pDraw);
    BOOL    LoadBitmap(int nID, int cImages);

    // DrawBackground: Specifies whether to draw the theme background.
    // (Defaults to TRUE)
    void    DrawBackground(BOOL bDraw)
    {
        m_bDrawBackground = bDraw;
    }

    //  SetButtonImage: Set the image for a specified button state.
    //
    //  iState = PBS_xxx value, or (UINT)-1 to apply this image to all states.
    //  iImageIndex = index from the image strip
    BOOL    SetButtonImage(UINT iState, UINT iImageIndex);
};