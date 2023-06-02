all: voxelizer main

make_dirs:
	mkdir -p bin

main: make_dirs
	make -C src

voxelizer: make_dirs
	make -C voxelizer

clean:
	make -C src clean
	make -C voxelizer clean
	rm ./bin -r
