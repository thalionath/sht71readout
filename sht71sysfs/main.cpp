
#include <cstdio>
#include <cstdio>
#include <cerrno>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <unistd.h>

namespace sysfs
{
    class File
    {
        FILE* fp_;

    public:

        File(std::string const& filename, std::string const& mode)
        {
            auto const fp = fopen(
                filename.c_str(),
                mode.c_str()
            );

            if( 0 == fp )
            {
                std::string const error(
                    strerror(errno)
                );

                throw std::runtime_error("failed to open " + filename + ": " + error);
            }

            fp_ = fp;
        }

        File& write(uint8_t const* data, size_t size)
        {
            if( 0 == fwrite(data, size, 1, fp_) )
            {
                std::string const error(
                    strerror(errno)
                );

                throw std::runtime_error("write incomplete: " + error);
            }

            return *this;
        }

        File& write(char const* string)
        {
            return write(
                reinterpret_cast<uint8_t const*>(string),
                std::strlen(string)
            );
        }

        File& write(std::string const& string)
        {
            return write(
                reinterpret_cast<uint8_t const*>(
                    string.data()
                ),
                string.length()
            );
        }

        File& write(char character)
        {
            return write(
                reinterpret_cast<uint8_t const*>(&character),
                sizeof(char)
            );
        }

        std::string read(size_t size = 1)
        {
            std::vector<char> buffer(size);

            fread(buffer.data(), buffer.size(), 1, fp_);

            return {
                buffer.data(),
                buffer.size()
            };
        }

        ~File()
        {
            fclose(fp_);
        }
    };

    enum Direction
    {
        Output,
        Input
    };

    enum Level
    {
        Low = '0',
        High = '1'
    };

    class Gpio
    {
        unsigned id_;

        File file(std::string const& name, std::string const& mode) const
        {
            return File(
                std::string("/sys/class/gpio/gpio") + std::to_string(id_) + "/" + name,
                mode
            );
        }

    public:

        Gpio(unsigned id, Direction dir = Input)
        : id_(id)
        {
            // export the port
            File("/sys/class/gpio/export", "wb").write(
                std::to_string(id)
            );

            direction(dir);
        }

        ~Gpio()
        {
            File("/sys/class/gpio/unexport", "wb").write(
                std::to_string(id_)
            );
        }

        Gpio& direction(Direction dir)
        {
            file("direction", "w").write(
                dir == Output ? "out" : "in"
            );

            return *this;
        }

        Gpio& operator=(Direction dir)
        {
            return direction(dir);
        }

        Gpio& output()
        {
            return direction(Output);
        }

        Gpio& input()
        {
            return direction(Input);
        }

        Gpio& setValue(Level level)
        {
            file("value", "w").write(level);

            return *this;
        }

        Gpio& operator=(Level level)
        {
            return setValue(level);
        }

        Gpio& high()
        {
            return setValue(High);
        }

        Gpio& low()
        {
            return setValue(Low);
        }

        Level value() const
        {
            return "0" == file("value", "r").read() ? Low : High;
        }

        bool isLow() const
        {
            return Level::Low == value();
        }

        bool isHigh() const
        {
            return Level::High == value();
        }
    };
}

using namespace sysfs;

class Crc
{
    static uint8_t const kTable[];

    uint8_t crc_;

public:

    Crc(uint8_t crc = 0x00) : crc_(crc) {}

    Crc& add(uint8_t byte)
    {
        crc_ = kTable[crc_ ^ byte];

        return *this;
    }

    uint8_t value() const
    {
        return crc_;
    }

    operator uint8_t() const
    {
        return crc_;
    }

    Crc reversed() const
    {
        return reversed(crc_);
    }

    static uint8_t reversed(uint8_t b)
    {
        b = (b & 0xF0u) >> 4u | (b & 0x0Fu) << 4u;
        b = (b & 0xCCu) >> 2u | (b & 0x33u) << 2u;
        b = (b & 0xAAu) >> 1u | (b & 0x55u) << 1u;

        return b;
    }
};

uint8_t const Crc::kTable[] = {
      0,  49,  98,  83, 196, 245, 166, 151,
    185, 136, 219, 234, 125,  76,  31,  46,
     67, 114,  33,  16, 135, 182, 229, 212,
    250, 203, 152, 169,  62,  15,  92, 109,
    134, 183, 228, 213,  66, 115,  32,  17,
     63,  14,  93, 108, 251, 202, 153, 168,
    197, 244, 167, 150,   1,  48,  99,  82,
    124,  77,  30,  47, 184, 137, 218, 235,
     61,  12,  95, 110, 249, 200, 155, 170,
    132, 181, 230, 215,  64, 113,  34,  19,
    126,  79,  28,  45, 186, 139, 216, 233,
    199, 246, 165, 148,   3,  50,  97,  80,
    187, 138, 217, 232, 127,  78,  29,  44,
      2,  51,  96,  81, 198, 247, 164, 149,
    248, 201, 154, 171,  60,  13,  94, 111,
     65, 112,  35,  18, 133, 180, 231, 214,
    122,  75,  24,  41, 190, 143, 220, 237,
    195, 242, 161, 144,   7,  54, 101,  84,
     57,  8,   91, 106, 253, 204, 159, 174,
    128, 177, 226, 211,  68, 117,  38,  23,
    252, 205, 158, 175,  56,   9,  90, 107,
     69, 116,  39,  22, 129, 176, 227, 210,
    191, 142, 221, 236, 123,  74,  25,  40,
      6,  55, 100,  85, 194, 243, 160, 145,
     71, 118,  37,  20, 131, 178, 225, 208,
    254, 207, 156, 173,  58,  11,  88, 105,
      4,  53, 102,  87, 192, 241, 162, 147,
    189, 140, 223, 238, 121,  72,  27,  42,
    193, 240, 163, 146,   5,  52, 103,  86,
    120,  73,  26,  43, 188, 141, 222, 239,
    130, 179, 224, 209,  70, 119,  36,  21,
     59,  10,  89, 104, 255, 206, 157, 172
};

