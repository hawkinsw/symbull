#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <fcntl.h>
#include <memory.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <elf.h>
#include <libelf.h>

#include <symbol/symbol.hpp>
std::ostream &operator<<(std::ostream &os, const Symbols &symbols) {
  std::cout << "Function symbols in " << symbols.m_elf_filename << ":\n";
  std::cout << "-----------------------------------------------------\n";
  for (auto symbol : symbols.m_symbols) {
    std::cout << symbol << "\n";
  }
  std::cout << "-----------------------------------------------------\n";
  return os;
}

bool Symbols::initialize(std::string &err_message) {
  int elf_fd;
  Elf *elf_handle;
  Elf_Scn *scn_iterator{nullptr};
  Elf64_Ehdr *ehdr{nullptr};

  Elf_Scn *string_scn{nullptr};
  Elf_Data *string_data{nullptr};

  Elf64_Shdr *text_shdr{nullptr};
  Elf_Scn *text_scn{nullptr};
  Elf_Data *text_scn_data{nullptr};

  Elf64_Shdr *sym_shdr{nullptr};
  Elf_Scn *sym_scn{nullptr};
  Elf_Data *sym_scn_data{nullptr};

  Elf64_Shdr *sym_string_shdr{nullptr};
  Elf_Scn *sym_string_scn{nullptr};
  Elf_Data *sym_string_scn_data{nullptr};

  if ((elf_fd = open(m_elf_filename.c_str(), O_RDONLY)) < 0) {
    err_message = "Could not open the requested ELF file.";
    return false;
  }

  elf_version(EV_CURRENT);
  elf_handle = elf_begin(elf_fd, ELF_C_READ, nullptr);

  if (!elf_handle) {
    close(elf_fd);
    err_message = "Could not create an ELF handle for the ELF file.";
    return false;
  }

  /* Obtain the .shstrtab data buffer */
  if (((ehdr = elf64_getehdr(elf_handle)) == nullptr) ||
      ((string_scn = elf_getscn(elf_handle, ehdr->e_shstrndx)) == nullptr) ||
      ((string_data = elf_getdata(string_scn, nullptr)) == nullptr)) {
    elf_end(elf_handle);
    close(elf_fd);
    err_message = "Could not find the string header.";
    return false;
  }

  auto string_section_header = elf64_getshdr(string_scn);
  // This should be more robust against the size of individual elements
  // in the ELF section being different (larger) than a single byte.
  auto string_section_mallocd =
      (char *)malloc(sizeof(char) * string_section_header->sh_size);
  memcpy(string_section_mallocd, (char *)string_data->d_buf,
         sizeof(char) * string_section_header->sh_size);
  m_string_section_data = std::move(VirtualData{
      (char *)string_section_mallocd, string_section_header->sh_addr,
      string_section_header->sh_size});

  bool text_section_found = false;
  bool sym_section_found = false;
  bool sym_string_section_found = false;
  /* Traverse input filename, printing each section */
  while ((!text_section_found || !sym_section_found ||
          !sym_string_section_found) &&
         (scn_iterator = elf_nextscn(elf_handle, scn_iterator))) {
    Elf64_Shdr *shdr{nullptr};
    auto shdr_iterator{elf64_getshdr(scn_iterator)};
    if (shdr_iterator == nullptr) {
      elf_end(elf_handle);
      close(elf_fd);
      err_message = "Failed to iterate through the section headers.";
      return false;
    }

    char *current_section_name = m_string_section_data.get(
        m_string_section_data.start() + shdr_iterator->sh_name);
    if (m_debug) {
      std::cout << "Investigating section " << current_section_name << "\n";
    }

    if (m_text_section_name == current_section_name) {
      if (m_debug) {
        std::cout << "Found the program section!\n";
      }
      text_section_found = true;
      text_scn = scn_iterator;
      text_shdr = shdr_iterator;
      text_scn_data = elf_getdata(text_scn, nullptr);
      continue;
    }

    if (std::string{".symtab"} == current_section_name) {
      if (m_debug) {
        std::cout << "Found the symbol section!\n";
      }
      sym_section_found = true;
      sym_scn = scn_iterator;
      sym_shdr = shdr_iterator;
      sym_scn_data = elf_getdata(sym_scn, nullptr);
      continue;
    }
    if (std::string{".strtab"} == current_section_name) {
      if (m_debug) {
        std::cout << "Found the string table for the symbol section!\n";
      }
      sym_string_section_found = true;
      sym_string_scn = scn_iterator;
      sym_string_shdr = shdr_iterator;
      sym_string_scn_data = elf_getdata(sym_string_scn, nullptr);
      continue;
    }
  }

  if (!text_section_found || !sym_section_found || !sym_string_section_found) {
    elf_end(elf_handle);
    close(elf_fd);
    err_message = "Could not find the text section, the symbol section or the "
                  "symbol string section!";
    return false;
  }

  if (m_debug) {
    std::cout << "Initialization of Symbol complete.\n";
  }

  // This should be more robust against the size of individual elements
  // in the ELF section being different (larger) than a single byte.
  char *text_section_mallocd =
      (char *)malloc(sizeof(char) * text_scn_data->d_size);
  memcpy(text_section_mallocd, text_scn_data->d_buf, text_shdr->sh_size);
  m_text_section_data = std::move(VirtualData(
      text_section_mallocd, text_shdr->sh_addr, text_shdr->sh_size));

  // This should be more robust against the size of individual elements
  // in the ELF section being different (larger) than a single byte.
  char *sym_section_mallocd =
      (char *)malloc(sizeof(char) * sym_scn_data->d_size);
  memcpy(sym_section_mallocd, sym_scn_data->d_buf, sym_shdr->sh_size);
  m_sym_section_data = std::move(
      VirtualData(sym_section_mallocd, sym_shdr->sh_addr, sym_shdr->sh_size));

  // This should be more robust against the size of individual elements
  // in the ELF section being different (larger) than a single byte.
  char *sym_string_section_mallocd =
      (char *)malloc(sizeof(char) * sym_string_scn_data->d_size);
  memcpy(sym_string_section_mallocd, sym_string_scn_data->d_buf,
         sym_string_shdr->sh_size);
  m_sym_string_section_data = std::move(VirtualData(sym_string_section_mallocd,
                                                    sym_string_shdr->sh_addr,
                                                    sym_string_shdr->sh_size));

  elf_end(elf_handle);
  close(elf_fd);

  m_initialized = true;
  return true;
}

