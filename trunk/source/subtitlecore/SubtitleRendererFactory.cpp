#include "stdafx.h"
#include "SubtitleRendererFactory.h"
#include "SRTSubtitleRenderer.h"

using namespace SubtitleCore;

SubtitleRendererFactory* SubtitleRendererFactory::instance = nullptr;

SubtitleRendererFactory::SubtitleRendererFactory() 
	: m_SubCoreConfig(nullptr)
	, m_VideoInfo(nullptr)
{

}

SubtitleRendererFactory::~SubtitleRendererFactory()
{
	m_SubCoreConfig.reset();
	m_VideoInfo.reset();
	m_SubtitleRendererCache.clear();
}

void SubtitleRendererFactory::SetSubtitleCoreConfig(SubtitleCoreConfigurationData& config)
{
	m_SubCoreConfig.reset();
	m_SubCoreConfig = std::make_shared<SubtitleCoreConfigurationData>(config);
}

void SubtitleRendererFactory::SetVideoInfo(VideoInfo& vidInfo)
{
	m_VideoInfo.reset();
	m_VideoInfo = std::make_shared<VideoInfo>(vidInfo);
}

std::shared_ptr<ISubtitleRenderer> SubtitleRendererFactory::CreateSubtitleRenderer(SubtitleType type, bool bUniqueInstance)
{
	// If the user hasn't passed us all the data we need, return nullptr.
	if (m_SubCoreConfig && m_VideoInfo)
	{
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
					std::shared_ptr<ISubtitleRenderer> result = std::make_shared<SRTSubtitleRenderer>(*m_SubCoreConfig, *m_VideoInfo);

					m_SubtitleRendererCache[type].push_back(result);
					return result;
				}
				else
				{
					if (m_SubtitleRendererCache[type].size() == 0)
					{
						m_SubtitleRendererCache[type].push_back(std::make_shared<SRTSubtitleRenderer>(*m_SubCoreConfig, *m_VideoInfo));
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