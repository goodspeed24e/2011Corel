

#ifndef _SCD_AUTHENTICATION_H
#define _SCD_AUTHENTICATION_H

#define COREL_TEST_APPLICATION_ID 0xf505c140
#define COREL_REAL_APPLICATION_ID 0x00000000 // to be distributed securely

// Use this function to get the XOR pattern
// Input:  dwAppId    - This is always the same and is provided by Intel above
//         dwSeed     - This is a random number and should be different for every call
// Output: pdwPattern - The value returned to this pointer should be used to XOR the
//                      surface for scrambling.
// Return value       - 1 for success or 0 for failure
extern "C" BOOLEAN GetScramblePattern(DWORD dwAppId, DWORD dwSeed, DWORD* pdwPattern);

#endif //_SCD_AUTHENTICATION_H