#ifndef __SYMBOL_SYMBOL_HPP
#define __SYMBOL_SYMBOL_HPP

#include <iostream>
#include <string>
#include <symbol/virtualdata.hpp>
#include <vector>

class Symbols {
public:
  Symbols(const std::string &elf_filename,
          const std::string &text_section_name = ".text", bool debug = false)
      : m_elf_filename(elf_filename), m_initialized(false), m_scanned(false),
        m_debug(debug), m_string_section_data(), m_sym_string_section_data(),
        m_text_section_name(text_section_name), m_text_section_data(),
        m_symbols() {
  }
  bool initialize(std::string &err_message);
  bool scan(std::string &err_message);
  bool output(std::ostream &output_stream, size_t first_x_bytes, std::string &err_message);

  friend std::ostream &operator<<(std::ostream &os, const Symbols &symbols);

private:
  struct Symbol {
    Symbol(const std::string &symbol_name, uint64_t symbol_address)
        : m_symbol_name(symbol_name), m_symbol_address(symbol_address) {
    }

    friend std::ostream &operator<<(std::ostream &os, const Symbol &symbol) {
      os << std::hex << symbol.m_symbol_address << ": " << symbol.m_symbol_name;
      return os;
    }

    std::string m_symbol_name;
    uint64_t m_symbol_address;
  };

  std::string m_elf_filename;
  bool m_initialized, m_scanned, m_debug;

  std::string m_text_section_name;

  VirtualData m_string_section_data;
  VirtualData m_sym_string_section_data;
  VirtualData m_sym_section_data;
  VirtualData m_text_section_data;

  std::vector<Symbol> m_symbols;
};

#endif