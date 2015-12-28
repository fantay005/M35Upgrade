// CMD
// gcc packfile.cpp -o packfile -lstdc++ -static
// packfile.exe xxx.bin xxx.pack

#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

using namespace std;

#define BLOCK_SIZE 2048
#define HEADER_SIZE 32

class PackHeader {
public:
	PackHeader(const char *path) : m_path(path) {
	}

	~PackHeader() {
	}

	bool write(const char *outFile) {
		ifstream ifs(m_path.c_str(), ifstream::in | ifstream::binary);
		if (!ifs.is_open()) return false;
		ofstream ofs(outFile, ifstream::out | ifstream::binary);
		if (!ofs.is_open()) return false;

		ifs.seekg(0, ios_base::end);
		m_header.fileLength = ifs.tellg();
		ifs.seekg(0, ios_base::beg);

		int blockNum = (m_header.fileLength + BLOCK_SIZE - 1) / BLOCK_SIZE;

		m_header.headerLength = HEADER_SIZE + blockNum * 4;

		for (int i=0; i < sizeof(m_header.resolved)/sizeof(m_header.resolved[0]); i++) {
			m_header.resolved[i] = 0x00;
		}

		cout << __func__ << __LINE__ << endl;

		unsigned int int_buf[BLOCK_SIZE/4];
		char *buf = (char *)&int_buf[0];
		int readLen;
		int block = 0;
		while(1) {
			ifs.read((char *)buf, BLOCK_SIZE);
			readLen = ifs.gcount();
			if (readLen != BLOCK_SIZE && !ifs.eof())
				return false;

			if (block == 0) {
				m_header.startAddr = int_buf[8];
			}

			if (readLen != BLOCK_SIZE)
				memset(&buf[readLen], 0, BLOCK_SIZE - readLen);

			m_header.blockCrs[block] = calculateCrc32Stm32(int_buf, BLOCK_SIZE);

			if (ifs.eof()) break;
			block++;
		}

		ifs.close();
		ifs.open(m_path.c_str(), ifstream::in | ifstream::binary);
		if (!ifs.is_open()) return false;

		m_header.headerCrc = calculateCrc32Stm32(&m_header.headerLength, m_header.headerLength - 4);
		ofs.write((const char *)&m_header, m_header.headerLength);

		cout << hex;
		cout << "  HeaderCRC: " << m_header.headerCrc << endl;
		cout << "  HeaderLength: " << m_header.headerLength << endl;
		cout << "  FileLength: " << m_header.fileLength << endl;
		cout << "  startAddr: " << m_header.startAddr << endl;

		for (int i = 0; i < blockNum; i++) {
			cout << "  Block[" << i << "] CRC: " << m_header.blockCrs[i] << endl;
		}
		cout << dec;


		ifs.seekg(0, ios_base::beg);
		while(!ifs.eof()) {
			ifs.read((char *)buf, BLOCK_SIZE);
			readLen = ifs.gcount();
			ofs.write((char *)buf, readLen);
		}

		return true;
	}


private:
	struct Header {
		unsigned int headerCrc;
		unsigned int headerLength;
		unsigned int fileLength;
		unsigned int startAddr;
		unsigned int resolved[4];
		unsigned int blockCrs[512];
	} m_header;

	string m_path;


	unsigned int calculateCrc32Stm32(unsigned int *buf, int len)
	{
		unsigned int xbit, data;
		unsigned int polynomial = 0x04c11db7;
		unsigned int crc = 0xFFFFFFFF;    // init
		unsigned int *ptr = (unsigned int *)buf;
		len = len / 4;

	    while (len--) {
	        data = *ptr++;
	        for (xbit = 1 << 31; xbit != 0; xbit = xbit>>1) {
	            if (crc & 0x80000000) {
	            	crc <<= 1;
	            	crc ^= polynomial;
	            } else {
	                crc <<= 1;
	            }

	            if (data & xbit) {
	                crc ^= polynomial;
	            }
	        }
	    }
	    return crc;
	}
};

void usage(char *program)
{
	cout << "Usage: " << program << " in out" << endl;
	cout << "    in ........... file want to pack, output file is file.out" << endl;
	cout << "    out .......... output file" << endl;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		usage(argv[0]);
		exit(1);
	}
	PackHeader packHeader(argv[1]);

	if (packHeader.write(argv[2])) {
		cout << "OK, output file is " << argv[2] << endl;
		exit(0);
	}

	cout << "Pack file error." << endl;
	exit(2);
}