bool Symbols::scan(std::string &err_message) {
  if (!m_initialized) {
    err_message = "Cannot scan before initialization.";
    return false;
  }

  size_t index = 0;
  for (size_t index = 0; index < m_sym_section_data.size();
       index += sizeof(Elf64_Sym)) {
    Elf64_Sym *current_sym =
        (Elf64_Sym *)m_sym_section_data.get(m_sym_section_data.start() + index);

    auto current_name_sym_string_offset = current_sym->st_name;
    char *current_sym_name = (char *)m_sym_string_section_data.get(
        m_sym_string_section_data.start() + current_name_sym_string_offset);

    if (ELF64_ST_TYPE(current_sym->st_info) != STT_FUNC) {
      if (m_debug) {
        std::cout << "Skipping " << current_sym_name
                  << " because it is not a function.\n";
      }
      continue;
    }

    auto symbol = Symbol(std::string{current_sym_name}, current_sym->st_value);
    m_symbols.push_back(symbol);

    if (m_debug && current_sym_name != nullptr) {
      std::cout << "Discovered " << symbol << "\n";
    }
  }
  m_scanned = true;
  return true;
}

bool Symbols::output(std::ostream &output_stream, size_t first_x_bytes,
                     std::string &err_message) {
  for (auto symbol : m_symbols) {
    char *prologue_bytes = nullptr;
    if ((prologue_bytes = m_text_section_data.get(symbol.m_symbol_address))) {
      if (!(output_stream << symbol.m_symbol_name.size()
                        << symbol.m_symbol_name)) {
        err_message = "Could not output symbol name and header for ." +
                      symbol.m_symbol_name;
        return false;
      }
      for (size_t i = 0; i < first_x_bytes; i++) {
        if (!(output_stream << prologue_bytes[i])) {
          err_message = "Could not output byte " + std::to_string(i) +
                        " of the prologue of " + symbol.m_symbol_name;
          return false;
        }
      }
    }
  }
  return true;
}