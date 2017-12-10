
.PHONY:
all: 
	cd dfs++/client && make -j client
	cd dfs++/file-manager && make -j file-manager
	
.PHONY:
clean:
	cd dfs++/client && make clean
	cd dfs++/file-manager && make clean
	