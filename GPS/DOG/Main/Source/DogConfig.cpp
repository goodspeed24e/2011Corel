#include "DogConfig.h"

#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

// namespace Diagnosis-of-GPS
namespace dog
{


//-----------------------------------------------------------------------------
CDogConfig::CDogConfig() : m_nSizeOfMMF(64*1024), m_MmfStamp(0)
{
	InitConfigStringMapTable();
	Reset();

	const LPCTSTR szMmfName = L"DogConfig-{D9E5ADCD-1914-4e2e-97A5-BC076B2FC5CF}";
	if( !m_MMF.Open(szMmfName))
	{
		m_MMF.Create(szMmfName, static_cast<DWORD>(m_nSizeOfMMF));
		TMmfHeader mmfHeader;
		memcpy(m_MMF.GetData(), &mmfHeader, sizeof(mmfHeader));
	}
}

CDogConfig::~CDogConfig()
{
	m_MMF.Close();
}

void CDogConfig::Reset()
{
	m_PropertyTrees.clear();
	m_PropertyTrees.resize(NUM_OF_DOG_FUNC);

	m_ErrorMessage.clear();
	m_ReadLineErrMsg.clear();
}

//-----------------------------------------------------------------------------
bool CDogConfig::ReadFromFile(const char* filename)
{
	std::ifstream fin(filename, std::ios_base::in);
	if(fin.fail())
	{
		return false;
	}

	bool bResult = true;
	int nLine = 0;
	std::string strline;
	while(!fin.fail() && !fin.eof())
	{
		char linebuf[256];
		fin.getline(linebuf, sizeof(linebuf));
		if(fin.rdstate() & std::ifstream::eofbit)
		{
			break;
		}

		strline = linebuf;

		while ((fin.rdstate() & std::ifstream::failbit) && !(fin.rdstate() & std::ifstream::eofbit))
		{
			fin.getline(linebuf, sizeof(linebuf));
			if(fin.rdstate() & std::ifstream::eofbit)
			{
				break;
			}

			strline += std::string(linebuf);
		}

		try
		{
			++nLine;
			bool ret = ReadLine(strline);
			if(!ret)
			{
				bResult = false;
				m_ErrorMessage.push_back( (boost::format("Error: line %d: \"%s\". ") %nLine %strline).str() );
				m_ErrorMessage.push_back(m_ReadLineErrMsg);
			}
		}
		catch (...) {
			// syntax error
			bResult = false;
		}
	}

	fin.close();

	UpdateToMMF();
	return bResult;
}

bool CDogConfig::ReadFromString(const char* szConfigString)
{
	bool bResult = true;
	int nLine = 0;
	split_vector_type lines;
	boost::split( lines, szConfigString, boost::is_any_of("\n"));
	for(size_t i=0; i<lines.size(); ++i)
	{
		try
		{
			++nLine;
			bool ret = ReadLine(lines[i]);
			if(!ret)
			{
				bResult = false;
				m_ErrorMessage.push_back( (boost::format("Error: line %d: \"%s\". ") %nLine %lines[i]).str() );
				m_ErrorMessage.push_back(m_ReadLineErrMsg);
			}
		}
		catch (...) {
			// syntax error
			bResult = false;
		}
	}

	UpdateToMMF();
	return bResult;
}

//-----------------------------------------------------------------------------
namespace dog_internal
{
	class string_map_id_error : public std::exception
	{
	public:
		string_map_id_error(std::string _what) : std::exception(_what.c_str()) {}
	};
} // namespace dog_internal

static inline unsigned int StringMapToId(std::string str, const std::map<std::string, unsigned int>& mapTable)
{
	std::map<std::string, unsigned int>::const_iterator iter = mapTable.find(str);
	if(iter == mapTable.end())
	{
		using boost::lexical_cast;
		using boost::bad_lexical_cast;

		try {
			return lexical_cast<unsigned int>(str);
		} catch(bad_lexical_cast &) {
			throw dog_internal::string_map_id_error(std::string("'") + str + "' doesn't be defined");
		}
	}
	else
	{
		return iter->second;
	}

	return (std::numeric_limits<unsigned int>::max)();
}

static inline unsigned int StringMapToId(std::string str)
{
	using boost::lexical_cast;
	using boost::bad_lexical_cast;

	try {
		return lexical_cast<unsigned int>(str);
	} catch(bad_lexical_cast &) {
		throw dog_internal::string_map_id_error(std::string("'") + str + "' doesn't be defined");
	}

	return (std::numeric_limits<unsigned int>::max)();
}

//-----------------------------------------------------------------------------
bool CDogConfig::ReadLine(const std::string& str)
{
	m_ReadLineErrMsg.clear();
	std::string line = boost::trim_copy(str);
	if(line.empty()) {
		return true;
	}

	split_vector_type PropertyAndValue;
	boost::split( PropertyAndValue, line, boost::is_any_of("="));
	if(PropertyAndValue.size() != 2)
	{
		// syntax error : no '=' is found
		return false;
	}

	split_vector_type PropertyNode;
	boost::split( PropertyNode, PropertyAndValue[0], boost::is_any_of("."));
	if(PropertyNode.empty() || PropertyNode.size() > 4) {
		return false; // syntax error
	}

	for(size_t i=0; i<PropertyNode.size(); ++i)
	{
		boost::trim(PropertyNode[i]);
		boost::to_lower(PropertyNode[i]);
	}

	split_vector_type Values;
	boost::split( Values, PropertyAndValue[1], boost::is_any_of(","));
	if(Values.empty()) {
		return false; // syntax error: no value specified
	}

	for(size_t i=0; i<Values.size(); ++i)
	{
		boost::trim(Values[i]);
		boost::to_lower(Values[i]);
	}

	try
	{
		if( std::string("output") == PropertyNode[PropertyNode.size()-1]) {
			UpdateOutputProperty(PropertyNode, Values);
		} else {
			UpdateEnableProperty(PropertyNode, Values);
		}
	}
	catch(const dog_internal::string_map_id_error& e)
	{
		m_ReadLineErrMsg = e.what();
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void CDogConfig::UpdateOutputProperty(const CDogConfig::split_vector_type& PropertyNode, const CDogConfig::split_vector_type& Values)
{
	std::vector<unsigned int> outputList;
	for(size_t i=0; i<Values.size(); ++i)
	{
		const unsigned int outputId = StringMapToId(Values[i], m_OutputNameMap);
		outputList.push_back(outputId);
	}

	// remove duplicated elements
	std::vector<unsigned int>::iterator new_last;
	std::sort(outputList.begin(), outputList.end());
	new_last = std::unique(outputList.begin(), outputList.end());
	outputList.erase(new_last, outputList.end());


	if(2 == PropertyNode.size()) // function.output = ...
	{
		const unsigned int functionId = StringMapToId(PropertyNode[0], m_DogFunctionNameMap);

		TProperty property = GetProperty(functionId);
		property.OutputList = outputList;
		SetProperty(functionId, property);
	}
	else if (3 == PropertyNode.size()) // function.category.output = ...
	{
		const unsigned int functionId = StringMapToId(PropertyNode[0], m_DogFunctionNameMap);
		const unsigned int categoryId = StringMapToId(PropertyNode[1], m_CategoryNameMap);

		TProperty property = GetProperty(functionId, categoryId);
		property.OutputList = outputList;
		SetProperty(functionId, categoryId, property);
	}
	else if (4 == PropertyNode.size()) // function.category.subcategory.output = ...
	{
		const unsigned int functionId = StringMapToId(PropertyNode[0], m_DogFunctionNameMap);
		const unsigned int categoryId = StringMapToId(PropertyNode[1], m_CategoryNameMap);
		unsigned int subcategoryId = 0;

		std::map<unsigned int, TStringIdMap >::iterator subiter;
		subiter = m_SubCategoryNameMap.find(categoryId);
		if(subiter != m_SubCategoryNameMap.end()) {
			subcategoryId = StringMapToId(PropertyNode[2], subiter->second);
		} else {
			subcategoryId = StringMapToId(PropertyNode[2]);
		}

		TProperty property = GetProperty(functionId, categoryId, subcategoryId);
		property.OutputList = outputList;
		SetProperty(functionId, categoryId, subcategoryId, property);
	}
}

void CDogConfig::UpdateEnableProperty(const CDogConfig::split_vector_type& PropertyNode, const CDogConfig::split_vector_type& Values)
{
	if(1 == PropertyNode.size()) // function = ...
	{
		const unsigned int functionId = StringMapToId(PropertyNode[0], m_DogFunctionNameMap);

		for(size_t i=0; i<Values.size(); ++i)
		{
			if(std::string("all") == Values[i])
			{
				TProperty property = GetProperty(functionId);
				property.bEnable = true;
				SetProperty(functionId, property);
			}
			else if (std::string("none") == Values[i])
			{
				TProperty property = GetProperty(functionId);
				property.bEnable = false;
				SetProperty(functionId, property);
			}
			else
			{
				std::string strCategory;
				bool bEnableSetting = true;
				if('+' == Values[i][0])
				{
					bEnableSetting =  true;
					strCategory = Values[i].substr(1);
				}
				else if('-' == Values[i][0])
				{
					bEnableSetting =  false;
					strCategory = Values[i].substr(1);
				}
				else
				{
					bEnableSetting =  true;
					strCategory = Values[i];
				}

				const unsigned int categoryId = StringMapToId(strCategory, m_CategoryNameMap);
				TProperty property = GetProperty(functionId, categoryId);
				property.bEnable = bEnableSetting;
				SetProperty(functionId, categoryId, property);
			}
		}
	}
	else if (2 == PropertyNode.size()) // function.category = ...
	{
		const unsigned int functionId = StringMapToId(PropertyNode[0], m_DogFunctionNameMap);
		const unsigned int categoryId = StringMapToId(PropertyNode[1], m_CategoryNameMap);

		for(size_t i=0; i<Values.size(); ++i)
		{
			if(std::string("all") == Values[i])
			{
				TProperty property = GetProperty(functionId, categoryId);
				property.bEnable = true;
				SetProperty(functionId, categoryId, property);
			}
			else if (std::string("none") == Values[i])
			{
				TProperty property = GetProperty(functionId, categoryId);
				property.bEnable = false;
				SetProperty(functionId, categoryId, property);
			}
			else
			{
				std::string strSubCategory;
				bool bEnableSetting = true;
				if('+' == Values[i][0])
				{
					bEnableSetting =  true;
					strSubCategory = Values[i].substr(1);
				}
				else if('-' == Values[i][0])
				{
					bEnableSetting =  false;
					strSubCategory = Values[i].substr(1);
				}
				else
				{
					bEnableSetting =  true;
					strSubCategory = Values[i];
				}


				unsigned int subcategoryId = 0;
				std::map<unsigned int, TStringIdMap >::iterator subiter;
				subiter = m_SubCategoryNameMap.find(categoryId);
				if(subiter != m_SubCategoryNameMap.end()) {
					subcategoryId = StringMapToId(strSubCategory, subiter->second);
				} else {
					subcategoryId = StringMapToId(strSubCategory);
				}

				TProperty property = GetProperty(functionId, categoryId, subcategoryId);
				property.bEnable = bEnableSetting;
				SetProperty(functionId, categoryId, subcategoryId, property);
			}
		}
	}
}

//-----------------------------------------------------------------------------
const CDogConfig::TProperty& CDogConfig::GetProperty(unsigned int function)
{
	if(function > m_PropertyTrees.size()) {
		return m_EmptyProperty;
	}

	if(GetMmfUpdateStamp() != m_MmfStamp) {
		UpdateFromMMF();
	}

	return m_PropertyTrees[function].m_Property;
}

const CDogConfig::TProperty& CDogConfig::GetProperty(unsigned int function, unsigned int category)
{
	if(function > m_PropertyTrees.size()) {
		return m_EmptyProperty;
	}

	if(GetMmfUpdateStamp() != m_MmfStamp) {
		UpdateFromMMF();
	}

	const std::map<unsigned int, TCategoryNode> & catNodes = m_PropertyTrees[function].m_CatNodes;
	const std::map<unsigned int, TCategoryNode>::const_iterator catIter = catNodes.find(category);
	if(catIter == catNodes.end()) {
		return m_PropertyTrees[function].m_Property;
	}

	return catIter->second.m_Property;
}

const CDogConfig::TProperty& CDogConfig::GetProperty(unsigned int function, unsigned int category, unsigned int subcategory)
{
	if(function > m_PropertyTrees.size()) {
		return m_EmptyProperty;
	}

	if(GetMmfUpdateStamp() != m_MmfStamp) {
		UpdateFromMMF();
	}

	const std::map<unsigned int, TCategoryNode> & catNodes = m_PropertyTrees[function].m_CatNodes;
 	const std::map<unsigned int, TCategoryNode>::const_iterator catIter = catNodes.find(category);
 	if(catIter == catNodes.end()) {
		return m_PropertyTrees[function].m_Property;
	}

	const std::map<unsigned int, TProperty> & subNodes = catIter->second.m_SubcatNodes;
	const std::map<unsigned int, TProperty>::const_iterator subIter = subNodes.find(subcategory);
	if(subIter == subNodes.end()) {
		return catIter->second.m_Property;
	}

	return subIter->second;
}

//-----------------------------------------------------------------------------
void CDogConfig::SetProperty(unsigned int function, const CDogConfig::TProperty& property)
{
	if(function > m_PropertyTrees.size()) {
		return;
	}
	m_PropertyTrees[function].m_Property = property;

	if(!property.OutputList.empty())
	{
		// update OutputList
		std::map<unsigned int, TCategoryNode> & catNodes = m_PropertyTrees[function].m_CatNodes;
		std::map<unsigned int, TCategoryNode>::iterator iter;
		for(iter = catNodes.begin(); iter!=catNodes.end(); ++iter )
		{
			const unsigned int category = iter->first;
			const TProperty& catProp = GetProperty(function, category);
			if( catProp.OutputList.empty())
			{
				TProperty catPropSetting = catProp;
				catPropSetting.OutputList = property.OutputList;
				SetProperty(function, category, catPropSetting);
			}
		}
	}
}

void CDogConfig::SetProperty(unsigned int function, unsigned int category, const CDogConfig::TProperty& property)
{
	if(function > m_PropertyTrees.size()) {
		return;
	}
	m_PropertyTrees[function].m_CatNodes[category].m_Property = property;

	if(!property.OutputList.empty())
	{
		// update OutputList
		std::map<unsigned int, TProperty> & subNodes = m_PropertyTrees[function].m_CatNodes[category].m_SubcatNodes;
		std::map<unsigned int, TProperty>::const_iterator iter;
		for(iter = subNodes.begin(); iter!=subNodes.end(); ++iter )
		{
			const unsigned int subcategory = iter->first;
			const TProperty& subcatProp = GetProperty(function, category, subcategory);
			if( subcatProp.OutputList.empty())
			{
				TProperty subcatPropSetting = subcatProp;
				subcatPropSetting.OutputList = property.OutputList;
				SetProperty(function, category, subcategory, subcatPropSetting);
			}
		}
	}
}

void CDogConfig::SetProperty(unsigned int function, unsigned int category, unsigned int subcategory, const CDogConfig::TProperty& property)
{
	if(function > m_PropertyTrees.size()) {
		return;
	}

	// in case of set subcategory directly when category is not specified
	const std::map<unsigned int, TCategoryNode> & catNodes = m_PropertyTrees[function].m_CatNodes;
	const std::map<unsigned int, TCategoryNode>::const_iterator catIter = catNodes.find(category);
	if(catIter == catNodes.end()) {
		m_PropertyTrees[function].m_CatNodes[category].m_Property = m_PropertyTrees[function].m_Property;
	}

	m_PropertyTrees[function].m_CatNodes[category].m_SubcatNodes[subcategory] = property;
}

//-----------------------------------------------------------------------------
void CDogConfig::SetTimerType(unsigned int timerType)
{
	m_TimerSetting = timerType;

	UpdateMMFHeader();
}

unsigned int CDogConfig::GetTimerType()
{
	if(GetMmfUpdateStamp() != m_MmfStamp) {
		UpdateFromMMF();
	}

	return m_TimerSetting;
}

//-----------------------------------------------------------------------------
size_t CDogConfig::CopyPropertyToMem(BYTE* pMem, size_t sizeOfMem, unsigned int depth, unsigned int function, unsigned int category, unsigned int subcategory, const TProperty& property)
{
	TMmfPropertyEntry entry;
	entry.depth = depth;
	entry.function = function;
	entry.category = category;
	entry.subcategory = subcategory;
	entry.bEnableFlag = property.bEnable;
	entry.numOfOutput = property.OutputList.size();
	const size_t bytesOfOutputList = entry.numOfOutput*sizeof(unsigned int);
	const size_t bytesOfRecord = sizeof(entry)+bytesOfOutputList;

	if(sizeOfMem < bytesOfRecord) {
		return 0;
	}

	memcpy(pMem, &entry, sizeof(entry));
	if(!property.OutputList.empty()) {
		memcpy(pMem+sizeof(entry), &property.OutputList[0], bytesOfOutputList);
	}

	return bytesOfRecord;
}

void CDogConfig::UpdateMMFHeader()
{
	TMmfHeader mmfHeader;
	mmfHeader = *((TMmfHeader*)m_MMF.GetData());
	mmfHeader.updateStamp = ++mmfHeader.updateStamp;

	mmfHeader.timerSetting = m_TimerSetting;

	memcpy(m_MMF.GetData(), &mmfHeader, sizeof(mmfHeader));
	m_MmfStamp = mmfHeader.updateStamp;
}

void CDogConfig::UpdateToMMF()
{
	TMmfHeader mmfHeader;
	mmfHeader = *((TMmfHeader*)m_MMF.GetData());

	BYTE* pMem = (BYTE*)m_MMF.GetData();
	size_t nSizeOfMem = m_nSizeOfMMF;
	pMem += sizeof(TMmfHeader);
	nSizeOfMem -= sizeof(TMmfHeader);

	size_t numOfEntries = 0;

	for(unsigned int function=EVENT_LOG; function<NUM_OF_DOG_FUNC; ++function)
	{
		size_t bytesOfWrite = CopyPropertyToMem(pMem, nSizeOfMem, 1, function, 0,0, m_PropertyTrees[function].m_Property);
		if(bytesOfWrite)
		{
			++numOfEntries;
			pMem += bytesOfWrite;
		}

		const std::map<unsigned int, TCategoryNode> & catNodes = m_PropertyTrees[function].m_CatNodes;
		std::map<unsigned int, TCategoryNode>::const_iterator catIter;
		for(catIter = catNodes.begin(); catIter!=catNodes.end(); ++catIter )
		{
			const unsigned int category = catIter->first;
			size_t bytesOfWrite = CopyPropertyToMem(pMem, nSizeOfMem, 2, function, category, 0, GetProperty(function, category));
			if(bytesOfWrite)
			{
				++numOfEntries;
				pMem += bytesOfWrite;
				nSizeOfMem -= bytesOfWrite;
			}

			const std::map<unsigned int, TProperty> & subNodes = catIter->second.m_SubcatNodes;
			std::map<unsigned int, TProperty>::const_iterator subIter;
			for(subIter = subNodes.begin(); subIter!=subNodes.end(); ++subIter )
			{
				const unsigned int subcategory = subIter->first;
				size_t bytesOfWrite = CopyPropertyToMem(pMem, nSizeOfMem, 3, function, category, subcategory, GetProperty(function, category, subcategory));
				if(bytesOfWrite)
				{
					++numOfEntries;
					pMem += bytesOfWrite;
					nSizeOfMem -= bytesOfWrite;
				}
			}
		}
	}

	mmfHeader.updateStamp = ++mmfHeader.updateStamp;
	mmfHeader.timerSetting = m_TimerSetting;
	mmfHeader.numOfEntries = numOfEntries;
	mmfHeader.size = pMem - (BYTE*)m_MMF.GetData() - sizeof(mmfHeader);
	memcpy(m_MMF.GetData(), &mmfHeader, sizeof(mmfHeader));

	m_MmfStamp = mmfHeader.updateStamp;
}

void CDogConfig::UpdateFromMMF()
{
	//reset
	m_PropertyTrees.clear();
	m_PropertyTrees.resize(NUM_OF_DOG_FUNC);

	TMmfHeader mmfHeader;
	mmfHeader = *((TMmfHeader*)m_MMF.GetData());

	m_TimerSetting = mmfHeader.timerSetting;

	BYTE* pMem = (BYTE*)m_MMF.GetData();
	size_t nSizeOfMem = m_nSizeOfMMF;
	pMem += sizeof(TMmfHeader);

	for(size_t i=0; i<mmfHeader.numOfEntries; ++i)
	{
		TMmfPropertyEntry entry = *(TMmfPropertyEntry*)pMem;
		const size_t bytesOfOutputList = entry.numOfOutput*sizeof(unsigned int);

		TProperty property;
		property.bEnable = entry.bEnableFlag;
		if(entry.numOfOutput) {
			property.OutputList.insert(property.OutputList.begin(), pMem+sizeof(entry), pMem+bytesOfOutputList);
		}

		switch(entry.depth)
		{
		case 1:
			SetProperty(entry.function, property);
			break;
		case 2:
			SetProperty(entry.function, entry.category, property);
			break;
		case 3:
			SetProperty(entry.function, entry.category, entry.subcategory, property);
			break;
		default:
			break;
		}

		const size_t bytesOfRecord = sizeof(entry)+bytesOfOutputList;
		pMem += bytesOfRecord;
	}

	m_MmfStamp = mmfHeader.updateStamp;
}


} // namespace dog
