#ifndef _DIAGNOSIS_OF_GPS_VARIANT_TYPE_H_
#define _DIAGNOSIS_OF_GPS_VARIANT_TYPE_H_ 

// namespace Diagnosis-of-GPS
namespace dog
{

/// Variant data type is a tagged union that can be used to represent any other data type 
struct Variant
{
	enum VariantType {
		VT_NONE,
		VT_INT,
		VT_UINT,
		VT_LONGLONG,
		VT_ULONGLONG,
		VT_FLOAT,
		VT_DOUBLE,
	};
	VariantType type;

	union 
	{
		int iVal;
		unsigned int uiVal;
		long long llVal;
		unsigned long long ullVal;
		float fVal;
		double dblVal;
	};

	Variant() :                       type(VT_NONE),      ullVal(0)   {}
	Variant(int val) :                type(VT_INT),       iVal(val)   {}
	Variant(unsigned int val) :       type(VT_UINT),      uiVal(val)  {}
	Variant(long long val) :          type(VT_LONGLONG),  llVal(val)  {}
	Variant(unsigned long long val) : type(VT_ULONGLONG), ullVal(val) {}
	Variant(float val) :              type(VT_FLOAT),     fVal(val)   {}
	Variant(double val) :             type(VT_DOUBLE),    dblVal(val) {}

	Variant operator=(int val)                { type=VT_INT;       iVal  =val; return *this; }
	Variant operator=(unsigned int val)       { type=VT_UINT;      uiVal =val; return *this; }
	Variant operator=(long long val)          { type=VT_LONGLONG;  llVal =val; return *this; }
	Variant operator=(unsigned long long val) { type=VT_ULONGLONG; ullVal=val; return *this; }
	Variant operator=(float val)              { type=VT_FLOAT;     fVal  =val; return *this; }
	Variant operator=(double val)             { type=VT_DOUBLE;    dblVal=val; return *this; }
};


} // namespace dog

#endif // _DIAGNOSIS_OF_GPS_VARIANT_TYPE_H_
