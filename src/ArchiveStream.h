#pragma once

#include "Archive.h"
#include "ArchiveSource.h"
#include "WritableSourceStream.h"

namespace Zip {

	template<typename InputStream, typename OutputStream>
	class ArchiveStream : public ArchiveSource {
	public:

		ArchiveStream(InputStream inputStreamPtr, OutputStream outputStreamPtr) :
			_src(inputStreamPtr, outputStreamPtr),
			ArchiveSource(ZIP_CREATE)
		{}

	protected:

		virtual zip_source_callback _getSourceDispath()
		{
			return &WritableSourceStream<InputStream, OutputStream>::dispatch;
		}

		virtual void* _getSourcePtr()
		{
			return &_src;
		}

	private:

		WritableSourceStream<InputStream, OutputStream> _src;

	};

}