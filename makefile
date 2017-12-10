
.PHONY:
all: 
	cd dfs++/client && make -j client
	cd dfs++/file-manager && make -j file-manager
	cd dfs++/data-storage && make -j storage


.PHONY:
clean:
	cd dfs++/client && make clean
	cd dfs++/file-manager && make clean
	cd dfs++/data-storage && make clean
	