main: src/main.cpp
	mkdir -p bin

	g++ -o bin/main src/main.cpp -lhikvision_api -L/usr/local/lib -I/usr/local/include -Wl,-rpath=$(MVCAM_COMMON_RUNENV)/64 -L$(MVCAM_COMMON_RUNENV)/64 -lMvCameraControl -lpthread

clean:
	rm -rf bin