struct Measurement
{
    uint8_t status;
    uint16_t so_t;
    uint16_t so_rh;
    double temperature;
    double rh_linear;
    double rh_true;
};

class Sht71
{
    using Level = sysfs::Level;

    sysfs::Gpio data_;
    sysfs::Gpio clk_;

public:

    // coefficients to convert 12-bit humidity measurements.
    static auto constexpr c1 = -2.0468;
    static auto constexpr c2 =  0.0367;
    static auto constexpr c3 = -1.5955e-6;

    // coefficients to convert 14-bit temp. measurement with Vcc = 3V.
    static auto constexpr d1 = -39.60;
    static auto constexpr d2 =   0.01;

    enum Command
    {
        MeasureTemperature      = 0b00011,
        MeasureRelativeHumidity = 0b00101,
        ReadStatusRegister      = 0b00111,
        SoftReset               = 0b11110
    };

    enum Result
    {
        Success,
        CrcError
    };

    Sht71()
    : data_(1)
    , clk_(0)
    {
    }

    void clockCycle()
    {
        clk_ = High;
        clk_ = Low;
    }

    void writeDataBit(uint8_t bit)
    {
        data_ = bit > 0 ? High : Low;
        clockCycle();
    }

    uint8_t readByte()
    {
        uint8_t data = 0;

        for( auto i = 0u; i < 8u; ++i )
        {
            clk_ = High;

            auto const input = data_.isHigh();

            data <<= 1u;
            data  |= input ? 1u : 0u;

            clk_ = Low;
        }

        data_ = Output;
        data_ = Low;

        clockCycle();

        data_ = Input;

        return data;
    }

    template<typename T>
    Result startTransmission(uint8_t command, T& data)
    {
        clk_  = Output;
        data_ = Output;

        clk_  = Low;
        data_ = High;

        // Transmission Start

        clk_  = High;
        data_ = Low;
        clk_  = Low;
        clk_  = High;
        data_ = High;
        clk_  = Low;
        data_ = Low;

        for( uint8_t mask = 0x80u; mask; mask >>= 1u )
        {
            writeDataBit(command & mask);
        }

        data_ = Input;

        clockCycle();

        usleep(50000);

        auto counter = 0u;

        while( data_.isHigh() ) { counter++; }

        Crc crc;

        crc.add(command);

        for( auto i = 0u; i < sizeof(T); ++i )
        {
            auto const byte = readByte();

            data <<= 8u;
            data  |= byte;

            crc.add(byte);
        }

        auto expected_crc = readByte();

        data_ = Output;
        data_ = High;

        return crc.reversed().value() == expected_crc ? Result::Success : Result::CrcError;
    }

    Result readRelativeHumidity(double& rh_linear)
    {
        uint16_t so_rh;

        auto const result = startTransmission(MeasureRelativeHumidity, so_rh);

        if( Success == result )
        {
            rh_linear = c1 + c2 * so_rh + c3 * so_rh * so_rh;
        }

        return result;
    }

    Result readTemperature(double& temperature)
    {
        uint16_t so_t;

        auto const result = startTransmission(MeasureTemperature, so_t);

        if( Success == result )
        {
            temperature = d1 + d2 * so_t;
        }

        return result;
    }

    Result read(Measurement& m)
    {
        uint8_t status;
        uint16_t so_t;
        uint16_t so_rh;

        auto result = startTransmission(ReadStatusRegister, status);

        if( result != Success )
        {
            return result;
        }

        result = startTransmission(MeasureTemperature, so_t);

        if( result != Success )
        {
            return result;
        }

        result = startTransmission(MeasureRelativeHumidity, so_rh);

        if( Success != result )
        {
            return result;
        }

        // coefficients to convert 12-bit measurement
        static auto constexpr t1 = 0.01;
        static auto constexpr t2 = 0.00008;

        // see data sheet
        m.status      = status;
        m.so_t        = so_t;
        m.so_rh       = so_rh;
        m.temperature = d1 + d2 * so_t;
        m.rh_linear   = c1 + c2 * so_rh + c3 * so_rh * so_rh;
        m.rh_true     = (m.temperature - 25)*(t1 + t2 * so_rh) + m.rh_linear;

        return Success;
    }
};

int main()
{
    /*
    Crc c;

    c.add(0b101).add(0b1001).add(0b110001);

    printf("%02x\r\n", c.value());
    */

    auto constexpr sensor_id = 0u;

    Sht71 sht71;

    Measurement m;

    if( Sht71::Success == sht71.read(m) )
    {
        printf(
            "sht71,sensor=%u status=%u,t=%f,rh_linear=%f,rh_true=%f",
            sensor_id,
            m.status,
            m.temperature,
            m.rh_linear,
            m.rh_true
        );
    }

    return 0;
}