#pragma once

#include <stdexcept>
#include <functional>

#include "ReadableEntryStream.h"

namespace Zip {

	class ArchiveEntry {
	public:

		typedef std::function<
			ReadableEntryStream(zip_int64_t)
		> OpenEntryFunc;

		ArchiveEntry() :
			_entryIndex(0)
		{}

		ArchiveEntry(
			zip_int64_t entryIndex,
			OpenEntryFunc openEntryFunc
		) :
			_entryIndex(entryIndex),
			_openEntryFunc(openEntryFunc)
		{}

		zip_int64_t getIndex()
		{
			return _entryIndex;
		}

		bool exists() const
		{
			return _entryIndex >= 0;
		}

		explicit operator bool() const
		{
			return exists();
		}

		ReadableEntryStream openForReading()
		{
			if (!exists()) {
				throw std::logic_error("archive file entry not found");
			}

			return _openEntryFunc(_entryIndex);
		}

		template<typename T>
		void copyToStream(T& writableStream)
		{
			Zip::ReadableEntryStream rs = openForReading();

			char readBuff[512];
			rs.read(readBuff, sizeof(readBuff));

			while (rs.gcount() > 0) {
				writableStream.write(readBuff, rs.gcount());
				rs.read(readBuff, sizeof(readBuff));
			}
		}

		template<typename T>
		T& operator>> (T& writableStream)
		{
			copyToStream(writableStream);
			return writableStream;
		}

	private:

		zip_int64_t _entryIndex;
		OpenEntryFunc _openEntryFunc;

	};

}