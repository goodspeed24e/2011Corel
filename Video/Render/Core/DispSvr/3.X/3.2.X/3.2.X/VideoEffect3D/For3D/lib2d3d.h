//------------------------------------------------------------------------------
// File: lib2D3D.h
//
// Desc: 2d3d anaglyph library interface
//
// Copyright (c) 2007 For3d Inc.  All rights reserved.
// Patent Number 6,108,005 and other pending patents
//------------------------------------------------------------------------------

#ifndef _LIB2D3D_H_
#define _LIB2D3D_H_

//#include <d3d9.h>
//#include <d3dx9.h>

int lib2d3d_Init(IDirect3DDevice9* pD3Ddev, DWORD dwWidth, DWORD dwHeight);

IDirect3DTexture9* lib2d3d_ModifyTexture(IDirect3DDevice9* pD3Ddev, IDirect3DTexture9* pTexture);

int lib2d3d_Deinit(IDirect3DDevice9* pD3Ddev);

#endif //_LIB2D3D_H_
