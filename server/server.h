#pragma once

// handles a single request for the socket connection.
// if the client wants to send any more requests, it should open a new connection.
void handle_request(int sock_fd);