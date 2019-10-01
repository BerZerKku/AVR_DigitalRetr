/**
 *
 */
class TCrc16 {
public:
	TCrc16() {
		reset();
	}

	void reset() {
		crc = 0xFFFF;
	}

	void add(uint8_t val) {
		crc ^= val;

		for (int i = 8; i != 0; i--) {
			if ((crc & 0x0001) != 0) {
				crc >>= 1;
				crc ^= 0xA001;
			} else {
				crc >>= 1;
			}
		}
	}

	uint16_t get() const {
		return crc;
	}

	uint8_t getLow() const {
		return crc;
	}

	uint8_t getHigh() const {
		return crc >> 8;
	}

private:
	uint16_t crc;
};
