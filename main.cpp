#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <symbol/symbol.hpp>

#include <args.hxx>

const int PROLOGUE_LENGTH_DEFAULT{30};

int main(int argc, char *argv[]) {
  args::ArgumentParser parser("Perform symbol scan on an ELF binary.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag debug(parser, "debug", "Enable debugging.", {"d", "debug"});
  args::ValueFlag<std::string> outfile_param(
      parser, "outfile", "Store output to a file", {"o", "outfile"});
  args::ValueFlag<int> prologue_length(
      parser, "prologue-length", "Number of bytes of the prologue to capture for analysis", {"l", "prologue-length"});
  args::Positional<std::string> binary(
      parser, "binary", "The name of the binary file to disassemble.",
      args::Options::Required);
  try {
    parser.ParseCLI(argc, argv);
  } catch (args::Help) {
    std::cout << parser;
    return 0;
  } catch (args::ParseError e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  } catch (args::ValidationError e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  std::ofstream outfile_stream{};
  if (outfile_param) {
    outfile_stream.open(outfile_param.Get(), std::ios::trunc);
    if (!outfile_stream.is_open()) {
      std::cerr << "Error: Could not open output file ... output will be sent "
                   "to stdout!\n";
    }
  }
  std::ostream &output_stream =
      outfile_stream.is_open() ? outfile_stream : std::cout;

  std::ifstream binary_stream{binary.Get()};
  if (!binary_stream.is_open()) {
    std::cerr << "Error: Could not open the binary for symbolic analysis!\n";
    if (outfile_stream.is_open()) {
      outfile_stream.close();
    }
    exit(EXIT_FAILURE);
  }

  if (!prologue_length) {
    *prologue_length = PROLOGUE_LENGTH_DEFAULT;
  }

  Symbols *symbols = nullptr;
  symbols = new Symbols(binary.Get(), ".text", debug);

  std::string initialization_error_msg{""};
  if (!symbols->initialize(initialization_error_msg)) {
    std::cerr << "Initialization failed: " << initialization_error_msg << "\n";
    if (outfile_stream.is_open()) {
      outfile_stream.close();
    }
    exit(EXIT_FAILURE);
  }

  std::string scan_error_msg{""};
  if (!symbols->scan(scan_error_msg)) {
    std::cerr << "scan failed: " << scan_error_msg << "\n";
    if (outfile_stream.is_open()) {
      outfile_stream.close();
    }
    exit(EXIT_FAILURE);
  }

  if (debug) {
    std::cout << *symbols << "\n";
  }

  std::string output_error_msg{""};
  if (!symbols->output(output_stream, *prologue_length, output_error_msg)) {
    std::cerr << "output failed: " << output_error_msg << "\n";
    if (outfile_stream.is_open()) {
      outfile_stream.close();
    }
    exit(EXIT_FAILURE);
  }

  delete symbols;

  if (outfile_stream.is_open()) {
    outfile_stream.close();
  }

  return EXIT_SUCCESS;
}