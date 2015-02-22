/*
 *  zc - zip crack library
 *  Copyright (C) 2013  Marc Ferland
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TEST_PLAINTEXT_H_
#define _TEST_PLAINTEXT_H_

#define TEST_PLAINTEXT_SIZE 500

static const uint8_t test_plaintext[TEST_PLAINTEXT_SIZE] = {
    0x9d, 0x57, 0xdb, 0x72, 0x13, 0x47, 0x10, 0x7d, 0x9f, 0xaf,
    0x98, 0xbc, 0xc8, 0x76, 0x90, 0x84, 0x6d, 0x6c, 0xc7, 0xa1,
    0x2a, 0xa9, 0x28, 0xb6, 0x21, 0x04, 0x13, 0x0c, 0x36, 0x90,
    0x14, 0xeb, 0x87, 0xd1, 0xec, 0x68, 0x35, 0x78, 0x76, 0x66,
    0x3d, 0x17, 0x63, 0xa3, 0xd2, 0x07, 0xf0, 0x92, 0x6f, 0xc8,
    0x5b, 0x50, 0x9e, 0xf3, 0x07, 0xfb, 0x63, 0xe9, 0x9e, 0x5d,
    0x49, 0x2b, 0xf9, 0x02, 0xa4, 0x8a, 0x32, 0xda, 0xb9, 0x9e,
    0x3e, 0xdd, 0x7d, 0xba, 0x27, 0x49, 0x0d, 0x0f, 0xb9, 0xd0,
    0x9e, 0x2b, 0xe6, 0xdc, 0xdb, 0x8d, 0xf5, 0xc2, 0x9f, 0x8e,
    0x98, 0xf5, 0x92, 0x2b, 0x31, 0x26, 0x49, 0x70, 0xa2, 0x60,
    0xfc, 0x8c, 0x65, 0xe2, 0x6d, 0xf0, 0x83, 0xdd, 0xd3, 0x91,
    0xd4, 0x45, 0xf0, 0x42, 0xf3, 0xc5, 0xb9, 0x9c, 0xd9, 0x4c,
    0xea, 0x1f, 0x1e, 0xf0, 0xfc, 0x74, 0x94, 0x09, 0x93, 0x0b,
    0x6f, 0xaf, 0x16, 0x56, 0xc0, 0x91, 0x96, 0x5d, 0xb5, 0xe9,
    0x25, 0x37, 0xca, 0xd8, 0xc5, 0x29, 0x25, 0x0b, 0x17, 0x72,
    0x18, 0x4b, 0xc5, 0x40, 0x6a, 0x11, 0x57, 0xc0, 0x60, 0x36,
    0xf4, 0x19, 0xec, 0x19, 0x8f, 0xaa, 0xbf, 0xeb, 0xdd, 0x5d,
    0x58, 0xa2, 0xc5, 0x7b, 0x98, 0x0f, 0xb9, 0xf6, 0x57, 0x85,
    0x18, 0x1d, 0x8e, 0x47, 0x3f, 0x8e, 0x12, 0xcb, 0xb2, 0x4c,
    0xa4, 0x4a, 0x0c, 0xfc, 0xb8, 0x80, 0x65, 0x1b, 0x5b, 0x89,
    0x17, 0x97, 0xfe, 0xbd, 0x4c, 0xfd, 0x70, 0x7c, 0x6d, 0xcb,
    0xcb, 0xf1, 0x08, 0x17, 0xed, 0xde, 0xb0, 0x26, 0xcf, 0x99,
    0x4e, 0x93, 0xd7, 0x2f, 0x83, 0x12, 0xa3, 0x64, 0x19, 0x45,
    0x72, 0x61, 0x61, 0x9c, 0xc6, 0x1d, 0x74, 0xbd, 0xbb, 0x5d,
    0xf8, 0x31, 0x21, 0x89, 0x97, 0x1e, 0x17, 0xf7, 0x07, 0x4e,
    0x58, 0x29, 0x5c, 0xf2, 0x4b, 0xc8, 0x04, 0x7d, 0xc6, 0x2c,
    0xa7, 0x8f, 0x84, 0x55, 0x70, 0x1c, 0x9c, 0xcd, 0x82, 0x1f,
    0xc2, 0x51, 0xbb, 0x3b, 0x1b, 0xd4, 0x06, 0x41, 0x4f, 0x6c,
    0x39, 0xc9, 0xcd, 0x55, 0x92, 0x90, 0x17, 0xa1, 0x9c, 0xf4,
    0x05, 0xa7, 0xab, 0xf5, 0x8f, 0xb5, 0x36, 0x7d, 0xbc, 0xf1,
    0x3b, 0xdd, 0xea, 0x6d, 0xc2, 0x24, 0x30, 0xca, 0xbb, 0x83,
    0xea, 0x90, 0x9f, 0xb2, 0x9c, 0x49, 0xd5, 0x05, 0x84, 0x30,
    0x71, 0x52, 0x4e, 0x54, 0xf7, 0x21, 0xdd, 0xda, 0xd8, 0xed,
    0xec, 0x6c, 0x7f, 0xdf, 0xd9, 0x7e, 0xb0, 0xbd, 0x49, 0x90,
    0x3b, 0xe6, 0xc5, 0x08, 0xfe, 0xef, 0x0b, 0x70, 0xc4, 0x68,
    0xea, 0x54, 0x18, 0xc8, 0xd9, 0x99, 0x88, 0x30, 0x49, 0xe2,
    0x04, 0xf7, 0xd2, 0xe8, 0x6f, 0x47, 0xe5, 0xc7, 0x34, 0x70,
    0x86, 0xbf, 0x67, 0x3b, 0x3c, 0xeb, 0x07, 0xc5, 0xec, 0x78,
    0x74, 0xf8, 0xcd, 0xa8, 0xe2, 0x60, 0xfc, 0x72, 0x4c, 0x36,
    0xd7, 0xd7, 0xb7, 0x5b, 0x3f, 0x33, 0xce, 0x99, 0x62, 0x01,
    0x80, 0x33, 0x4f, 0x85, 0xa6, 0x4f, 0xf4, 0xc0, 0xd8, 0x1c,
    0xf6, 0x9f, 0x07, 0xd1, 0xa6, 0xaf, 0xb4, 0xbc, 0x10, 0xd6,
    0x49, 0x5f, 0x4e, 0x68, 0x2a, 0xe8, 0xf1, 0x50, 0xd8, 0xbe,
    0x35, 0xe6, 0x4c, 0x24, 0xc9, 0x5b, 0xa0, 0xe9, 0x14, 0x0f,
    0xd9, 0x6c, 0x9d, 0x08, 0x3e, 0xd4, 0xb8, 0xe1, 0xfa, 0x01,
    0x7b, 0xe5, 0x24, 0x13, 0x05, 0xee, 0x55, 0x8c, 0x3e, 0x66,
    0xae, 0x28, 0x27, 0x4e, 0xc2, 0x3a, 0x0f, 0x43, 0x8e, 0x96,
    0x7f, 0x2a, 0x20, 0x36, 0x21, 0x89, 0xd0, 0xe9, 0x0c, 0x25,
    0x69, 0x18, 0x73, 0xc8, 0x74, 0x16, 0x84, 0xfb, 0x8c, 0x25,
    0x8f, 0x2c, 0xd3, 0xe5, 0xdf, 0x4c, 0xba, 0x56, 0xb5, 0x9e,
};

static const uint8_t test_ciphertext[TEST_PLAINTEXT_SIZE] = {
    0xbb, 0x1c, 0xe2, 0xec, 0x37, 0x00, 0x50, 0x2e, 0x39, 0x43,
    0xf1, 0xcf, 0x46, 0x2e, 0xba, 0x7b, 0x6f, 0x4d, 0xf1, 0xc9,
    0x54, 0x15, 0x3a, 0x58, 0x7c, 0x33, 0x3c, 0xe8, 0xf5, 0xf2,
    0x43, 0x33, 0x90, 0xd9, 0x02, 0xd1, 0xb5, 0xc4, 0x9b, 0x26,
    0xfd, 0xee, 0xde, 0x56, 0x77, 0x3d, 0x41, 0x4f, 0x13, 0xfa,
    0xc8, 0x4c, 0x7c, 0x20, 0x8c, 0x58, 0x31, 0x85, 0xf3, 0x32,
    0x95, 0x38, 0x2e, 0x1b, 0x08, 0x71, 0x93, 0x8f, 0x0c, 0xd2,
    0xbb, 0xdd, 0x2e, 0x72, 0xa9, 0x62, 0x7e, 0xc2, 0x8d, 0x99,
    0xeb, 0x97, 0xbc, 0x5f, 0xa1, 0xe8, 0x2a, 0x3e, 0x9c, 0x4b,
    0xe8, 0x50, 0x45, 0x54, 0x68, 0x45, 0x91, 0x0b, 0xef, 0x61,
    0x29, 0x7f, 0x30, 0x94, 0x8c, 0xd7, 0x59, 0x06, 0xb5, 0xab,
    0x61, 0xd4, 0xe2, 0xca, 0x75, 0xda, 0x23, 0xa2, 0x09, 0x98,
    0xf3, 0x4d, 0xc3, 0x06, 0xe9, 0xb7, 0x96, 0xc0, 0x72, 0x74,
    0xec, 0xb3, 0xd0, 0x30, 0xe0, 0x83, 0xe6, 0x13, 0xf1, 0x32,
    0xe6, 0x63, 0xfa, 0x26, 0xb7, 0xd4, 0xb9, 0x11, 0x45, 0x60,
    0x20, 0xb5, 0x11, 0x1a, 0xc8, 0x54, 0x1f, 0xab, 0x4f, 0x15,
    0xec, 0x04, 0x0d, 0xfd, 0xbc, 0x85, 0x75, 0x81, 0xd5, 0x8b,
    0x17, 0x84, 0x72, 0xdd, 0xf1, 0x50, 0x11, 0x90, 0x31, 0xa5,
    0x26, 0x11, 0x15, 0xbd, 0x13, 0x53, 0x69, 0xa0, 0x66, 0xd3,
    0x0c, 0x95, 0x28, 0xa2, 0xfa, 0xfb, 0x25, 0x07, 0xb7, 0xac,
    0xa6, 0x2c, 0x05, 0xa0, 0xb9, 0xeb, 0xd4, 0x6e, 0x98, 0x1d,
    0xeb, 0x23, 0x5d, 0xbc, 0x99, 0x3b, 0x31, 0x7d, 0x0b, 0x26,
    0xf3, 0x26, 0x45, 0x64, 0xd7, 0x1e, 0x96, 0x48, 0xc5, 0x91,
    0xe7, 0x49, 0xd6, 0x74, 0x2c, 0xfd, 0x3a, 0xd7, 0x04, 0xa8,
    0x9a, 0x54, 0x69, 0xbb, 0x09, 0x2f, 0x77, 0x77, 0xba, 0x32,
    0x9a, 0x01, 0x91, 0x98, 0x31, 0x8d, 0xa4, 0x41, 0xc4, 0x65,
    0x58, 0x39, 0x7a, 0x24, 0x5b, 0x82, 0x4b, 0x31, 0xa3, 0xef,
    0x6c, 0x87, 0xf1, 0xda, 0xb5, 0x26, 0xf2, 0xfd, 0xd2, 0x97,
    0xe6, 0x46, 0xad, 0x49, 0x69, 0x18, 0x36, 0xdf, 0x7d, 0xba,
    0x46, 0x1d, 0x70, 0xbc, 0x62, 0x3f, 0x5e, 0x7c, 0xae, 0x17,
    0xd9, 0x11, 0x9d, 0x1d, 0xe2, 0x29, 0x56, 0x97, 0x5f, 0xfc,
    0x02, 0xbf, 0x30, 0x3a, 0x1c, 0x56, 0x08, 0xbd, 0xae, 0x9d,
    0xa7, 0x41, 0xca, 0x41, 0xed, 0x34, 0xd1, 0x1e, 0x1b, 0xca,
    0x5e, 0x06, 0x41, 0x6b, 0x29, 0x5f, 0xc9, 0x4d, 0x78, 0xd4,
    0x0d, 0xc6, 0x9c, 0xbc, 0xc0, 0xf0, 0x3f, 0x87, 0x7d, 0x27,
    0xfb, 0x10, 0xb9, 0xbf, 0x55, 0x65, 0x5f, 0x73, 0xcc, 0x82,
    0x6d, 0x65, 0xad, 0xfb, 0x12, 0xcd, 0x66, 0x7e, 0xfb, 0x54,
    0x2e, 0x94, 0x53, 0xd7, 0xa9, 0x43, 0x2d, 0x78, 0xf8, 0xc9,
    0x92, 0x11, 0x6b, 0x09, 0x5a, 0x5c, 0x78, 0x85, 0xc0, 0x20,
    0x6a, 0x9e, 0xda, 0x05, 0xcb, 0xbb, 0x27, 0xc8, 0xdd, 0xbf,
    0x76, 0x70, 0x67, 0xcf, 0x19, 0x44, 0x75, 0xdd, 0x57, 0x60,
    0xb3, 0xc9, 0xdb, 0xcf, 0x8e, 0x2f, 0x14, 0x32, 0xdd, 0x03,
    0x90, 0xb5, 0xe4, 0x35, 0x97, 0x38, 0xbe, 0x68, 0x88, 0xf5,
    0x8e, 0xfe, 0x22, 0x58, 0x4e, 0x19, 0xcf, 0xcd, 0xd2, 0xab,
    0x2e, 0x89, 0x63, 0xb5, 0x7e, 0x5d, 0x14, 0x0b, 0x58, 0x24,
    0x85, 0xe1, 0x6f, 0x50, 0x1b, 0x61, 0x73, 0xf0, 0x3d, 0xc0,
    0x1b, 0xe2, 0xfe, 0xaf, 0x1a, 0x72, 0x12, 0x8c, 0x64, 0xda,
    0x1a, 0xbd, 0x1f, 0x4c, 0xab, 0xa0, 0xa5, 0x64, 0xbd, 0x83,
    0xc1, 0xee, 0x4c, 0x73, 0x3b, 0xf0, 0x53, 0xa5, 0xc4, 0x4f,
    0xbd, 0x1a, 0x12, 0x2d, 0x67, 0xd4, 0x34, 0x5a, 0x4c, 0x3e,
};

static const uint8_t test_encrypted_header[12] = {
    0x44, 0x87, 0x7d, 0x06, 0x52, 0xbd, 0x2d, 0x09, 0xbc, 0x4f,
    0x1e, 0xe2,
};

#endif
