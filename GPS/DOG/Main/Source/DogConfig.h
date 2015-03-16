#pragma once

#include "MemMapFile.h"
#include <boost/utility.hpp>

#include <string>
#include <vector>
#include <map>

// namespace Diagnosis-of-GPS
namespace dog
{

/**
 * CDogConfig accepts and parses configuration setting from a file or string.
 * CDogConfig will keep configuration on a memory mapping file. Therefore,
 * client program and DOG UI can share the same setting.
 *
 * There brief syntax description below:
 *
 * function= category1, [category2,] [category3]
 * function.output = destination1, [destination2,] [destination3]
 *
 * function.category = sub-category1, [sub-category2,] [sub-category3]
 * function.category.output = destination1, [destination2,] [destination3]
 *
 * ====================================================================
 * supported functions are:   profiler, log
 *
 */
class CDogConfig : public boost::noncopyable
{
public:
	CDogConfig();
	~CDogConfig();

	enum FunctionEnum
	{
		FUNCTION_NONE = 0,
		EVENT_LOG,
		PROFILER,
		NUM_OF_DOG_FUNC
	};

	enum OutputEnum
	{
		DOG_VIEW = 0,
		DOG_LOG_FILE,
		DEBUG_VIEW,
	};

	struct TProperty
	{
		bool bEnable;
		std::vector<unsigned int> OutputList;
		TProperty() : bEnable(false) {}
	};

	/// Clears all setting
	void Reset();

	/// Parses configuration setting from a file
	bool ReadFromFile(const char* filename);

	/// Parses configuration setting from a string
	bool ReadFromString(const char* szConfigString);

	/// Returns parsing error message
	typedef std::vector<std::string> TMessageList;
	const TMessageList& GetErrorMessage() { return m_ErrorMessage; }

	/// Returns the property with the specified function, category, subcategory.
	const TProperty& GetProperty(unsigned int function);
	const TProperty& GetProperty(unsigned int function, unsigned int category);
	const TProperty& GetProperty(unsigned int function, unsigned int category, unsigned int subcategory);

	/// Set/Get timer type
	/// Timer types are defined in IDOGSessionConfig
	void SetTimerType(unsigned int timerType);
	unsigned int GetTimerType();

protected:
	void InitConfigStringMapTable();

	typedef std::map<std::string, unsigned int> TStringIdMap;
	TStringIdMap m_DogFunctionNameMap;
	TStringIdMap m_OutputNameMap;
	TStringIdMap m_CategoryNameMap;
	std::map<unsigned int, TStringIdMap > m_SubCategoryNameMap;

protected:
	bool ReadLine(const std::string& line);
	std::string m_ReadLineErrMsg;
	TMessageList m_ErrorMessage;

	typedef std::vector< std::string > split_vector_type;
	void CDogConfig::UpdateOutputProperty(const split_vector_type& PropertyNode, const split_vector_type& Values);
	void CDogConfig::UpdateEnableProperty(const split_vector_type& PropertyNode, const split_vector_type& Values);

	void SetProperty(unsigned int function, const TProperty& property);
	void SetProperty(unsigned int function, unsigned int category, const TProperty& property);
	void SetProperty(unsigned int function, unsigned int category, unsigned int subcategory, const TProperty& property);

	const TProperty m_EmptyProperty;

	struct TCategoryNode
	{
		TProperty m_Property;
		std::map<unsigned int, TProperty> m_SubcatNodes;
	};

	struct TRoot
	{
		TProperty m_Property;
		std::map<unsigned int, TCategoryNode> m_CatNodes;
	};

	std::vector<TRoot> m_PropertyTrees;

protected:
	CMemMapFile m_MMF;
	const size_t m_nSizeOfMMF;
	unsigned int m_TimerSetting;

	/// GetProperty() will check m_MmfStamp and TMmfHeader.updateStamp.
	/// If they are different, it means that the current configuration doesn't
	/// be synchronized with the memory mapping configuration.
	unsigned int m_MmfStamp;

	/// Header of the memory mapping configuration
	struct TMmfHeader
	{
		unsigned int version;      ///< version of the configuration setting
		unsigned int updateStamp;  ///< monotoned increased when the memory mapping configuration is changed
		unsigned int timerSetting; ///< timer type setting
		unsigned int reserved[64]; ///< reserved for future use
		size_t numOfEntries;       ///< Number of TMmfPropertyEntry
		size_t size;               ///< Size of memory allocated for the memory mapping configuration

		TMmfHeader() : version(0), updateStamp(0), timerSetting(0), numOfEntries(0), size(0) {}
	};

	/// Entry of the memory mapping configuration
	struct TMmfPropertyEntry
	{
		unsigned int depth;
		unsigned int function;
		unsigned int category;
		unsigned int subcategory;
		bool bEnableFlag;
		size_t numOfOutput;
		//unsigned int outputList[];

		TMmfPropertyEntry() : depth(0), function(0), category(0), subcategory(0), bEnableFlag(0), numOfOutput(0) {}
	};

	/// Update configuration setting to memory mapping file
	/// This method will increases updateStamp filed of TMmfHeader
	inline void UpdateToMMF();

	/// Update configuration setting from memory mapping file
	/// UpdateFromMMF will update m_MmfStamp to TMmfHeader.updateStamp
	inline void UpdateFromMMF();

	inline void UpdateMMFHeader();

	inline size_t CopyPropertyToMem(BYTE* pMem, size_t sizeOfMem, unsigned int depth, unsigned int function, unsigned int category, unsigned int subcategory, const TProperty& property);

	unsigned int GetMmfUpdateStamp()
	{
		const size_t offsetof_updateStamp = ((size_t)(&((TMmfHeader *)0)->updateStamp));
		const BYTE* pMem = (BYTE*)m_MMF.GetData() + offsetof_updateStamp;

		const unsigned int updateStamp = *(unsigned int *)pMem;
		return updateStamp;
	}
};


} // namespace dog
