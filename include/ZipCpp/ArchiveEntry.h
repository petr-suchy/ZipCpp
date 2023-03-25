#pragma once

#include <stdexcept>
#include <vector>

#include "ReadableEntryStream.h"
#include "WritableEntryStream.h"

namespace Zip {

	class ArchiveEntry {
	public:

		typedef std::function<
			ReadableEntryStream::SharedPtr(zip_int64_t)
		> OpenForReading;

		typedef std::function<
			WritableEntryStream::SharedPtr(zip_int64_t)
		> OpenForWriting;

		ArchiveEntry(
			zip_int64_t entryIndex,
			OpenForReading openForReading,
			OpenForWriting openForWriting
		) :
			_entryIndex(entryIndex),
			_openForReading(openForReading),
			_openForWriting(openForWriting)
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

			return _openForReading(_entryIndex);
		}

		WritableEntryStream::SharedPtr openForWriting()
		{
			return _openForWriting(_entryIndex);
		}

		template<typename T>
		void exportTo(T& os)
		{
			copyStream(openForReading(), &os);
		}

		template<typename T>
		void importFrom(T& is)
		{
			copyStream(&is, openForWriting());
		}

		template<typename T>
		T& operator>> (T& os)
		{
			exportTo(os);
			return os;
		}

		template<typename T>
		T& operator<< (T& is)
		{
			importFrom(is);
			return is;
		}

	private:

		zip_int64_t _entryIndex;
		OpenForReading _openForReading;
		OpenForWriting _openForWriting;

		template<typename IStream, typename OStream>
		static void copyStream(IStream is, OStream os)
		{
			std::vector<char> buf(4096);

			if (!is->good()) {
				throw std::logic_error("input stream is not ready for reading");
			}

			if (!os->good()) {
				throw std::logic_error("output stream is not ready for writing");
			}

			do {

				is->read(buf.data(), buf.size());

				if (is->fail() && !is->eof()) {
					throw std::runtime_error(
						"failed to read data from input stream"
					);
				}

				if (is->gcount() > 0) {

					os->write(buf.data(), is->gcount());

					if (os->fail()) {
						throw std::runtime_error(
							"failed to write data to output stream"
						);
					}

				}

			} while (!is->eof());
		}

	};

}