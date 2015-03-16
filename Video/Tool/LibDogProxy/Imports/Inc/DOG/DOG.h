#ifndef _DIAGNOSIS_OF_GPS_H_
#define _DIAGNOSIS_OF_GPS_H_

#include <windows.h>

#ifndef STDCALL
#define STDCALL __stdcall
#endif

// namespace Diagnosis-of-GPS
namespace dog
{

/// The base class of DOG event. Any new DOG event should be derived from the base class.
struct DogEvent
{
	DWORD           Category;     // Major event category: e.g. demux, video, audio, ...
	DWORD           SubCategory;  // Sub-category: e.g. for audio, ac3, dts, dd+, ...
	DWORD           Id;           // Event Id, which identifies a unique event.
	DWORD			DataType;     // Event data type
	DWORD           Size;         // Size of entire record

	// Revision of the event data type.
	// The new version of data type must be backward compatible the old event type,
	// which means the new version event must be extended from the old event.
	// Therefore, the size of new version event will be larger than its base event,
	// and DOG can distinguish the event version by its size.

	// DOG will fill the following fields automatically
	ULONG           ThreadId;  // Thread Id
	ULONG           ProcessId; // Process Id
	mutable DWORD	Index;     // Index number of event. It's mono increased when an event files.
	LARGE_INTEGER   TimeStamp; // Time in 100-nanoseconds when event happens
	mutable LARGE_INTEGER BaseTime; // For TimeStamp base reference adjustment

	DogEvent() : Category(0), SubCategory(0), Id(0), DataType(0), Size(sizeof(DogEvent)), ThreadId(0), ProcessId(0), Index(0) {}

	DogEvent(DWORD cat, DWORD subcat, DWORD id) :
		Category(cat), SubCategory(subcat), Id(id), DataType(0), Size(sizeof(DogEvent)), ThreadId(0), ProcessId(0), Index(0) {}

	DogEvent(DWORD cat, DWORD subcat, DWORD id, DWORD type, DWORD size) :
		Category(cat), SubCategory(subcat), Id(id), DataType(type), Size(size),
		ThreadId(0), ProcessId(0), Index(0) {}
};


/// A DOG event consumer needs to implement IEventListener and register the DOG event listener to DOG.
/// DOG will invoke NotifyEvent() when a new DOG event is emitted.
class IEventListener
{
public:
	/// Notifies a new DOG event coming.
	virtual void STDCALL NotifyEvent(const DogEvent& event) = 0;
};


/// Interface for event provider and consumer
class IEventTracing : public IUnknown
{
public:
	/// Setup DOG system by the specified configuration file.
	virtual void STDCALL SetupDogConfigByFile(const char* filename) = 0;

	/// Setup DOG system by the specified configuration string.
	virtual void STDCALL SetupDogConfigByString(const char* szConfigString) = 0;

	/// Returns whether the category.subcategory is enabled
	virtual bool STDCALL GetEnableFlag(DWORD dwCategory, DWORD dwSubCategory) = 0;

	/// Writes a DOG event.
	virtual void STDCALL WriteEvent(const DogEvent& dogEvent) = 0;

	/// Registers a dog event listener
	virtual void STDCALL RegisterListener(IEventListener* listener) = 0;

	/// Removes the specified dog event listener
	virtual void STDCALL UnregisterListener(IEventListener* listener) = 0;
};

// {B7BE5C2F-9C28-4d32-843A-02CEEEBAD201}
static const GUID IID_IEventTracing =
{ 0xb7be5c2f, 0x9c28, 0x4d32, { 0x84, 0x3a, 0x2, 0xce, 0xee, 0xba, 0xd2, 0x1 } };



/// Interface for event consumer
class IEventConsumer : public IUnknown
{
public:
	/// event consumer's status

	/// Gets the indices of available data range of the DOG session
	/// @param [out] pUpperBound
	/// @param [out] pLowerBound
	/// @return:
	///      S_OK if it succeeds
	///      E_FAIL if it fails
	virtual HRESULT STDCALL GetAvailableRange(LONGLONG* pUpperBound, LONGLONG* pLowerBound) = 0;

	/// Returns the current read index of the DOG session consumer
	virtual LONGLONG STDCALL GetReadIndex() = 0;

	/// Sets a specified read index to the DOG session consumer
	virtual LONGLONG STDCALL SetReadIndex(LONGLONG readIdx) = 0;

	/// Returns number of missing events of the DOG session consumer
	virtual DWORD STDCALL GetMissCount() = 0;
};

// {089E197B-B881-4800-9695-F985AE461909}
static const GUID IID_IEventConsumer =
{ 0x89e197b, 0xb881, 0x4800, { 0x96, 0x95, 0xf9, 0x85, 0xae, 0x46, 0x19, 0x9 } };


/// Interface for DOG session configuration
class IDOGSessionConfig : public IUnknown
{
public:
	/// DOG session will give time stamp to events. However, different timers have different
	/// time resolution and overhead. DOG clients can set their perfered timer type.
	enum TimerType
	{
		eTimeGetTime = 0, ///< low overhead, low-resolution
		eFreqTimer,       ///< QueryPerformanceFrequency takes 1.927E-05 seconds as API overhead
		eRdtscTimer,      ///< CPU RDTSC instruction, high-resolution, low overhead
	};

	virtual void STDCALL SetTimerType(TimerType timerType) = 0;
	virtual TimerType STDCALL GetTimerType() = 0;
};

// {32F31261-AF71-4c16-9250-D41C5EB27E28}
static const GUID IID_IDOGSessionConfig =
{ 0x32f31261, 0xaf71, 0x4c16, { 0x92, 0x50, 0xd4, 0x1c, 0x5e, 0xb2, 0x7e, 0x28 } };



} // namespace dog

#endif // _DIAGNOSIS_OF_GPS_H_
