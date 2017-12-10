#include "common.h"

#include "typedefs.h"

#include "StorageManager.h"

StorageManager::StorageManager(const string& rootPath) :
		aRoot(rootPath) {

}

void StorageManager::put(const CmdPackage& request, const char* data) {
	string fileName(aRoot + "/" + makeFileName(request.fname));
	aStoredFiles[request.fname] = fileName;

	ofstream outfile(fileName, ios::binary);
	if (!outfile.is_open())
		throw "Failed to open file";

	outfile.write(data, request.dataSize);
	outfile.close();
}

char* StorageManager::get(const CmdPackage& request) {
	string fileName(aStoredFiles.at(request.fname));

	cout << "Data stored in file: " << fileName;

	ifstream infile(fileName, ios::binary);
	if (!infile.is_open())
		throw "Failed to open file";

	const size_t begin = infile.tellg();
	infile.seekg(0, ios::end);
	const size_t end = infile.tellg();
	infile.seekg(ios::beg);
	const size_t inputFileSize = end - begin;

	cout << "Size is: " << inputFileSize;

	char* data = new char[inputFileSize];
	infile.read(data, inputFileSize);
	infile.close();
	return data;
}

string StorageManager::makeFileName(const string& originalName) {
	static int i = 0;
	return string("file" + to_string(i++));
}
