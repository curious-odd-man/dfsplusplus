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

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		throw "ERROR opening socket";

	server = gethostbyname(address.c_str());
	if (server == NULL) {
		throw "ERROR, no such host\n";
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(FILE_MANAGER_PORT);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		throw "ERROR connecting";
}

void FileManagerClient::putFileCommand(const string& fileName) {

	ifstream infile(fileName, ios::binary);
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
	memset(package.fname, 0 , sizeof(package.fname));
	memcpy(package.fname, fileName.c_str(), fileName.length());

	cout << "Sending: " << package.toString() << endl;
	long n = write(sockfd, &package, sizeof(CmdPackage));

	if (n < 0)
		throw "ERROR writing to socket";

	CmdPackage response;
	n = read(sockfd, &response, sizeof(CmdPackage));
	if (n < 0)
		throw "ERROR reading from socket";

	cout << "Received: " << response.toString() << endl;

	if (package.index != response.index)
		throw "Unexpected response";// FIXME: just ignore and wait for correct one

	if (response.dataSize > 0) {
		char* arrayOfFileParts = new char[response.dataSize];
		n = read(sockfd, arrayOfFileParts, response.dataSize);
		if (n < 0) {
			delete[] arrayOfFileParts;
			throw "ERROR reading from socket 2";
		}

		dump(arrayOfFileParts, response.dataSize, "Received bytes:");

		char* inputFileData = new char[inputFileSize];
		infile.read(inputFileData, inputFileSize);

		// FIXME: parallelize distribution
		// Distribute data
		FilePart* parts = (FilePart*) arrayOfFileParts;
		for (size_t i = 0; i < response.dataSize / sizeof(FilePart); ++i) {
			sendToDataStorage(inputFileData, parts[i]);
		}

		delete[] arrayOfFileParts;
	} else {
		throw "Unexpected empty response";
	}
}

void FileManagerClient::sendToDataStorage(char* fileData, FilePart& part) {

	cout << part.toString() << endl;
	// FIXME: Not implemented
	return;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		throw "ERROR opening socket";

	server = gethostbyname(part.host);
	if (server == NULL) {
		throw "ERROR, no such host\n";
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(DATA_STORAGE_PORT);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		throw "ERROR connecting";
}

FileManagerClient::~FileManagerClient() {
	close(sockfd);
}

