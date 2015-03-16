/****************************************************************************** 
/* Header: ThemidaSDK.h
/* Description: SDK header definition for the C/C++ language
/*
/* Author/s: Rafael Ahucha  
/* (c) 2004 Oreans Technologies
/*****************************************************************************/ 

///*!
// [Internal use. Do not call it.]
//
// Themida SDK Macro.
//*/

#ifndef __THEMIDASDK__

#define __THEMIDASDK__


/* Definition for Borland C/C++  */

#ifdef __BORLANDC__
 
 #define CODEREPLACE_START __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define CODEREPLACE_END  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x01, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
  
 #define ENCODE_START     __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x04, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define ENCODE_END       __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x05, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);

 #define CLEAR_START      __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x06, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define CLEAR_END        __emit__ (0xEB, 0x15, 0x57, 0x4C, 0x20, 0x20, 0x07, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, \
                                    0x00, 0x00, 0x00);

 #define VM_START     __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
 #define VM_END       __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0D, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);

 #define VM_START_WITHLEVEL(x)     __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, x, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);

 #define __WL_MACROS__

 #endif
 
#endif


/* Definition for Intel cpp compiler */
 
#ifndef __WL_MACROS__
 
 #ifdef __ICL
 
 #define CODEREPLACE_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
 
 #define CODEREPLACE_END \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x01 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
 
 #define ENCODE_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x04 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  
 #define ENCODE_END \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x05 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  
 #define CLEAR_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x06 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
 
 #define CLEAR_END \
  __asm __emit 0xEB \
  __asm __emit 0x15 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x07 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 

  #define VM_START \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x0C \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  
 #define VM_END \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x0D \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \

  #define VM_START_WITHLEVEL(x) \
  __asm __emit 0xEB \
  __asm __emit 0x10 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \
  __asm __emit 0x0C \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit x \
  __asm __emit 0x00 \
  __asm __emit 0x00 \
  __asm __emit 0x57 \
  __asm __emit 0x4C \
  __asm __emit 0x20 \
  __asm __emit 0x20 \

 #define __WL_MACROS__
 
 #endif

#endif
 
 
/* Definition for LCC by Jacob Navia */
 
#ifndef __WL_MACROS__
 
 #ifdef __LCC__
 
 #define CODEREPLACE_START __asm__ (" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 #define CODEREPLACE_END  __asm__ (" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x01, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
  
 #define ENCODE_START     __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x04, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 #define ENCODE_END       __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x05, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 
 #define CLEAR_START      __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x06, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");
 
 #define CLEAR_END        __asm__(" .byte\t0xEB, 0x15, 0x57, 0x4C, 0x20, 0x20, 0x07, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x00, \
                                    0x00, 0x00, 0x00");
 #define VM_START         __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define VM_END           __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0D, 0x00, 0x00, 0x00, \
                                    0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define VM_START_WITHLEVEL(x) __asm__(" .byte\t0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0C, 0x00, 0x00, 0x00, \
                                    0x00, x, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20");

 #define __WL_MACROS__
 
 #endif
 
#endif
 

/* Definition for Visual C++ and others */

#ifndef __WL_MACROS__

///*!
// [Internal use. Do not call it.]
//
// place start mark for encrypted by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place before your protected source code in function scope.
//*/
 #define CODEREPLACE_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \

///*!
// [Internal use. Do not call it.]
//
// place end mark for encrypted by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place after your protected source code in function scope.
// Do not place after 'return' code.
//*/
  #define CODEREPLACE_END \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x01 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
///*!
// [Internal use. Do not call it.]
//
// place start mark for encrypted by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place before your protected source code in function scope.
//*/
 #define ENCODE_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x04 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
///*!
// [Internal use. Do not call it.]
//
// place end mark for encrypted by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place after your protected source code in function scope.
// Do not place after 'return' code.
//*/
 #define ENCODE_END \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x05 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
///*!
// [Internal use. Do not call it.]
//
// place start mark for clearing stack by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place before your protected source code in function scope.
//*/
 #define CLEAR_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x06 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
///*!
// [Internal use. Do not call it.]
//
// place end mark for clearing stack by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place after your protected source code in function scope.
// Do not place after 'return' code.
//*/
 #define CLEAR_END \
  __asm _emit 0xEB \
  __asm _emit 0x15 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x07 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 

///*!
// [Internal use. Do not call it.]
//
// place start mark for encrypted by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place before your protected source code in function scope.
//*/
 #define VM_START \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x0C \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
 
///*!
// [Internal use. Do not call it.]
//
// place end mark for encrypted by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	High
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Place after your protected source code in function scope.
// Do not place after 'return' code.
//*/
 #define VM_END \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x0D \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \

///*!
// [Internal use. Do not call it.]
//
// set VM encoding security level by 3rd-party software "Themida"
// 
// Return value - None
//
// Supported platform:		All Windows
// Performance overhead:	Low
// Security level:			High
// Usage scope:				Macro scope
//
// Note:\n
// Need co-orperated with 3rd-party software.
// Need used before VM_START macro.
// Place before your protected source code in function scope.
//*/
 #define VM_START_WITHLEVEL(x) \
  __asm _emit 0xEB \
  __asm _emit 0x10 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \
  __asm _emit 0x0C \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit x \
  __asm _emit 0x00 \
  __asm _emit 0x00 \
  __asm _emit 0x57 \
  __asm _emit 0x4C \
  __asm _emit 0x20 \
  __asm _emit 0x20 \

#endif
