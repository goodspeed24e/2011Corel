#ifndef _NVH264_MCP_H_
#define _NVH264_MCP_H_

#include "global.h"

#if defined(_HW_ACCEL_)

#define END_OF_FRAME            0
#define INTRA_BUFFER_IS_FULL    1
#define INTER_BUFFER_IS_FULL    2

#define NVH264VP1_PRED_FLAG_INTER               0x01    // inter partition
#define NVH264VP1_PRED_FLAG_CHROMA              0x02    // chroma partition
#define NVH264VP1_PRED_FLAG_BI_PRED             0x04    // bi prediction (inter only)
#define NVH264VP1_PRED_FLAG_PREV_INTRA          0x08    // previous macroblock was intra
#define NVH264VP1_PRED_FLAG_MBAFF_FIELD         0x10    // field macroblock (MBAFF only)
#define NVH264VP1_PRED_FLAG_MBAFF_BOTTOM_FIELD  0x20    // bottom field (MBAFF only)
#define NVH264VP1_PRED_FLAG_MBAFF_LEFT_FRAME    0x40    // left mb frame, cur top field
#define NVH264VP1_PRED_FLAG_MBAFF_LEFT_FIELD    0x80    // left bot field, cur frame


enum nvh264vp1UncompressedBufType
{
	NVH264VP1_UNCOMPRESSED_BUF_LUMA = 0xFFFFFFF1,
	NVH264VP1_UNCOMPRESSED_BUF_CHROMA,
	NVH264VP1_UNCOMPRESSED_BUF_LUMA_CHROMA,
};


enum nvh264vp1IntraMacroblockType 
{
	NVH264VP1_INTRA_MBTYPE_PCM = 1,             
	NVH264VP1_INTRA_MBTYPE_16x16 = 2, 
	NVH264VP1_INTRA_MBTYPE_8x8 = 3,
	NVH264VP1_INTRA_MBTYPE_4x4 = 4,
};

// Values for 4x4 intra prediction mode
enum nvh264vp1Intra4x4PredictionMode 
{
	NVH264VP1_INTRA_4X4_VERTICAL = 1,
	NVH264VP1_INTRA_4X4_HORIZONTAL,
	NVH264VP1_INTRA_4X4_DC_BOTH,
	NVH264VP1_INTRA_4X4_DIAGONAL_DOWN_LEFT,
	NVH264VP1_INTRA_4X4_DIAGONAL_DOWN_RIGHT,
	NVH264VP1_INTRA_4X4_VERTICAL_RIGHT,
	NVH264VP1_INTRA_4X4_HORIZONTAL_DOWN,
	NVH264VP1_INTRA_4X4_VERTICAL_LEFT,
	NVH264VP1_INTRA_4X4_HORIZONTAL_UP,
	NVH264VP1_INTRA_4X4_DC_NONE,
	NVH264VP1_INTRA_4X4_DC_LEFT,
	NVH264VP1_INTRA_4X4_DC_TOP,
	NVH264VP1_INTRA_4X4_DIAGONAL_DOWN_LEFT_NO_TOP_RIGHT,
	NVH264VP1_INTRA_4X4_VERTICAL_LEFT_NO_TOP_RIGHT
};

// Values for 8x8 intra prediction mode
enum nvh264vp1Intra8x8PredictionMode 
{
	NVH264VP1_INTRA_8X8_VERTICAL = 1,
	NVH264VP1_INTRA_8X8_HORIZONTAL,
	NVH264VP1_INTRA_8X8_DC_BOTH,
	NVH264VP1_INTRA_8X8_DIAGONAL_DOWN_LEFT,
	NVH264VP1_INTRA_8X8_DIAGONAL_DOWN_RIGHT,
	NVH264VP1_INTRA_8X8_VERTICAL_RIGHT,
	NVH264VP1_INTRA_8X8_HORIZONTAL_DOWN,
	NVH264VP1_INTRA_8X8_VERTICAL_LEFT,
	NVH264VP1_INTRA_8X8_HORIZONTAL_UP,
	NVH264VP1_INTRA_8X8_DC_NONE,
	NVH264VP1_INTRA_8X8_DC_LEFT,
	NVH264VP1_INTRA_8X8_DC_TOP
};

// Additional flags added to 8x8 intra prediction mode
#define NVH264VP1_INTRA_8X8_TOP_LEFT_AVAILABLE      0x10
#define NVH264VP1_INTRA_8X8_TOP_RIGHT_AVAILABLE     0x20

