#include "stdafx.h"

namespace SubtitleCore
{
	class __declspec(uuid("7A930731-751B-46E8-9A01-BE89C111F636")) DrawingEffectsCollection : public IUnknown
	{
	public:

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

	private:
		unsigned long m_uRefCount;
	};
};