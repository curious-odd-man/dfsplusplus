#pragma once

#include "typedefs.h"

class FileManagerClient {
public:
	FileManagerClient(const string& address);
	void putFileCommand(const string& fileName);
	virtual ~FileManagerClient();

private:
	int sockfd;

	void sendToDataStorage(char* fileData, FilePart& part);
};

