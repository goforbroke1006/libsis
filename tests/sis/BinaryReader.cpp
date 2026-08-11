//
// Created by goforbroke on 8/9/26.
//

#include <gtest/gtest.h>

#include <sis/BinaryReader.h>

TEST(Parser, sis__BinaryReader__seek) {
    BinaryReader target({1, 2, 3, 4});
    EXPECT_EQ(0, target.position());
    target.seek(3);
    EXPECT_EQ(3, target.position());
}

TEST(Parser, sis__BinaryReader__read_u8) {
    BinaryReader target({
        0x40, 0xE2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    });
    EXPECT_EQ(64, target.read_u8());
}

TEST(Parser, sis__BinaryReader__read_u16_le) {
    BinaryReader target({
        0x40, 0xE2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    });
    EXPECT_EQ(57920, target.read_u16_le());
}

TEST(Parser, sis__BinaryReader__read_u32_le) {
    BinaryReader target({
        0x40, 0xE2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    });
    EXPECT_EQ(123456, target.read_u32_le());
}

TEST(Parser, sis__BinaryReader__read_u64_le) {
    BinaryReader target({
        0x40, 0xE2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    });
    EXPECT_EQ(123456, target.read_u64_le());
}

TEST(Parser, sis__BinaryReader__read_bytes) {
    BinaryReader target({
        0x40, 0xE2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    });
    const auto bytes = target.read_bytes(4);
    EXPECT_EQ(4, bytes.size());
}
