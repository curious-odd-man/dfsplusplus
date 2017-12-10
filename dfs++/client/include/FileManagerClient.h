#pragma once

#include "typedefs.h"

class FileManagerClient {
public:
	FileManagerClient(const string& address);
	void putFileCommand(const string& localFileName, const string& dfsFileName);
	void getFileCommand(const string& localFileName, const string& dfsFileName);
	virtual ~FileManagerClient();

private:
	int aFileManagerSocket;

	void sendToDataStorage(const char* fileData, FilePart& part, const string& fname);
	void getDataFromStorage(char* fileData, FilePart& part, const string& fname);
};

