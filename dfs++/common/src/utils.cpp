#include "common.h"

void continuos_copy(void* dst, long& offset, const void* src,
		const size_t size) {
	memcpy((char*) dst + offset, src, size);
	offset += size;
}

void dump(void* indata, size_t size, const string& header) {

	if (!header.empty()) {
		cout << header << endl;
	}

	const unsigned char * data = (unsigned char*) indata;

	const string TEMPLATE =
			"000000    00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 00    0123456789ABCDEF";
	const string FMT = "%06x    %-48.48s    %-16.16s\n";

	char bytes[255] = { 0 };
	char prtData[255] = { 0 };

	size_t currOffset = 0;
	for (; currOffset < size; ++currOffset) {
		if (currOffset != 0 && currOffset % 16 == 0) {
			printf(FMT.c_str(), currOffset - 16, bytes, prtData);
			memset(bytes, 0, sizeof(bytes));
			memset(prtData, 0, sizeof(prtData));
		}

		const size_t charInLine = currOffset % 16;
		const size_t prtOffset = charInLine * 3 + (charInLine > 7 ? 1 : 0);

		sprintf(bytes + prtOffset, "%02x  ", data[currOffset]);
		sprintf(prtData + charInLine, "%c", (isprint(data[currOffset]) ? data[currOffset] : '.'));
	}

	if (currOffset % 16) {
		printf(FMT.c_str(), currOffset - currOffset % 16, bytes, prtData);
	}
}
