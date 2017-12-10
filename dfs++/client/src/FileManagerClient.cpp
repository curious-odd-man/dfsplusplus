#include "common.h"
#include "defs.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "FileManagerClient.h"
#include "utils.h"

FileManagerClient::FileManagerClient(const string& address) {

	struct sockaddr_in serv_addr;
	struct hostent *server;

	aFileManagerSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (aFileManagerSocket < 0)
		throw "ERROR opening socket";

	server = gethostbyname(address.c_str());
	if (server == NULL) {
		throw "ERROR, no such host\n";
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(FILE_MANAGER_PORT);
	if (connect(aFileManagerSocket, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
		throw "ERROR connecting";
}

void FileManagerClient::putFileCommand(const string& localFileName,
		const string& dfsFileName) {

	ifstream infile(localFileName, ios::binary);
	if (!infile.is_open())
		throw "Failed to open a file";

	// Determine file size
	const size_t begin = infile.tellg();
	infile.seekg(0, ios::end);
	const size_t end = infile.tellg();
	infile.seekg(ios::beg);
	const size_t inputFileSize = end - begin;

	CmdPackage package;
	package.index = 0;					// FIXME;
	package.command = CMD::PUT;
	package.dataSize = inputFileSize;
	strcpy(package.fname, dfsFileName.c_str());

	cout << "Sending: " << package.toString() << endl;
	long n = write(aFileManagerSocket, &package, sizeof(CmdPackage));

	if (n < 0)
		throw "ERROR writing to socket";

	CmdPackage response;
	n = read(aFileManagerSocket, &response, sizeof(CmdPackage));
	if (n < 0)
		throw "ERROR reading from socket";

	cout << "Received: " << response.toString() << endl;

	if (package.index != response.index)
		throw "Unexpected response index";// FIXME: just ignore and wait for correct one

	if (response.dataSize > 0) {
		char* arrayOfFileParts = new char[response.dataSize];
		n = read(aFileManagerSocket, arrayOfFileParts, response.dataSize);
		if (n < 0) {
			delete[] arrayOfFileParts;
			throw "ERROR reading from socket 2";
		}

		// dump(arrayOfFileParts, response.dataSize, "Received bytes:");

		char* inputFileData = new char[inputFileSize];
		infile.read(inputFileData, inputFileSize);

		// FIXME: parallelize distribution
		// Distribute data
		FilePart* parts = (FilePart*) arrayOfFileParts;
		for (size_t i = 0; i < response.dataSize / sizeof(FilePart); ++i) {
			sendToDataStorage(inputFileData, parts[i], dfsFileName);
		}

		delete[] arrayOfFileParts;
	} else {
		throw "Unexpected empty response";
	}
}

void FileManagerClient::getFileCommand(const string& localFileName,
		const string& dfsFileName) {

	CmdPackage request;
	request.index = 0;					// FIXME;
	request.command = CMD::GET;
	strcpy(request.fname, dfsFileName.c_str());

	cout << "Sending: " << request.toString() << endl;
	long n = write(aFileManagerSocket, &request, sizeof(CmdPackage));

	if (n < 0)
		throw "ERROR writing to socket";

	CmdPackage response;
	n = read(aFileManagerSocket, &response, sizeof(CmdPackage));
	if (n < 0)
		throw "ERROR reading from socket";

	cout << "Received: " << response.toString() << endl;

	if (request.index != response.index)
		throw "Unexpected response index";// FIXME: just ignore and wait for correct one

	if (response.dataSize > 0) {
		char* arrayOfFileParts = new char[response.dataSize];
		n = read(aFileManagerSocket, arrayOfFileParts, response.dataSize);
		if (n < 0) {
			delete[] arrayOfFileParts;
			throw "ERROR reading from socket 2";
		}

		//dump(arrayOfFileParts, response.dataSize, "Received bytes:");

		// FIXME: parallelize acquisition
		// Collect data
		FilePart* parts = (FilePart*) arrayOfFileParts;
		size_t numOfParts = response.dataSize / sizeof(FilePart);

		char* usedPart = new char[numOfParts];
		memset(usedPart, 0, numOfParts);

		size_t fileSize = 0;
		for (size_t i = 0; i < numOfParts; ++i) {
			FilePart& part = parts[i];
			if (usedPart[part.index])
				continue;

			fileSize += part.size;
			usedPart[part.index] = 1;
		}

		memset(usedPart, 0, numOfParts);
		char* fileData = new char[fileSize];

		for (size_t i = 0; i < numOfParts; ++i) {
			FilePart& part = parts[i];
			if (usedPart[part.index])
				continue;

			try {
				getDataFromStorage(fileData, part, dfsFileName);
				usedPart[part.index] = 1;
			} catch (const string& e) {
				// TODO:
			}
		}

		delete[] usedPart;

		ofstream outFile(localFileName, ios::binary);
		//dump(fileData, fileSize, "Writing file to output");
		outFile.write(fileData, fileSize);
		outFile.close();

		delete[] fileData;
		delete[] arrayOfFileParts;
	} else {
		throw "Unexpected empty response";
	}
}

void FileManagerClient::sendToDataStorage(const char* fileData, FilePart& part,
		const string& fname) {

	cout << part.toString() << endl;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	int dataStoreSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (dataStoreSocket < 0)
		throw "ERROR opening socket";

	server = gethostbyname(part.host);
	if (server == NULL) {
		throw "ERROR, no such host\n";
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(part.port);
	if (connect(dataStoreSocket, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
		throw "ERROR connecting";

	CmdPackage request;
	request.command = CMD::PUT;
	request.dataSize = part.size;
	strcpy(request.fname, fname.c_str());

	write(dataStoreSocket, &request, sizeof(CmdPackage));
	write(dataStoreSocket, fileData + part.offset, part.size);

	close(dataStoreSocket);
}

void FileManagerClient::getDataFromStorage(char* fileData, FilePart& part,
		const string& fname) {
	cout << part.toString() << endl;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	int dataStoreSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (dataStoreSocket < 0)
		throw "ERROR opening socket";

	server = gethostbyname(part.host);
	if (server == NULL) {
		throw "ERROR, no such host\n";
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(part.port);
	if (connect(dataStoreSocket, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0)
		throw "ERROR connecting";

	CmdPackage request;
	request.index = 666;		// FIXME
	request.command = CMD::GET;
	request.dataSize = part.size;		// FIXME: Withot it data-storage will send zero bytes back
	strcpy(request.fname, fname.c_str());

	long n = write(dataStoreSocket, &request, sizeof(CmdPackage));// TODO: check return value
	cout << "Sent: " << request.toString() << endl;
	if (n < 0) {
		throw "Failed to write request";
	}
#if 0		// Do we need this response before data???
	CmdPackage response;
	n = read(dataStoreSocket, &response, sizeof(CmdPackage));// TODO: check return value
	if (n < 0) {
		throw "Failed to read response";
	}
	cout << "Received: " << response.toString() << endl;
	if (request.index != response.index)
	throw "Inconsistent request/response indexes";

	if (response.dataSize != part.size)
	throw "Inconsistent expected and actual data sizes";

	if (response.dataSize > 0) {
#else
	{
#endif
		n = read(dataStoreSocket, fileData + part.offset, part.size);
		if (n < 0) {
			throw "ERROR reading from socket 2";
		}
	}

	close(dataStoreSocket);
}

FileManagerClient::~FileManagerClient() {
	close(aFileManagerSocket);
}

