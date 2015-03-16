// IVIAntiDebug
#undef iviTR_VERIFY_SOFTICE
#undef iviTR_VERIFY_ATTACHED_DEBUGGER
#undef iviTR_VERIFY_VTUNE_30
#undef iviTR_HALT_THIS_PROCESS
#undef iviTR_CHECK_MICROSOFT_PIN
#undef iviTR_CHECK_THREAD_TIMER
#undef iviTR_GETRDTSC

#define iviTR_VERIFY_SOFTICE(proc) {}
#define iviTR_VERIFY_ATTACHED_DEBUGGER(proc) {}
#define iviTR_VERIFY_VTUNE_30(proc) {}
#define iviTR_HALT_THIS_PROCESS(x) ExitProcess(0)
#define iviTR_CHECK_MICROSOFT_PIN(testpin, flag) {(flag)=0;}
#define iviTR_CHECK_THREAD_TIMER(timeout_sec,proc) {}
#define iviTR_GETRDTSC(dword_variable) {dword_variable=0;}

// IVIAntiDebug2
#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_A
#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A
#undef iviTR_VERIFY_SOFTICE_SICE_A
#undef iviTR_VERIFY_SOFTICE_SICE_DD_A
#undef iviTR_VERIFY_SOFTICE_NTICE_A
#undef iviTR_VERIFY_SOFTICE_NTICE_DD_A
#undef iviTR_VERIFY_SOFTICE_SIWVID_A
#undef iviTR_VERIFY_SOFTICE_SIWVID_DD_A
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_A
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A

#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV
#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV
#undef iviTR_VERIFY_SOFTICE_SICE_A_INV
#undef iviTR_VERIFY_SOFTICE_SICE_DD_A_INV
#undef iviTR_VERIFY_SOFTICE_NTICE_A_INV
#undef iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV
#undef iviTR_VERIFY_SOFTICE_SIWVID_A_INV
#undef iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV

#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_W
#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W
#undef iviTR_VERIFY_SOFTICE_SICE_W
#undef iviTR_VERIFY_SOFTICE_SICE_DD_W
#undef iviTR_VERIFY_SOFTICE_NTICE_W
#undef iviTR_VERIFY_SOFTICE_NTICE_DD_W
#undef iviTR_VERIFY_SOFTICE_SIWVID_W
#undef iviTR_VERIFY_SOFTICE_SIWVID_DD_W
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_W
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W

#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV
#undef iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV
#undef iviTR_VERIFY_SOFTICE_SICE_W_INV
#undef iviTR_VERIFY_SOFTICE_SICE_DD_W_INV
#undef iviTR_VERIFY_SOFTICE_NTICE_W_INV
#undef iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV
#undef iviTR_VERIFY_SOFTICE_SIWVID_W_INV
#undef iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV
#undef iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV

#undef iviTR_VERIFY_SOFTICE_LCREAT
#undef iviTR_VERIFY_SOFTICE_LCREAT1
#undef iviTR_VERIFY_SOFTICE_LCREAT2
#undef iviTR_VERIFY_SOFTICE_LCREAT3
#undef iviTR_VERIFY_SOFTICE_LCREAT4

#undef iviTR_VERIFY_SOFTICE_LCREAT_INV
#undef iviTR_VERIFY_SOFTICE_LCREAT1_INV
#undef iviTR_VERIFY_SOFTICE_LCREAT2_INV
#undef iviTR_VERIFY_SOFTICE_LCREAT3_INV
#undef iviTR_VERIFY_SOFTICE_LCREAT4_INV

#undef iviTR_VERIFY_SOFTICE_LOPEN
#undef iviTR_VERIFY_SOFTICE_LOPEN1
#undef iviTR_VERIFY_SOFTICE_LOPEN2
#undef iviTR_VERIFY_SOFTICE_LOPEN3
#undef iviTR_VERIFY_SOFTICE_LOPEN4

#undef iviTR_VERIFY_SOFTICE_LOPEN_INV
#undef iviTR_VERIFY_SOFTICE_LOPEN1_INV
#undef iviTR_VERIFY_SOFTICE_LOPEN2_INV
#undef iviTR_VERIFY_SOFTICE_LOPEN3_INV
#undef iviTR_VERIFY_SOFTICE_LOPEN4_INV

#undef iviTR_VERIFY_SOFTICE_NTICE_SERVICE
#undef iviTR_VERIFY_SOFTICE_NTICE_SERVICE_INV

#undef iviTR_VERIFY_SYSER_A
#undef iviTR_VERIFY_SYSER_DD_A
#undef iviTR_VERIFY_SYSER_W
#undef iviTR_VERIFY_SYSER_DD_W
#undef iviTR_VERIFY_SYSER_LCREAT
#undef iviTR_VERIFY_SYSER_LOPEN

#undef iviTR_INT1H_CHECK
#undef iviTR_ICEBP_CHECK
#undef iviTR_API_ISDEBUGGERPRESENT_CHECK

#undef iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK
#undef iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1
#undef iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2
#undef iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3
#undef iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4
#undef iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5
#undef iviTR_DEBUGBREAK_CHECK

#undef iviTR_DEBUG_REGISTER_CHECK

#undef iviTR_DAEMON_FINDER
#undef iviTR_DETECT_TRACER_TRAP_FLAG

