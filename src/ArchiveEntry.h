#pragma once

#include <stdexcept>
#include <functional>
#include <vector>

#include "ReadableEntryStream.h"

namespace Zip {

	class ArchiveEntry {
	public:

		typedef std::function<
			ReadableEntryStream::SharedPtr(zip_int64_t)
		> OpenEntryFunc;

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

		ReadableEntryStream::SharedPtr openForReading()
		{
			if (!exists()) {
				throw std::logic_error("archive file entry not found");
			}

			return _openEntryFunc(_entryIndex);
		}

		template<typename T>
		void copyToStream(T& os)
		{
			Zip::ReadableEntryStream::SharedPtr is = openForReading();
			std::vector<char> buf(4096);

			if (!is->good()) {
				throw std::logic_error("input stream is not ready");
			}

			if (!os.good()) {
				throw std::logic_error("output stream is not ready");
			}

			do {

				is->read(buf.data(), buf.size());

				if (is->fail()) {
					throw std::runtime_error(
						"failed to read data from entry input stream"
					);
				}

				if (is->gcount() > 0) {

					os.write(buf.data(), is->gcount());

					if (is->fail()) {
						throw std::runtime_error(
							"failed to write data to output stream"
						);
					}

				}

			} while (!is->eof());

		}

		template<typename T>
		T& operator>> (T& os)
		{
			copyToStream(os);
			return os;
		}

	private:

		zip_int64_t _entryIndex;
		OpenEntryFunc _openEntryFunc;

	};

}