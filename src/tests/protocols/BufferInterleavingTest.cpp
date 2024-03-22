#include <gtest/gtest.h>

#include "BufferInterleaving.h"

using namespace lime;

TEST(BufferInterleaving, SISO_I12_to_I12)
{
    std::array<uint16_t, 4> inputSamples = { { 0x07FF, 0x0123, 0x0ABC, 0xF800 } };
    const int complexSamplesCount = inputSamples.size() / 2;
    std::array<uint8_t, 6> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I12;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I12;
    cfg.channelCount = 1;

    void* src = inputSamples.data();
    int bytesProduced = Interleave(output.data(), reinterpret_cast<void**>(&src), complexSamplesCount, cfg);

    const std::array<uint8_t, 6> expectedOutput = { { 0xFF, 0x37, 0x12, 0xBC, 0x0A, 0x80 } };
    EXPECT_EQ(bytesProduced, expectedOutput.size());
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferInterleaving, SISO_I16_to_I16)
{
    std::array<uint16_t, 4> inputSamples = { { 0x7FFF, 0x1234, 0xABCD, 0x8000 } };
    const int complexSamplesCount = inputSamples.size() / 2;
    std::array<uint8_t, 8> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.channelCount = 1;

    void* src = inputSamples.data();
    int bytesProduced = Interleave(output.data(), reinterpret_cast<void**>(&src), complexSamplesCount, cfg);

    const std::array<uint8_t, 8> expectedOutput = { { 0xFF, 0x7F, 0x34, 0x12, 0xCD, 0xAB, 0x00, 0x80 } };
    EXPECT_EQ(bytesProduced, expectedOutput.size());
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferInterleaving, SISO_I16_to_I12)
{
    std::array<uint16_t, 4> inputSamples = { { 0x7FFF, 0x1234, 0xABCD, 0x8000 } };
    const int complexSamplesCount = inputSamples.size() / 2;
    std::array<uint8_t, 6> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I12;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.channelCount = 1;

    void* src = inputSamples.data();
    int bytesProduced = Interleave(output.data(), reinterpret_cast<void**>(&src), complexSamplesCount, cfg);

    const std::array<uint8_t, 6> expectedOutput = { { 0xFF, 0x37, 0x12, 0xBC, 0x0A, 0x80 } };
    EXPECT_EQ(bytesProduced, expectedOutput.size());
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferInterleaving, MIMO_I16_to_I16)
{
    std::array<uint16_t, 4> inputSamplesA = { { 0x010A, 0x020A, 0x030A, 0x040A } };
    std::array<uint16_t, 4> inputSamplesB = { { 0x100B, 0x200B, 0x300B, 0x400B } };
    const int complexSamplesCount = inputSamplesA.size() / 2;
    std::array<uint8_t, 16> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.channelCount = 2;

    void* src[2] = { inputSamplesA.data(), inputSamplesB.data() };
    int bytesProduced = Interleave(output.data(), reinterpret_cast<void**>(&src), complexSamplesCount, cfg);

    const std::array<uint8_t, 16> expectedOutput = {
        { 0x0A, 0x01, 0x0A, 0x02, 0x0B, 0x10, 0x0B, 0x20, 0x0A, 0x03, 0x0A, 0x04, 0x0B, 0x30, 0x0B, 0x40 }
    };
    EXPECT_EQ(bytesProduced, expectedOutput.size());
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferInterleaving, MIMO_I16_to_I12)
{
    std::array<uint16_t, 4> inputSamplesA = { { 0x010A, 0x020A, 0x030A, 0x040A } };
    std::array<uint16_t, 4> inputSamplesB = { { 0x100B, 0x200B, 0x300B, 0x400B } };
    const int complexSamplesCount = inputSamplesA.size() / 2;
    std::array<uint8_t, 12> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I12;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.channelCount = 2;

    void* src[2] = { inputSamplesA.data(), inputSamplesB.data() };
    int bytesProduced = Interleave(output.data(), reinterpret_cast<void**>(&src), complexSamplesCount, cfg);

    const std::array<uint8_t, 12> expectedOutput = { { 0x10, 0x00, 0x02, 0x00, 0x01, 0x20, 0x30, 0x00, 0x04, 0x00, 0x03, 0x40 } };
    EXPECT_EQ(bytesProduced, expectedOutput.size());
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferInterleaving, SISO_F32_to_I16)
{
    std::array<float, 4> inputSamples = { { 1, -1, 0.5, -0.25 } };
    const int complexSamplesCount = inputSamples.size() / 2;
    std::array<uint8_t, 8> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::F32;
    cfg.channelCount = 1;

    void* src = inputSamples.data();
    int bytesProduced = Interleave(output.data(), reinterpret_cast<void**>(&src), complexSamplesCount, cfg);

    const std::array<uint8_t, 8> expectedOutput = { { 0xFF, 0x7F, 0x01, 0x80, 0xFF, 0x3F, 0x01, 0xE0 } };
    EXPECT_EQ(bytesProduced, expectedOutput.size());
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferDeinterleaving, SISO_I12_to_I12)
{
    const std::array<uint8_t, 6> src = { { 0xFF, 0x37, 0x12, 0xBC, 0x0A, 0x80 } };
    const std::array<uint16_t, 4> expectedOutput = { { 0x07FF, 0x0123, 0xFABC, 0xF800 } };
    std::array<uint16_t, 4> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I12;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I12;
    cfg.channelCount = 1;

    void* dest = output.data();
    int samplesProduced = Deinterleave(reinterpret_cast<void**>(&dest), src.data(), src.size(), cfg);

    EXPECT_EQ(samplesProduced, expectedOutput.size() / 2);
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferDeinterleaving, SISO_I16_to_I16)
{
    const std::array<uint8_t, 8> src = { { 0xFF, 0x7F, 0x34, 0x12, 0xCD, 0xAB, 0x00, 0x80 } };
    const std::array<uint16_t, 4> expectedOutput = { { 0x7FFF, 0x1234, 0xABCD, 0x8000 } };
    std::array<uint16_t, 4> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.channelCount = 1;

    void* dest = output.data();
    int samplesProduced = Deinterleave(reinterpret_cast<void**>(&dest), src.data(), src.size(), cfg);

    EXPECT_EQ(samplesProduced, expectedOutput.size() / 2);
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferDeinterleaving, SISO_I12_to_I16)
{
    const std::array<uint8_t, 6> src = { { 0xFF, 0x37, 0x12, 0xBC, 0x0A, 0x80 } };
    const std::array<uint16_t, 4> expectedOutput = { { 0x7FF0, 0x1230, 0xABC0, 0x8000 } };
    std::array<uint16_t, 4> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I12;
    cfg.channelCount = 1;

    void* dest = output.data();
    int samplesProduced = Deinterleave(reinterpret_cast<void**>(&dest), src.data(), src.size(), cfg);

    EXPECT_EQ(samplesProduced, expectedOutput.size() / 2);
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferDeinterleaving, SISO_I16_to_F32)
{
    const std::array<uint8_t, 8> src = { { 0xFF, 0x7F, 0x00, 0x80, 0xFF, 0x3F, 0x00, 0xE0 } };
    const std::array<float, 4> expectedOutput = { { 0.999969482, -1.0, 0.499969482, -0.25 } };
    std::array<float, 4> output;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::F32;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.channelCount = 1;

    void* dest = output.data();
    int samplesProduced = Deinterleave(reinterpret_cast<void**>(&dest), src.data(), src.size(), cfg);

    EXPECT_EQ(samplesProduced, expectedOutput.size() / 2);
    EXPECT_EQ(output, expectedOutput);
}

TEST(BufferDeinterleaving, MIMO_I16_to_I16)
{
    const std::array<uint8_t, 16> src = {
        { 0x0A, 0x01, 0x0A, 0x02, 0x0B, 0x10, 0x0B, 0x20, 0x0A, 0x03, 0x0A, 0x04, 0x0B, 0x30, 0x0B, 0x40 }
    };
    const std::array<uint16_t, 4> expectedOutputA = { { 0x010A, 0x020A, 0x030A, 0x040A } };
    const std::array<uint16_t, 4> expectedOutputB = { { 0x100B, 0x200B, 0x300B, 0x400B } };
    std::array<uint16_t, 4> outputA;
    std::array<uint16_t, 4> outputB;

    DataConversion cfg;
    cfg.destFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.srcFormat = SDRDevice::StreamConfig::DataFormat::I16;
    cfg.channelCount = 2;

    void* dest[2] = { outputA.data(), outputB.data() };
    int samplesProduced = Deinterleave(reinterpret_cast<void**>(&dest), src.data(), src.size(), cfg);

    EXPECT_EQ(samplesProduced, expectedOutputA.size() / 2);
    EXPECT_EQ(outputA, expectedOutputA);
    EXPECT_EQ(outputB, expectedOutputB);
}
