#include "stdafx.h"
#include "SubtitleRendererFactory.h"
#include "SRTSubtitleRenderer.h"
#include "NoSubtitleRenderer.h"

#include "DrawingEffectRendererFactory.h"

using namespace SubtitleCore;

SubtitleRendererFactory* SubtitleRendererFactory::instance = 0;

void SubtitleRendererFactory::InitializeSingleton()
{
	if (!instance)
	{
		instance = new SubtitleRendererFactory();

		// Also initialize DrawingEffectRenderer factory.
		DrawingEffectsRendererFactory::InitializeSingleton();
	}
}

SubtitleRendererFactory::SubtitleRendererFactory() 
	: m_SubCoreConfig(nullptr)
{
	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), 
		reinterpret_cast<IUnknown**>(&m_DWriteFactory))))
	{
		m_DWriteFactory = nullptr;
	}
}

SubtitleRendererFactory::~SubtitleRendererFactory()
{
	m_SubCoreConfig.reset();
	m_DWriteFactory->Release();
}

void SubtitleRendererFactory::SetSubtitleCoreConfig(const SubtitleCoreConfigurationData& config)
{
	Concurrency::reader_writer_lock::scoped_lock scoped_lock(m_RWLockFactory);

	m_SubCoreConfig.reset();
	m_SubCoreConfig = std::make_shared<SubtitleCoreConfigurationData>(config);
}

std::shared_ptr<ISubtitleRenderer> SubtitleRendererFactory::CreateSubtitleRenderer(SubtitleType type, const VideoInfo& targetVideoFrameInfo) const
{
	// If the user hasn't passed us all the data we need, return nullptr.
	if (m_SubCoreConfig)
	{
		switch(type)
		{
		case SBT_SRT:
			{
				return std::make_shared<SRTSubtitleRenderer>(*m_SubCoreConfig, targetVideoFrameInfo, m_DWriteFactory);
			}
		case SBT_NONE:
			{
				return std::make_shared<NoSubtitleRenderer>();
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