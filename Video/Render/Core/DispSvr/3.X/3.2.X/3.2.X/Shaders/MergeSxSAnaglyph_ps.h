#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.24.950.2656
//
//   fxc /T ps_3_0 /Fh MergeSxSAnaglyph_ps.h -E MergeSxSAnaglyph_PS
//    MergeSxSAnaglyph.psh
//
//
// Parameters:
//
//   sampler2D texture0;
//
//
// Registers:
//
//   Name         Reg   Size
//   ------------ ----- ----
//   texture0     s0       1
//

    ps_3_0
    def c0, 1, 0, 0, 0
    dcl_texcoord v0
    dcl_2d s0
    texld r0, v0, s0
    mad oC0.xw, r0.x, c0.xyzy, c0.yyzx
    texld r0, v0.zwzw, s0
    mov oC0.yz, r0

// approximately 4 instruction slots used (2 texture, 2 arithmetic)
#endif

const BYTE g_ps30_MergeSxSAnaglyph_PS[] =
{
      0,   3, 255, 255, 254, 255, 
     34,   0,  67,  84,  65,  66, 
     28,   0,   0,   0,  83,   0, 
      0,   0,   0,   3, 255, 255, 
      1,   0,   0,   0,  28,   0, 
      0,   0,   0,   1,   0,   0, 
     76,   0,   0,   0,  48,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,  60,   0, 
      0,   0,   0,   0,   0,   0, 
    116, 101, 120, 116, 117, 114, 
    101,  48,   0, 171, 171, 171, 
      4,   0,  12,   0,   1,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0, 112, 115, 
     95,  51,  95,  48,   0,  77, 
    105,  99, 114, 111, 115, 111, 
    102, 116,  32,  40,  82,  41, 
     32,  72,  76,  83,  76,  32, 
     83, 104,  97, 100, 101, 114, 
     32,  67, 111, 109, 112, 105, 
    108, 101, 114,  32,  57,  46, 
     50,  52,  46,  57,  53,  48, 
     46,  50,  54,  53,  54,   0, 
     81,   0,   0,   5,   0,   0, 
     15, 160,   0,   0, 128,  63, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     31,   0,   0,   2,   5,   0, 
      0, 128,   0,   0,  15, 144, 
     31,   0,   0,   2,   0,   0, 
      0, 144,   0,   8,  15, 160, 
     66,   0,   0,   3,   0,   0, 
     15, 128,   0,   0, 228, 144, 
      0,   8, 228, 160,   4,   0, 
      0,   4,   0,   8,   9, 128, 
      0,   0,   0, 128,   0,   0, 
    100, 160,   0,   0,  37, 160, 
     66,   0,   0,   3,   0,   0, 
     15, 128,   0,   0, 238, 144, 
      0,   8, 228, 160,   1,   0, 
      0,   2,   0,   8,   6, 128, 
      0,   0, 228, 128, 255, 255, 
      0,   0
};
