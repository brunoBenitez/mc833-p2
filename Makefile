all: client.x server.x
.PHONY: all

client.x:
	$(MAKE) --directory client
	cp client/client.x ./client.x
.PHONY: client.x

server.x:
	$(MAKE) --directory server
	cp server/server.x ./server.x
.PHONY: server.x

clean:
	$(MAKE) clean --directory client
	$(MAKE) clean --directory server
	rm -f server.x client.x
.PHONY: clean