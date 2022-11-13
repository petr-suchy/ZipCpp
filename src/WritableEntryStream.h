#pragma once

#include "ZipHandle.h"

#include <functional>
#include <sstream>
#include <string>

namespace Zip {

	class WritableEntryStream {
	public:

		typedef std::shared_ptr<WritableEntryStream> SharedPtr;
		typedef std::weak_ptr<WritableEntryStream> WeakPtr;

		WritableEntryStream(
			ZipHandle::WeakPtr handle,
			std::weak_ptr<std::stringstream> ostream
		) :
			_handle(handle),
			_ostream(ostream)
		{}

		bool fail()
		{
			bool fail = true;

			if (auto os = _ostream.lock()) {
				fail = os->fail();
			}

			return fail;
		}

		bool good()
		{
			bool good = false;

			if (auto os = _ostream.lock()) {
				good = os->good();
			}

			return good;
		}

		void clear()
		{
			if (auto os = _ostream.lock()) {
				os->clear();
			}
		}

		void write(const char* buf, std::streamsize nbytes)
		{
			if (auto os = _ostream.lock()) {
				os->write(buf, nbytes);
			}
		}

		void flush()
		{
			if (auto os = _ostream.lock()) {
				os->flush();
			}
		}

	private:

		ZipHandle::WeakPtr _handle;
		std::weak_ptr<std::stringstream> _ostream;

	};

}