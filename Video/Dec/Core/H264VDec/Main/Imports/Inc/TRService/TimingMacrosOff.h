//Code Timing Macros
#ifdef iviTR_SET_TIMEOUT_COUNT
#undef iviTR_SET_TIMEOUT_COUNT
#endif

#ifdef iviTR_SET_INITIAL_TIMESTAMP
#undef iviTR_SET_INITIAL_TIMESTAMP
#endif

#ifdef iviTR_SET_FINAL_TIMESTAMP
#undef iviTR_SET_FINAL_TIMESTAMP
#endif

#ifdef iviTR_TIMESTAMP_CHECK
#undef iviTR_TIMESTAMP_CHECK
#endif

#define iviTR_SET_TIMEOUT_COUNT(int_variable)
#define iviTR_SET_INITIAL_TIMESTAMP(t1)
#define iviTR_SET_FINAL_TIMESTAMP(t2)
#define iviTR_TIMESTAMP_CHECK(t1, t2, dword_variable, bool_variable) {bool_variable = false;}
