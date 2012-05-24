#include "stdafx.h"
#include "SubtitleRendererFactory.h"
#include "SRTSubtitleRenderer.h"

using namespace SubtitleCore;

SubtitleRendererFactory* SubtitleRendererFactory::instance = new SubtitleRendererFactory();

SubtitleRendererFactory::SubtitleRendererFactory() 
	: m_SubCoreConfig(nullptr)
	, m_VideoInfo(nullptr)
{
	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), 
		reinterpret_cast<IUnknown**>(m_DWriteFactory))))
	{
		m_DWriteFactory = nullptr;
	}
}

SubtitleRendererFactory::~SubtitleRendererFactory()
{
	m_SubCoreConfig.reset();
	m_VideoInfo.reset();
	m_SubtitleRendererCache.clear();
	m_DWriteFactory->Release();
}

void SubtitleRendererFactory::SetSubtitleCoreConfig(SubtitleCoreConfigurationData& config)
{
	Concurrency::reader_writer_lock::scoped_lock scoped_lock(m_RWLockFactory);

	m_SubCoreConfig.reset();
	m_SubCoreConfig = std::make_shared<SubtitleCoreConfigurationData>(config);
}

void SubtitleRendererFactory::SetVideoInfo(VideoInfo& vidInfo)
{
	Concurrency::reader_writer_lock::scoped_lock scoped_lock(m_RWLockFactory);

	m_VideoInfo.reset();
	m_VideoInfo = std::make_shared<VideoInfo>(vidInfo);
}

std::shared_ptr<ISubtitleRenderer> SubtitleRendererFactory::CreateSubtitleRenderer(SubtitleType type, bool bUniqueInstance)
{
	// If the user hasn't passed us all the data we need, return nullptr.
	if (m_SubCoreConfig && m_VideoInfo)
	{
		// We don't expect SubtitleRenderer creation to be a common and frequen operation amongst
		// threads so it's simpler to just lock down the whole process.
		Concurrency::reader_writer_lock::scoped_lock scoped_lock(m_RWLockFactory);

		// Relies on the fact that enums are guaranteed to be monotonically increasing to resize
		// the cache if we haven't encountered this subtitle type before.
		if (m_SubtitleRendererCache.size() < static_cast<size_t>(type))
		{
			m_SubtitleRendererCache.resize(type);
		}

		switch(type)
		{
		case SBT_SRT:
			{
				if (bUniqueInstance)
				{
					std::shared_ptr<ISubtitleRenderer> result = std::make_shared<SRTSubtitleRenderer>(*m_SubCoreConfig, *m_VideoInfo, m_DWriteFactory);

					m_SubtitleRendererCache[type].push_back(result);
					return result;
				}
				else
				{
					if (m_SubtitleRendererCache[type].size() == 0)
					{
						m_SubtitleRendererCache[type].push_back(std::make_shared<SRTSubtitleRenderer>(*m_SubCoreConfig, *m_VideoInfo, m_DWriteFactory));
					}
					return m_SubtitleRendererCache[type][0];
				}
			}
		default:
			{
				return nullptr;
			}
		}
	}
	else
	{
		return nullptr;
	}
}