#ifndef UTILS_H
#define UTILS_H
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <string>

std::string sha256(std::string message);
std::string sha256String(std::string message);
std::string sha512(std::string message);
std::string sha512String(std::string message);
std::string sha2toString(std::string digest);
std::string keccak256(std::string message);
std::string keccak256String(std::string message);
#endif