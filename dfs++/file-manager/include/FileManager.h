#pragma once

struct FileChunk {
	long aStorageId;
};

struct DFile {
	list<FileChunk> aChunks;
};

class FileManager {
public:
	void addStorage(const string& host, int port);
	char* put(const CmdPackage& request, CmdPackage& response);

private:

	vector<pair<string, short>> aStorageServices;
	map<string, DFile> aStoredFiles;

	long calculateStorageId(long index, long replicaId) const;
};

