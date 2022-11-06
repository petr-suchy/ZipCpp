#pragma once

#include "SourceStream.h"
#include "ZipFileHandle.h"

#include <map>

namespace Zip {

	class ZipHandle {
	public:

		typedef zip_t* RawPtr;
		typedef std::shared_ptr<ZipHandle> SharedPtr;
		typedef std::weak_ptr<ZipHandle> WeakPtr;

		ZipHandle(RawPtr zipPtr = nullptr) :
			_zipPtr(zipPtr),
			_hasBeenSaved(false)
		{}

		~ZipHandle()
		{
			if (isOpen()) {
				// close all open files before closing the archive
				_openFiles.clear();
				// close the archive without saving changes
				zip_discard(_zipPtr);
			}
		}

		bool isOpen()
		{
			return _zipPtr != nullptr;
		}

		bool hasBeenSaved()
		{
			return _hasBeenSaved;
		}

		RawPtr get()
		{
			if (!isOpen()) {
				throw std::logic_error("archive is not open");
			}

			return _zipPtr;
		}

		// discards changes and closes the archive
		void discardAndClose()
		{
			if (!isOpen()) {

				if (hasBeenSaved()) {
					throw std::logic_error("archive has been closed");
				}

				return;
			}

			// close all open files before closing the archive
			_openFiles.clear();
			// close the archive
			zip_discard(_zipPtr);

			_zipPtr = nullptr;
		}

		// saves changes and closes the archive
		void saveAndClose()
		{
			if (!isOpen()) {

				if (!hasBeenSaved()) {
					throw std::logic_error("archive has been closed");
				}

				return;
			}

			// close all open files before closing the archive
			_openFiles.clear();

			// save changes and close the archive
			int result = zip_close(_zipPtr);

			if (result == -1) {

				throw std::runtime_error(
					std::string("cannot close zip archive -> ")
						+ zip_strerror(_zipPtr)
				);

			}

			_zipPtr = nullptr;
			_hasBeenSaved = true;
		}

		void attachSourceForSaving(SourceStream::SharedPtr srcPtr)
		{
			_attachedSourcesForSaving.push_back(srcPtr);
		}

		ZipFileHandle::SharedPtr openEntry(zip_int64_t entryIndex)
		{
			zip_file_t* zipFilePtr = zip_fopen_index(
				get(),
				entryIndex,
				0
			);

			if (!zipFilePtr) {

				throw std::runtime_error(
					std::string("cannot open archive entry for reading -> ")
						+ zip_strerror(get())
				);

			}

			auto openFile = std::make_shared<
				ZipFileHandle
			>(zipFilePtr);

			_openFiles[zipFilePtr] = openFile;

			return openFile;
		}

		ZipFileHandle::SharedPtr openEncryptedEntry(
			zip_int64_t entryIndex,
			const std::string& entryPwd
		)
		{
			zip_file_t* zipFilePtr = zip_fopen_index_encrypted(
				get(),
				entryIndex,
				0,
				entryPwd.c_str()
			);

			if (!zipFilePtr) {

				throw std::runtime_error(
					std::string("cannot open encrypted archive entry for reading -> ")
						+ zip_strerror(get())
				);

			}

			auto openFile = std::make_shared<
				ZipFileHandle
			>(zipFilePtr);

			_openFiles[zipFilePtr] = openFile;

			return openFile;
		}

		void closeEntry(ZipFileHandle::RawPtr zipFilePtr)
		{
			_openFiles.erase(zipFilePtr);
		}

	private:

		RawPtr _zipPtr;
		bool _hasBeenSaved;

		std::vector<
			SourceStream::SharedPtr
		> _attachedSourcesForSaving;

		std::map<
			ZipFileHandle::RawPtr,
			ZipFileHandle::SharedPtr
		> _openFiles;

	};

}