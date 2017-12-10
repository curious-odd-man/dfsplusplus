#include "common.h"

#include "typedefs.h"
#include "defs.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "utils.h"
#include "StorageManager.h"

static void registerAtFileManager(const string& addr, unsigned short port,
		unsigned short ownport) {
	struct sockaddr_in serv_addr;
	struct hostent *server;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		throw "ERROR opening socket";

	server = gethostbyname(addr.c_str());
	if (server == NULL) {
		throw "ERROR, no such host\n";
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		throw "ERROR connecting";

	CmdPackage request;
	request.command = CMD::REGISTER;
	strcpy(request.fname, ("localhost:" + to_string(ownport)).c_str());

	cout << "Sending registration request:" << endl;
	cout << request.toString() << endl;
	long n = write(sockfd, &request, sizeof(CmdPackage));
	if (n < 0)
		throw "Failed to send request.";

	close(sockfd);
}

int main(int argc, char** argv) {

	cout << "Launched file storage service;" << endl;

	string rootDirectory = ".";
	string fileManager = "localhost";
	unsigned short fileManagerPort = FILE_MANAGER_PORT;
	unsigned short port = DATA_STORAGE_PORT;

	for (int i = 0; i < argc; ++i) {
		if (!strncmp(argv[i], "--port", 6)) {
			++i;
			port = (unsigned short) stoi(argv[i]);

		}

		if (!strncmp(argv[i], "--file-manager-port", 6)) {
			++i;
			fileManagerPort = (unsigned short) stoi(argv[i]);

		}

		if (!strncmp(argv[i], "--root", 6)) {
			++i;
			rootDirectory.assign(argv[i]);
		}

		if (!strncmp(argv[i], "--file-manager", 6)) {
			++i;
			fileManager.assign(argv[i]);
		}
	}

	cout << "Root directory: " << rootDirectory << endl;
	cout << "Port: " << port << endl;
	cout << "File manager: " << fileManager << endl;
	cout << "File manager port: " << fileManagerPort << endl;

	registerAtFileManager(fileManager, fileManagerPort, port);

	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	// create a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cout << "ERROR opening socket" << endl;
		return -1;
	}

	// clear address structure
	bzero((char *) &serv_addr, sizeof(serv_addr));

	/* setup the host_addr structure for use in bind call */
	// server byte order
	serv_addr.sin_family = AF_INET;

	// automatically be filled with current host's IP address
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// convert short integer value for port must be converted into network byte order
	serv_addr.sin_port = htons(port);

	// bind() passes file descriptor, the address structure,
	// and the length of the address structure
	// This bind() call will bind  the socket to the current IP address on port, portno
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		cout << "ERROR on binding" << endl;
		return -2;
	}

	// The listen() function places all incoming connection into a backlog queue
	// until accept() call accepts the connection.
	// Here, we set the maximum size for the backlog queue to 5.

	listen(sockfd, 5);

	StorageManager sm(rootDirectory);

	while (true) {

		// The accept() call actually accepts an incoming connection
		clilen = sizeof(cli_addr);

		// This accept() function will write the connecting client's address info
		// into the the address structure and the size of that structure is clilen.
		// The accept() returns a new socket file descriptor for the accepted connection.
		// So, the original socket file descriptor can continue to be used
		// for accepting new connections while the new socket file descriptor is used for
		// communicating with the connected client.
		cout << "Listening for incoming connection." << endl;
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		cout << "Accepted connection..." << endl;
		if (newsockfd < 0) {
			cout << "ERROR on accept" << endl;
			return -4;
		}

		printf("server: got connection from %s port %d\n",
				inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

		CmdPackage request;
		long n = read(newsockfd, &request, sizeof(CmdPackage));
		if (n < 0) {
			cout << "ERROR reading from socket" << endl;
			return -3;
		}

		cout << "Received: " << request.toString() << ". Reading data..."
				<< endl;

		switch (request.command) {
		case CMD::PUT: {
			char* data = new char[request.dataSize];
			n = read(newsockfd, data, request.dataSize);
			if (n < 0) {
				cout << "ERROR reading from socket" << endl;
				delete[] data;
				return -3;
			}
			sm.put(request, data);
			delete[] data;
		}
			break;

		case CMD::GET: {
			char * data = sm.get(request);
			n = write(newsockfd, data, request.dataSize);
			if (n < 0) {
				cout << "ERROR writing to socket" << endl;
				delete[] data;
				return -3;
			}
			delete[] data;
		}
			break;

		default:
			cout << "Unknown command" << endl;
			return -1;
		}

		close(newsockfd);

		cout << "Processing ended.." << endl;
	}
	close(sockfd);

	cout << "Terminating file storage service;" << endl;
	return 0;
}
