#pragma once

#include "DogVariant.h"
#include <boost/format.hpp>

struct MyVariant : public dog::Variant
{
	MyVariant() {}
	MyVariant(const dog::Variant& var) : dog::Variant(var) {}
	MyVariant operator= (const dog::Variant& var) { (*(dog::Variant*)this) = var;  return *this; }

	MyVariant operator += (int val) { *this = *this + val;  return *this; }
	MyVariant operator -= (int val) { *this = *this - val;  return *this; }

	MyVariant operator + (int val) const
	{
		switch(type)
		{
		default:
		case VT_INT:       return iVal   + val; break;
		case VT_UINT:      return uiVal  + val; break;
		case VT_LONGLONG:  return llVal  + val; break;
		case VT_ULONGLONG: return ullVal + val; break;
		case VT_FLOAT:     return fVal   + val; break;
		case VT_DOUBLE:    return dblVal + val; break;
		}
		return *this;
	}

	MyVariant operator - (int val) const
	{
		switch(type)
		{
		default:
		case VT_INT:       return iVal   - val; break;
		case VT_UINT:      return uiVal  - val; break;
		case VT_LONGLONG:  return llVal  - val; break;
		case VT_ULONGLONG: return ullVal - val; break;
		case VT_FLOAT:     return fVal   - val; break;
		case VT_DOUBLE:    return dblVal - val; break;
		}
		return *this;
	}

	operator const int() const                { return iVal;   }
	operator const unsigned int() const       { return uiVal;  }
	operator const long long() const          { return llVal;  }
	operator const unsigned long long() const { return ullVal; }
	operator const float() const              { return fVal;   }
	operator const double() const             { return dblVal; }

	std::wstring ToWString() const
	{
		switch(type)
		{
		case VT_INT:       return (boost::wformat(L"%d") % iVal).str();
		case VT_UINT:      return (boost::wformat(L"%d") % uiVal).str();
		case VT_LONGLONG:  return (boost::wformat(L"%d") % llVal).str();
		case VT_ULONGLONG: return (boost::wformat(L"%d") % ullVal).str();
		case VT_FLOAT:     return (boost::wformat(L"%f") % fVal).str();
		case VT_DOUBLE:    return (boost::wformat(L"%f") % dblVal).str();
		default:
			return L"";
		}
		return L"";
	}

	double ToDouble() const
	{
		switch(type)
		{
		default:
		case VT_INT:       return (double)iVal;
		case VT_UINT:      return (double)uiVal;
		case VT_LONGLONG:  return (double)llVal;
		case VT_ULONGLONG: return (double)ullVal;
		case VT_FLOAT:     return (double)fVal;
		case VT_DOUBLE:    return dblVal;
		}
		return 0.0f;
	}

	const bool operator < (const MyVariant& var) const
	{
		if(type == var.type)
		{
			switch(type)
			{
			default:
			case VT_INT:       return iVal   < var.iVal;   break;
			case VT_UINT:      return uiVal  < var.uiVal;  break;
			case VT_LONGLONG:  return llVal  < var.llVal;  break;
			case VT_ULONGLONG: return ullVal < var.ullVal; break;
			case VT_FLOAT:     return fVal   < var.fVal;   break;
			case VT_DOUBLE:    return dblVal < var.dblVal; break;
			}
		}
		else
		{
			return ToDouble() < var.ToDouble();
		}
		return false;
	}

	const bool operator == (const MyVariant& var) const
	{
		if(type == var.type) {return ullVal == var.ullVal; }
		else {return ToDouble() == var.ToDouble(); }
	}
};

