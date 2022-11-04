#pragma once

#include "SourceStream.h"

namespace Zip {

	class ZipHandle {
	public:

		typedef std::shared_ptr<ZipHandle> SharedPtr;
		typedef std::weak_ptr<ZipHandle> WeakPtr;

		bool isOpen() { return _zipPtr != nullptr; }
		bool isSaved() { return _isSaved; }
		zip_t* get() { return _zipPtr; }

		ZipHandle(zip_t* zipPtr) :
			_zipPtr(zipPtr),
			_isSaved(false)
		{}

		~ZipHandle()
		{
			if (isOpen()) {
				// close the archive without writing changes to disk
				zip_discard(_zipPtr);
			}
		}

		void saveAndClose()
		{
			if (isOpen()) {

				// write changes to the disk and close the archive
				int result = zip_close(_zipPtr);

				if (result == -1) {

					throw std::runtime_error(
						std::string("can't close zip archive -> ")
							+ zip_strerror(_zipPtr)
					);

				}

				_zipPtr = nullptr;
				_isSaved = true;
			}
		}

		void attachSourceForSaving(SourceStream::SharedPtr srcPtr)
		{
			_attachedSourcesForSaving.push_back(srcPtr);
		}

	private:

		zip_t* _zipPtr;

		std::vector<
			SourceStream::SharedPtr
		> _attachedSourcesForSaving;

		bool _isSaved;

	};

}