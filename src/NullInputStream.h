#pragma once

// This class mocks empty input stream

namespace Zip {

	class NullInputStream {
	public:

		static const int beg = 0;
		static const int cur = 0;
		static const int end = 0;

		NullInputStream() :
			_failFlag(false),
			_eofFlag(false)
		{}

		long long gcount() const { return 0; }
		bool fail() const { return _failFlag; }
		bool eof() const { return _eofFlag; }

		void clear()
		{
			_failFlag = false;
			_eofFlag = false;
		}

		void seekg(long long pos, int)
		{
			if (pos != 0) {
				_failFlag = true;
			}
		}

		long long tellg()
		{
			return 0;
		}

		void read(char* buff, long long len)
		{
			if (len != 0) {
				_failFlag = true;
				_eofFlag = true;
			}
		}

	private:

		bool _failFlag;
		bool _eofFlag;

	};

}