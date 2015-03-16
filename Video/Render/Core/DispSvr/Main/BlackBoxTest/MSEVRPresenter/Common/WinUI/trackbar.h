//-----------------------------------------------------------------------------
// File: trackbar.h
//
// Trackbar control class
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

class Trackbar : public Control
{
public:

    Trackbar()
    {
    }

    //-----------------------------------------------------------------------------
    // Name: Create
    // Description: Create a new instance of a trackbar control.
    //
    // hParent: Parent window
    // rcSize:  Bounding rectangle
    // dwStyle: Additional styles
    //-----------------------------------------------------------------------------
	HRESULT Create(HWND hParent, const Rect& rcSize, DWORD dwStyle = 0)
    {
        CreateStruct create;
        create.lpszClass = TRACKBAR_CLASS;
        create.SetBoundingRect(rcSize);
        create.hwndParent = hParent;
        create.style = dwStyle;

        return Control::Create(create);
    }

    //-----------------------------------------------------------------------------
    // Name: GetPosition
    // Description: Gets the current position of the trackbar.
    //-----------------------------------------------------------------------------
    LONG GetPosition() const
    {
        return (LONG)SendMessage(TBM_GETPOS, 0, 0);
    }

    //-----------------------------------------------------------------------------
    // Name: SetPosition
    // Description: Sets the position of the trackbar.
    //-----------------------------------------------------------------------------
    void SetPosition(LONG pos)
    {
        SendMessage(TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
    }

    //-----------------------------------------------------------------------------
    // Name: SetRange
    // Description: Sets the trackbar range.
    //-----------------------------------------------------------------------------
    void SetRange(LONG min, LONG max)
    {
        SendMessage(TBM_SETRANGEMIN, TRUE, min);
        SendMessage(TBM_SETRANGEMAX, TRUE, max);
    }

    //-----------------------------------------------------------------------------
    // Name: SetRangeAndPosition
    // Description: Sets the range and position.
    //-----------------------------------------------------------------------------
    void SetRangeAndPosition(LONG min, LONG max, LONG pos)
    {
        SetRange(min, max);
        SetPosition(pos);
    }

    //-----------------------------------------------------------------------------
    // Name: SetThumbLength
    // Description: Sets the size of the trackbar, in pixels.
    //-----------------------------------------------------------------------------
    void SetThumbLength(UINT iLen)
    {
        // To change the size of the trackbar, it must have the TBS_FIXEDLENGTH style.
        AddStyle(TBS_FIXEDLENGTH);
        SendMessage(TBM_SETTHUMBLENGTH, (WPARAM)iLen, 0);
    }

    //-----------------------------------------------------------------------------
    // Name: SetTickFrequency
    // Description: Sets the tick frequency.
    //-----------------------------------------------------------------------------
    void SetTickFrequency(WORD dwFreq)
    {
        // Add the AUTOTICKS style to display the ticks.
        AddStyle(TBS_AUTOTICKS);
        SendMessage(TBM_SETTICFREQ, (WPARAM)dwFreq, 0);
    }

    //-----------------------------------------------------------------------------
    // Name: InitTrackbar
    // Description: Initializes all of a trackbar's properties.
    //-----------------------------------------------------------------------------

    static void InitTrackbar(Trackbar& trackbar, HWND hwnd, LONG min, LONG max, WORD pos)
    {
        trackbar.SetWindow(hwnd);
        trackbar.SetRange(min, max);
        trackbar.SetPosition(pos);
    }
};

