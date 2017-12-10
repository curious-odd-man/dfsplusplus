#include "common.h"

#include "typedefs.h"

CmdPackage::CmdPackage() {
	memset(this, 0, sizeof(CmdPackage));
}

string CmdPackage::toString() const {
	char buf[1024];
	snprintf(buf, sizeof(buf), "idx=%i;cmd=%i;fname=%s;size=%lu", index, command, fname, dataSize);
	return string(buf);
}

FilePart::FilePart() {
	memset(this, 0, sizeof(FilePart));
}

string FilePart::toString() const {
	char buf[1024];
	snprintf(buf, sizeof(buf), "idx=%ld;repl_id=%ld;offset=%lu;size=%lu;host=%s;port=%u", index, replica_id, offset, size, host, port);
	return string(buf);
}
