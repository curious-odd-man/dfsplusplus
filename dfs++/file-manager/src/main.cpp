#include "common.h"
#include "defs.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "typedefs.h"
#include "const.h"
#include "utils.h"

#include "FileManager.h"

int main(int argc, char** argv) {

	cout << "DFS++ file manager launching..." << endl;

	int sockfd, newsockfd;
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
	serv_addr.sin_port = htons(FILE_MANAGER_PORT);

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

	FileManager fm;

	while (true) {
		cout << "Listening for incoming connection." << endl;

		// The accept() call actually accepts an incoming connection
		socklen_t clilen = sizeof(cli_addr);

		// This accept() function will write the connecting client's address info
		// into the the address structure and the size of that structure is clilen.
		// The accept() returns a new socket file descriptor for the accepted connection.
		// So, the original socket file descriptor can continue to be used
		// for accepting new connections while the new socket file descriptor is used for
		// communicating with the connected client.
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		cout << "Accepted connection. Processing..." << endl;
		if (newsockfd < 0) {
			cout << "ERROR on accept" << endl;
			return -4;
		}

		printf("server: got connection from %s port %d\n",
				inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

		CmdPackage request;
		cout << "Ready to read request." << endl;
		long n = read(newsockfd, &request, sizeof(CmdPackage));
		if (n < 0) {
			cout << "ERROR reading from socket" << endl;
			return -3;	// FIXME:
		}

		cout << "Received: " << request.toString() << endl;

		if (request.command == CMD::REGISTER) {
			string storage(request.fname);
			size_t colPos = storage.find(':');
			if (colPos == string::npos) {
				cout << "No colon in data storage registration address: "
						<< storage << endl;
				return -1;		// FIXME:
			}

			fm.addStorage(storage.substr(0, colPos),
					stoi(storage.substr(colPos + 1)));

		} else if (request.command != CMD::UNKNOWN) {

			CmdPackage response;
			response.index = request.index;
			response.command = request.command;
			char* data = nullptr;

			switch (request.command) {
			case CMD::PUT:
				data = fm.put(request, response);
				break;

			case CMD::GET:
				data = fm.get(request, response);
				break;

			default:
				cout << "Unknown command" << endl;
				return -1;		// FIXME:
			}

			cout << "Sending: " << response.toString() << endl;

			n = write(newsockfd, &response, sizeof(CmdPackage));
			if (data) {
				// dump(data, response.dataSize, "Sending data:");
				n = write(newsockfd, data, response.dataSize);
				delete[] data;
			}
		} else {
			cout << "Don't know what to do with command: " << request.command << endl;
		}
		close(newsockfd);
	}
	close(sockfd);

	cout << "DFS++ file manager TERMINATED" << endl;

	return 0;
}
