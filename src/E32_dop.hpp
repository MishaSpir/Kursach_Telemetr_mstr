#include <cstdint>
#include <cmath> 

constexpr uint8_t SIZE{64};
class Circular_buffer {
public:
	void put(uint8_t);
	uint8_t get();
	uint8_t get_rd();
	uint8_t get_wr();
	uint8_t count;
	bool empty();
	bool full();
	uint8_t wr_idx;
	uint8_t rd_idx;
	Circular_buffer(); // Circular_buffer b1;
	Circular_buffer(uint8_t); // Circular_buffer b2(128);
	uint8_t buf[SIZE]; // Противоречит конструктору с пармаетрами
		bool full_;
private:
};


void uart2_write(uint8_t* data, const uint32_t length );
void uart1_write(uint8_t* data, const uint32_t length );

char* reverse(char* buffer, int i, int j);

// сама функция itoa
char* itoa(int value, char* buffer, int base);