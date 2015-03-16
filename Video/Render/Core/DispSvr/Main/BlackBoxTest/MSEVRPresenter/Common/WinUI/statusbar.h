//------------------------------------------------------------------------------
//
// File: statusbar.h
// Thin wrapper around the status bar control.
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

#include "wincontrol.h"

class StatusBar : public Control
{
public:

    //------------------------------------------------------------------------------
    // Create
    //
    // Creates a new instance of the status bar control.
    //------------------------------------------------------------------------------

    HRESULT Create(HWND hParent, int nID)
    {
        CREATESTRUCT create;
        ZeroMemory(&create, sizeof(CREATESTRUCT));

        // NOTE: A status bar sets its own width and height.

        create.hwndParent = hParent;
        create.hMenu = (HMENU)(INT_PTR)nID;
        create.lpszClass = STATUSCLASSNAME;
        create.style = WS_CHILD | WS_VISIBLE;
        return Control::Create(create);
    }

    //------------------------------------------------------------------------------
    // SetParts
    //
    // Sets the number of parts and their widths.
    //
    // nParts:  Number of parts.
    // pWidths: Pointer to an array of widths of size nParts.
    //
    // (See the documentation for SB_SETPARTS for more information.)
    //------------------------------------------------------------------------------

    BOOL SetParts(int nParts, int *pWidths)
    {
        return (BOOL)SendMessage(SB_SETPARTS, (WPARAM)nParts, (LPARAM)pWidths);
    }


    //------------------------------------------------------------------------------
    // GetNumParts
    //
    // Returns the number of parts.
    //------------------------------------------------------------------------------

    int GetNumParts()
    {
        return (int)SendMessage(SB_GETPARTS, 0, 0);
    }


    //------------------------------------------------------------------------------
    // GetPartWidths
    //
    // Returns an array of part widths.
    //
    // nParts:  Size of the array.
    // pWidths: Pointer to an array of size nParts, which receives the widths.
    //------------------------------------------------------------------------------

    int GetPartWidths(int nParts, int *pWidths)
    {
        return (int)SendMessage(SB_GETPARTS, (WPARAM)nParts, (LPARAM)pWidths);
    }


    //------------------------------------------------------------------------------
    // SetText
    //
    // Sets the text for a specified part.
    //------------------------------------------------------------------------------

    BOOL SetText(int iPart, const TCHAR* szText, BOOL bNoBorders = FALSE, BOOL bPopOut = FALSE)
    {
        UINT flags = 0;
        if (bNoBorders) 
        { 
            flags |= SBT_NOBORDERS;
        }
        if (bPopOut)
        {
            flags |= SBT_POPOUT;
        }

        return (BOOL)SendMessage(SB_SETTEXT, (WPARAM)(iPart | flags), (LPARAM)szText);
    }
};