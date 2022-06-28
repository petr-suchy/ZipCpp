#pragma once

#include "Archive.h"
#include "ArchiveSource.h"
#include "WritableSourceStream.h"
#include "NullInputStream.h"

namespace Zip {

	template<typename OutputStream>
	class OutputArchiveStream : public ArchiveSource {
	public:

		OutputArchiveStream(OutputStream outputStreamPtr) :
			_src(&nullInputStream, outputStreamPtr),
			ArchiveSource(ZIP_CREATE)
		{}

	protected:

		virtual zip_source_callback _getSourceDispath()
		{
			return &WritableSourceStream<NullInputStream*, OutputStream>::dispatch;
		}

		virtual void* _getSourcePtr()
		{
			return &_src;
		}

	private:

		NullInputStream nullInputStream;
		WritableSourceStream<NullInputStream*, OutputStream> _src;

	};

}