#undef iviTR_SYSTEM_DEBUG_REGISTRY_CHECK
#undef iviTR_VERIFY_SYSER_ENGINE
#undef iviTR_VERIFY_SYSER_SYSERBOOT
#undef iviTR_DETECT_PRINTEXCEPTION
#undef iviTR_INT2D_CHECK
#undef iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK
#undef iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK_INV

#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SICE_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SICE_DD_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_NTICE_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_NTICE_DD_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWVID_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A(bool_variable){bool_variable = false;}

#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SICE_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SICE_DD_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_NTICE_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_NTICE_DD_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWVID_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_A_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_A_INV(bool_variable){bool_variable = true;}

#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SICE_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SICE_DD_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_NTICE_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_NTICE_DD_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWVID_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W(bool_variable){bool_variable = false;}

#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWVIDSTART_DD_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SICE_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SICE_DD_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_NTICE_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_NTICE_DD_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWVID_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWVID_DD_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_W_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_SIWDEBUG_DD_W_INV(bool_variable){bool_variable = true;}

#define iviTR_VERIFY_SOFTICE_LCREAT(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LCREAT1(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LCREAT2(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LCREAT3(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LCREAT4(bool_variable){bool_variable = false;}

#define iviTR_VERIFY_SOFTICE_LCREAT_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LCREAT1_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LCREAT2_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LCREAT3_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LCREAT4_INV(bool_variable){bool_variable = true;}

#define iviTR_VERIFY_SOFTICE_LOPEN(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LOPEN1(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LOPEN2(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LOPEN3(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_LOPEN4(bool_variable){bool_variable = false;}

#define iviTR_VERIFY_SOFTICE_LOPEN_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LOPEN1_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LOPEN2_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LOPEN3_INV(bool_variable){bool_variable = true;}
#define iviTR_VERIFY_SOFTICE_LOPEN4_INV(bool_variable){bool_variable = true;}

#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE(bool_variable) {bool_variable = false;}
#define iviTR_VERIFY_SOFTICE_NTICE_SERVICE_INV(bool_variable) {bool_variable = true;}

#define iviTR_VERIFY_SYSER_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SYSER_DD_A(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SYSER_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SYSER_DD_W(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SYSER_LCREAT(bool_variable){bool_variable = false;}
#define iviTR_VERIFY_SYSER_LOPEN(bool_variable){bool_variable = false;}

#define iviTR_INT1H_CHECK(bool_variable) {bool_variable = false;}
#define iviTR_ICEBP_CHECK(bool_variable) {bool_variable = false;}
#define iviTR_API_ISDEBUGGERPRESENT_CHECK(bool_variable) {bool_variable = false;}

#define iviTR_SOFTWARE_NUMEGA_SOFTICE_REGISTRY_CHECK(bool_variable) {bool_variable = false;}
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK1(bool_variable) {bool_variable = false;}
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK2(bool_variable) {bool_variable = false;}
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK3(bool_variable) {bool_variable = false;}
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK4(bool_variable) {bool_variable = false;}
#define iviTR_SOFTWARE_SYSERSOFT_SYSER_REGISTRY_CHECK5(bool_variable) {bool_variable = false;}
#define iviTR_DEBUGBREAK_CHECK(bool_variable) {bool_variable = false;}

#define iviTR_DEBUG_REGISTER_CHECK(bool_variable) {bool_variable = false;}

#define iviTR_DAEMON_FINDER(bool_variable) {bool_variable = false;}
#define iviTR_DETECT_TRACER_TRAP_FLAG(dword_variable) {dword_variable = 1;}

#define iviTR_SYSTEM_DEBUG_REGISTRY_CHECK(bool_variable) {bool_variable = false;}
#define iviTR_VERIFY_SYSER_ENGINE(bool_variable) {bool_variable = false;}
#define iviTR_VERIFY_SYSER_SYSERBOOT(bool_variable) {bool_variable = false;}
#define iviTR_DETECT_PRINTEXCEPTION(bool_variable) {bool_variable = false;}
#define iviTR_INT2D_CHECK(bool_variable) {bool_variable = false;}
#define iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK(bool_variable) {bool_variable = false;}
#define iviTR_SOFTICE_ICEEXT_REGISTRY_CHECK_INV(bool_variable) {bool_variable = true;}

// DebugDetectExcept
#ifdef iviTR_DEBUG_DETECT_INIT
    #undef iviTR_DEBUG_DETECT_INIT
#endif

#ifdef iviTR_DEBUG_DETECT_START
    #undef iviTR_DEBUG_DETECT_START
#endif

#ifdef iviTR_DEBUG_DETECT_WAIT
    #undef iviTR_DEBUG_DETECT_WAIT
#endif

#ifdef iviTR_DEBUG_DETECT_RES
    #undef iviTR_DEBUG_DETECT_RES
#endif

#ifdef iviTR_DEBUG_DETECT_END
    #undef iviTR_DEBUG_DETECT_END
#endif

#define iviTR_DEBUG_DETECT_INIT {}
#define iviTR_DEBUG_DETECT_START {}
#define iviTR_DEBUG_DETECT_WAIT {}
#define iviTR_DEBUG_DETECT_RES FALSE
#define iviTR_DEBUG_DETECT_END {}

