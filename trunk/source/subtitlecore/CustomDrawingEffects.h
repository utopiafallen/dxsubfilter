// This header contains all custom drawing effects.

#ifndef CUSTOMDRAWINGEFFECTS_H
#define CUSTOMDRAWINGEFFECTS_H
#pragma once

// Global forward declarations
struct ID2D1SolidColorBrush;

namespace SubtitleCore
{
	// Simple base interface for drawing effects to implement so that they can be aggregated. Exposes the ability
	// to determine what type of effect an effect is.
	class ICustomDrawingEffect
	{
	public:
		enum DrawingEffectType { DET_OUTLINEFILL, DET_OUTLINEONLY };

		DrawingEffectType GetType() const { return m_EffectType; }

	protected:
		DrawingEffectType m_EffectType;
	};

	// Aggregates multiple drawing effects together. This is what is passed to the CustomTextRenderer to control
	// the text rendering. Effects are rendered in the order they occur the list.
	class __declspec(uuid("7A930731-751B-46E8-9A01-BE89C111F636")) DrawingEffectsCollection : public IUnknown
	{
	public:
		DrawingEffectsCollection() 
			: m_uRefCount(0) 
		{}

		//===================================================
		// IUnknown implementations
		//===================================================
		unsigned long STDMETHODCALLTYPE AddRef()
		{
			return InterlockedIncrement(&m_uRefCount);
		}

		unsigned long STDMETHODCALLTYPE Release()
		{
			unsigned long newCount = InterlockedDecrement(&m_uRefCount);
			if (newCount == 0)
			{
				delete this;
				return 0;
			}
			
			return newCount;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(
			IID const& riid,
			void** ppvObject
			)
		{
			if (__uuidof(DrawingEffectsCollection) == riid)
			{
				*ppvObject = this;
			}
			else if (__uuidof(IUnknown) == riid)
			{
				*ppvObject = this;
			}
			else
			{
				*ppvObject = nullptr;
				return E_FAIL;
			}

			AddRef();

			return S_OK;
		}

	public:		// Data
		std::vector<std::shared_ptr<ICustomDrawingEffect>> m_Effects;

	private:
		unsigned long m_uRefCount;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//										Drawing effects											//
	//////////////////////////////////////////////////////////////////////////////////////////////////

	struct OutlineAndFillDrawingEffect : public ICustomDrawingEffect
	{
		ID2D1SolidColorBrush* m_pFillBrush;
		ID2D1SolidColorBrush* m_pOutlineBrush;
		float m_fBorderWidth;

		OutlineAndFillDrawingEffect(ID2D1SolidColorBrush* fillBrush, ID2D1SolidColorBrush* outlineBrush, float borderWidth) 
			: m_pFillBrush(fillBrush)
			, m_pOutlineBrush(outlineBrush)
			, m_fBorderWidth(borderWidth)
		{ m_EffectType = DET_OUTLINEFILL; }
	};

	struct OutlineOnlyDrawingEffect : public ICustomDrawingEffect
	{
		ID2D1SolidColorBrush* m_pOutlineBrush;
		float m_fBorderWidth;

		OutlineOnlyDrawingEffect(ID2D1SolidColorBrush* outlineBrush, float borderWidth)
			: m_pOutlineBrush(outlineBrush)
			, m_fBorderWidth(borderWidth)
		{ m_EffectType = DET_OUTLINEONLY; }
	};
};

#endif