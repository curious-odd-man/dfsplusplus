#include "common.h"

#include "typedefs.h"
#include "const.h"

#include "FileManager.h"

void FileManager::addStorage(const string& host, int port) {
	cout << "New storage registered: " << host << port << endl;
	aStorageServices.push_back(make_pair(host, port));
}

char* FileManager::put(const CmdPackage& request, CmdPackage& response) {
	size_t uniquePartsCount = (request.dataSize / MAX_CHUNK_SIZE
			+ min((size_t) 1, (request.dataSize % MAX_CHUNK_SIZE)));

	response.dataSize = DUPLICATION_FACTOR * uniquePartsCount
			* sizeof(FilePart);

	char * data = new char[response.dataSize];
	FilePart* parts = (FilePart*) data;
	for (size_t i = 0; i < uniquePartsCount; ++i) {
		for (size_t j = 0; j < DUPLICATION_FACTOR; ++j) {
			FilePart& part = parts[i * DUPLICATION_FACTOR + j];
			part.index = i;
			part.replica_id = j;
			part.offset = i * MAX_CHUNK_SIZE;
			part.size = min(MAX_CHUNK_SIZE, request.dataSize - part.offset);
			long storageId = calculateStorageId(i, j);
			part.port = aStorageServices[storageId].second;
			strcpy(part.host, aStorageServices[storageId].first.c_str());

			cout << part.toString() << endl;
		}
	}

	return data;
}

long FileManager::calculateStorageId(long index, long replicaId) const {
	return (index * DUPLICATION_FACTOR + replicaId) % aStorageServices.size();
}
