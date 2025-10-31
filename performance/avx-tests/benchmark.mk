CC = gcc
CFLAGS = -O3 -Wall -Wextra -fPIC -fvisibility=hidden -march=native
LDFLAGS = -lrt

TARGET = benchmark

.PHONY: all clean

all: $(TARGET)

$(TARGET): benchmark.o hexsimd_benchmark.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

benchmark.o: benchmark.c
	$(CC) $(CFLAGS) -c $< -o $@

hexsimd_benchmark.o: hexsimd_benchmark.c
	$(CC) $(CFLAGS) -c $< -o $@

hexsimd_benchmark.c: hexsimd.c
	cp $< $@
	sed -i 's/static ptrdiff_t hex_to_bytes_scalar_impl/ptrdiff_t hex_to_bytes_scalar_impl/' $@
	sed -i 's/static ptrdiff_t hex_to_bytes_sse2_impl/ptrdiff_t hex_to_bytes_sse2_impl/' $@
	sed -i 's/static ptrdiff_t hex_to_bytes_avx2_impl/ptrdiff_t hex_to_bytes_avx2_impl/' $@
	sed -i 's/static ptrdiff_t hex_to_bytes_avx512_impl/ptrdiff_t hex_to_bytes_avx512_impl/' $@

clean:
	rm -f $(TARGET) *.o hexsimd_benchmark.c
