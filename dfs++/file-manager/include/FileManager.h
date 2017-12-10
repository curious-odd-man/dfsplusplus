#pragma once

struct FileChunk {
	long aStorageId;

	long aIndex;
	long aReplicaId;
	size_t offset;
	size_t size;
};

struct DFile {
	list<FileChunk> aChunks;
};

class FileManager {
public:
	void addStorage(const string& host, int port);
	char* put(const CmdPackage& request, CmdPackage& response);
	char* get(const CmdPackage& request, CmdPackage& response);

private:

	vector<pair<string, short>> aStorageServices;
	map<string, DFile> aStoredFiles;		// This information shall persist between app run cycles

	long calculateStorageId(long index, long replicaId) const;
};