// Values for 16x16 intra prediction mode
enum nvh264vp1Intra16x16PredictionMode 
{
	NVH264VP1_INTRA_16X16_VERTICAL = 1,
	NVH264VP1_INTRA_16X16_HORIZONTAL,
	NVH264VP1_INTRA_16X16_DC_BOTH,
	NVH264VP1_INTRA_16X16_PLANE,
	NVH264VP1_INTRA_16X16_DC_NONE,
	NVH264VP1_INTRA_16X16_DC_LEFT,
	NVH264VP1_INTRA_16X16_DC_TOP
};
// Values for intra chroma prediction mode
enum nvh264vp1IntraChromaPredictionMode 
{
	NVH264VP1_INTRA_CHROMA_DC_LEFT_TOP = 1,
	NVH264VP1_INTRA_CHROMA_HORIZONTAL,
	NVH264VP1_INTRA_CHROMA_VERTICAL,
	NVH264VP1_INTRA_CHROMA_PLANE,
	NVH264VP1_INTRA_CHROMA_DC_NONE,
	NVH264VP1_INTRA_CHROMA_DC_UPPER_LEFT,
	NVH264VP1_INTRA_CHROMA_DC_LOWER_LEFT,
	NVH264VP1_INTRA_CHROMA_DC_LEFT,
	NVH264VP1_INTRA_CHROMA_DC_TOP,
	NVH264VP1_INTRA_CHROMA_DC_UPPER_LEFT_TOP,
	NVH264VP1_INTRA_CHROMA_DC_LOWER_LEFT_TOP
};

typedef struct nvh264vp1PictureDecode 
{
	DWORD ulFrameIndex;           // DXVA uncompressed surface index
	DWORD ulFlags;                // bit 0 = frame(0) or field(1)
	// bit 1 = top(0) or bottom(1) field
	// bit 2 = MBAFF(1)
	DWORD ulPicWidth;             // width of picture, in pixels
	DWORD ulPicHeight;            // height of picture, in pixels
}nvh264vp1PictureDecode;

typedef struct nvh264vp1MacroblockControlIntra 
{
	// Flags and partition information
	BYTE predFlags;           // prediction flags (see definitions)
	BYTE partIndex;           // bits 0-3 = mb type, bits 4-7 = decr partition index
	BYTE partOffset;          // bits 0-3 = X, bits 4-7 = Y
	BYTE predMode;            // intra prediction mode 

	// Macroblock position
	WORD uX;                 // macroblock X offset into picture
	WORD uY;                 // macroblock Y offset into picture
}nvh264vp1MacroblockControlIntra;


typedef struct nvh264vp1MacroblockControlInterLuma 
{
	// Flags and partition information
	BYTE predFlags;           // prediction flags (see definitions)
	BYTE partIndex;           // bits 0-3 = CBP, bits 4-7 = decrementing partition index
	BYTE partOffset;          // bits 0-3 = X, bits 4-7 = Y
	BYTE partSize;            // bits 0-3 = width/2, bits 4-7 = height/2

	// Macroblock position
	WORD uX;                 // macroblock X offset into picture
	WORD uY;                 // macroblock Y offset into picture

	// Reference frames
	BYTE refIndex;            // surface index for reference frame
	BYTE refFlags;            // flags for reference frame
	BYTE refIndex2;           // surface index for second reference frame
	BYTE refFlags2;           // flags for second reference frame

	// Prediction
	WORD mvx;                // X motion vector 
	WORD mvy;                // Y motion vector 
	WORD wL;                 // luma weight
	BYTE oL;                  // luma offset
	BYTE logWDL;

	// Extra data valid only if bi prediction
	WORD mvx2;               // X motion vector
	WORD mvy2;               // Y motion vector
	WORD wL2;                // luma weight
	WORD oL2;                // luma offset

}nvh264vp1MacroblockControlInterLuma;

typedef struct nvh264vp1MacroblockControlInterChroma
{
	// Flags and partition information
	BYTE predFlags;           // prediction flags (see definitions)
	BYTE partIndex;           // bits 0-1 = CBP, bits 4-7 = decrementing index
	BYTE partSize;            // bits 0-3 = width/2, bits 4-7 = height/2
	BYTE partOffset;          // bits 0-3 = X, bits 4-7 = Y

	// Macroblock position
	WORD uX;                 // macroblock X offset into picture
	WORD uY;                 // macroblock Y offset into picture

	// Reference frames
	BYTE refIndex;            // surface index for reference frame
	BYTE refFlags;            // flags for reference frame
	BYTE refIndex2;           // surface index for second reference frame
	BYTE refFlags2;           // flags for second reference frame

	// Prediction
	WORD mvx;                // X motion vector
	WORD mvy;                // Y motion vector
	WORD wCb;                // Cb weight
	WORD wCr;                // Cr weight
	BYTE oCb;                 // Cb offset
	BYTE oCr;                 // Cr offset
	WORD logWDC;

	// Extra data valid only if bi prediction
	WORD mvx2;               // X motion vector
	WORD mvy2;               // Y motion vector
	WORD wCb2;               // Cb weight
	WORD wCr2;               // Cr weight
	WORD oCb2;               // Cb offset
	WORD oCr2;               // Cr offset
}nvh264vp1MacroblockControlInterChroma;

