#ifdef FLAT_PROFILER_ENABLED

#include "profiler/profiler.h"
#include "profiler/binarywriter.h"
#include "profiler/profilersection.h"

#include "debug/assert.h"
#include "memory/memory.h"

namespace flat
{
namespace profiler
{

void Profiler::startSection(const ProfilerSection* profilerSection)
{
	m_currentSections.push_back(profilerSection);
	if (m_binaryWriter != nullptr && m_shouldWrite)
	{
		m_binaryWriter->pushSection(profilerSection->m_name, profilerSection->m_startTime);
	}
}

void Profiler::endSection(const ProfilerSection* profilerSection)
{
	FLAT_ASSERT(profilerSection == m_currentSections.back());
	m_currentSections.pop_back();
	if (m_binaryWriter != nullptr)
	{
		if (m_shouldWrite)
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
			m_binaryWriter->popSection(end);
		}
		else if (m_currentSections.size() == 0)
		{
			m_shouldWrite = true;
		}
	}
}

void Profiler::startRecording()
{
	m_shouldWrite = false;
	m_binaryWriter = new BinaryWriter("profile.fp");
}

void Profiler::stopRecording()
{
	popStartedSections();
	m_binaryWriter->writeSectionNames();
	FLAT_DELETE(m_binaryWriter);
}

void Profiler::popStartedSections()
{
	FLAT_ASSERT(m_binaryWriter != nullptr);
	std::chrono::time_point<std::chrono::high_resolution_clock> interruptedTime = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < m_currentSections.size(); ++i)
	{
		m_binaryWriter->popSection(interruptedTime);
	}
}

} // profiler
} // flat

#endif // FLAT_PROFILER_ENABLED