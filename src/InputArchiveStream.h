#pragma once

#include "Archive.h"
#include "ArchiveSource.h"
#include "SeekableSourceStream.h"

namespace Zip {

	template<typename InputStream>
	class InputArchiveStream : public ArchiveSource {
	public:

		InputArchiveStream(InputStream inputStreamPtr) :
			_src(inputStreamPtr),
			ArchiveSource(ZIP_RDONLY)
		{}

	protected:

		virtual zip_source_callback _getSourceDispath()
		{
			return &SeekableSourceStream<InputStream>::dispatch;
		}

		virtual void* _getSourcePtr()
		{
			return &_src;
		}

	private:

		SeekableSourceStream<InputStream> _src;

	};

}