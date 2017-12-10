#pragma once

struct CmdPackage;

class StorageManager {
public:

	StorageManager(const string& rootPath);

	void put(const CmdPackage& request, const char* data);
	char* get(const CmdPackage& request);

private:
	const string aRoot;
	map<string, string> aStoredFiles;

	string makeFileName(const string& originalName);
};

