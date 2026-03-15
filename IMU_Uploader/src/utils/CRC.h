#ifndef CRC_H
#define CRC_H

#include <QByteArray>
#include <cstdint>

class CRC
{
public:
    static uint8_t calculateCRC8(const QByteArray &data, int start = 0, int length = -1);
    static uint16_t calculateCRC16(const QByteArray &data, int start = 0, int length = -1);
    static uint32_t calculateCRC32(const QByteArray &data, int start = 0, int length = -1);
    
    static bool verifyCRC8(const QByteArray &data, uint8_t crc);
    static bool verifyCRC16(const QByteArray &data, uint16_t crc);
    static bool verifyCRC32(const QByteArray &data, uint32_t crc);
    
    static QByteArray addCRC8(QByteArray &data);
    static QByteArray addCRC16(QByteArray &data);
    static QByteArray addCRC32(QByteArray &data);

private:
    static const uint8_t CRC8_TABLE[256];
    static const uint16_t CRC16_TABLE[256];
    static const uint32_t CRC32_TABLE[256];
    
    static void initCRC8Table();
    static void initCRC16Table();
    static void initCRC32Table();
};

#endif
