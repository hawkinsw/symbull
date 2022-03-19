#ifndef __SYMBOL_SYMBOL_HPP
#define __SYMBOL_SYMBOL_HPP

#include <string>
#include <symbol/virtualdata.hpp>

class Symbol {
public:
  Symbol(const std::string &elf_filename,
         const std::string &text_section_name = ".text", bool debug = false)
      : m_elf_filename(elf_filename), m_initialized(false), m_scanned(false),
        m_debug(debug), m_string_section_data(), m_sym_string_section_data(),
        m_text_section_name(text_section_name), m_text_section_data() {
  }
  bool initialize(std::string &err_message);
  bool scan(std::string &err_message);

private:
  std::string m_elf_filename;
  bool m_initialized, m_scanned, m_debug;

  std::string m_text_section_name;

  VirtualData m_string_section_data;
  VirtualData m_sym_string_section_data;
  VirtualData m_sym_section_data;
  VirtualData m_text_section_data;
};

#endif