typedef struct _DXVA_H264DecUncompressedBufControl {
	unsigned int dwBufType;			// uncompressed buffer type
	unsigned int dwLines;			// number of picture lines
	unsigned int dwStartingLine;	// the starting line of the picture
} DXVA_H264DecUncompressedBufControl, *LPDXVA_H264DecUncompressedBufControl;

struct EdgeFlagByte0 
{
	unsigned char eFlag_0:2;                // 0 = no filtering
	// 1 = weak filtering
	// 2 = strong filtering
	unsigned char interleavedLeftEdge:2;    // 1 = interleaved left edge (vert only)
	// 2 = interleaved in field mode
	unsigned char twoTopEdges:1;            // 1 = two top edges (horz only)
	unsigned char mbField:1;                // 1 = field decoding for macroblock
	unsigned char lineEdgeFlag:2;           // bitwise OR of 4 edge flags 
};

struct eFlag_s
{
	union{
		unsigned char eFlag0;             
		EdgeFlagByte0 b0;
	};
	unsigned char eFlag1;
	unsigned char eFlag2;
	unsigned char eFlag3;
};

struct eFlagTC0_s
{
	eFlag_s edgeFlag;              // 4 flags per edge
	unsigned char tC0[4];          // 4 tC0 values per edge
};

// Luma deblocking parameters (non-MBAFF)
struct EdgeDataLuma {
	unsigned char          AlphaEdge;      // for left or top edge
	unsigned char          BetaEdge;       // for left or top edge
	unsigned char          AlphaInner;     // for inner edges
	unsigned char          BetaInner;      // for inner edges  
	eFlagTC0_s			   eFlagTC0[4];    // flags and tC0 for 4 edges
};

// Chroma deblocking parameters (non-MBAFF)
struct EdgeDataChroma 
{
	eFlag_s				   edgeFlag[2];        // 2 edge flags
	unsigned char          AlphaEdge_Cb;       // for left or top edge
	unsigned char          BetaEdge_Cb;        // for left or top edge         
	unsigned char          AlphaInner_Cb;      // for inner edges
	unsigned char          BetaInner_Cb;       // for inner edges
	unsigned char          tC0_Cb[2][4];       // tC0 [edge][block]
	unsigned char          AlphaEdge_Cr;       // for left or top edge
	unsigned char          BetaEdge_Cr;        // for left or top edge         
	unsigned char          AlphaInner_Cr;      // for inner edges
	unsigned char          BetaInner_Cr;       // for inner edges
	unsigned char          tC0_Cr[2][4];       // tC0 [edge][block]      
};

// Luma deblocking parameters (MBAFF)
struct EdgeDataLumaMBAFF {
	unsigned char          AlphaEdge;      // for left or top edge
	unsigned char          BetaEdge;       // for left or top edge         
	unsigned char          AlphaInner;     // for inner edges
	unsigned char          BetaInner;      // for inner edges         
	eFlagTC0_s			   eFlagTC0[5];    // 0 = extra if used, 1-4 = normal edges
	unsigned char          AlphaEdge2;     // for second left or top edge
	unsigned char          BetaEdge2;      // for second left or top edge    
	unsigned short         Reserved;       // for alignment
};

// Chroma deblocking parameters (MBAFF)
struct EdgeDataChromaMBAFF 
{
	eFlag_s				   edgeFlag[3];        // 3 edge flags
	unsigned char          AlphaEdge_Cb;       // for left or top edge
	unsigned char          BetaEdge_Cb;        // for left or top edge         
	unsigned char          AlphaInner_Cb;      // for inner edges
	unsigned char          BetaInner_Cb;       // for inner edges
	unsigned char          tC0_Cb[3][4];       // tC0 [edge][block]
	unsigned char          AlphaEdge2_Cb;      // for second left or top edge
	unsigned char          BetaEdge2_Cb;       // for second left or top edge    
	unsigned short         Reserved;           // for alignment
	unsigned char          AlphaEdge_Cr;       // for left or top edge
	unsigned char          BetaEdge_Cr;        // for left or top edge         
	unsigned char          AlphaInner_Cr;      // for inner edges
	unsigned char          BetaInner_Cr;       // for inner edges
	unsigned char          tC0_Cr[3][4];       // tC0 [edge][block]     
	unsigned char          AlphaEdge2_Cr;      // for second left or top edge
	unsigned char          BetaEdge2_Cr;       // for second left or top edge    
	unsigned short         Reserved2;          // for alignment
};

#endif /* defined(_HW_ACCEL_) */

#endif
