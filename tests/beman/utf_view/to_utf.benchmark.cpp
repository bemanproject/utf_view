// SPDX-License-Identifier: BSL-1.0

// Transcoding throughput benchmarks over the unicode_lipsum dataset
// (https://github.com/lemire/unicode_lipsum). For each of Arabic, Chinese,
// Japanese, and Korean we transcode the corpus both UTF-8 -> UTF-16 and
// UTF-16 -> UTF-8, to provide a point of comparison for the scalar
// transcoding views (and, eventually, a SIMD-backed implementation).

#include <benchmark/benchmark.h>

#include <beman/utf_view/config.hpp>
#include <beman/utf_view/to_utf_view.hpp>

#include <test_iterators.hpp>
#if BEMAN_UTF_VIEW_USE_MODULES()
import std;
#else
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#endif

#ifndef UNICODE_LIPSUM_DIR
#error "UNICODE_LIPSUM_DIR must be defined to the path of the lipsum data dir"
#endif

namespace {

constexpr std::string_view languages[]{"Arabic", "Chinese", "Japanese", "Korean"};

std::vector<char> read_file_bytes(std::string const& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("could not open " + path);
  }
  return std::vector<char>(std::istreambuf_iterator<char>(in),
                           std::istreambuf_iterator<char>{});
}

std::string lipsum_path(std::string_view lang, std::string_view encoding) {
  return std::string(UNICODE_LIPSUM_DIR) + "/" + std::string(lang) + "-Lipsum." +
         std::string(encoding) + ".txt";
}

std::vector<char8_t> load_utf8(std::string_view lang) {
  std::vector<char> bytes{read_file_bytes(lipsum_path(lang, "utf8"))};
  return std::vector<char8_t>(bytes.begin(), bytes.end());
}

// The unicode_lipsum .utf16.txt files are UTF-16LE and begin with a byte order
// mark (0xFF 0xFE), which we skip. This host is little-endian (x86-64), so the
// in-memory char16_t units match the on-disk code units after a raw copy.
std::vector<char16_t> load_utf16le(std::string_view lang) {
  std::vector<char> bytes{read_file_bytes(lipsum_path(lang, "utf16"))};
  std::size_t offset{0};
  if (bytes.size() >= 2 &&
      static_cast<unsigned char>(bytes[0]) == 0xFF &&
      static_cast<unsigned char>(bytes[1]) == 0xFE) {
    offset = 2;
  }
  std::size_t const count{(bytes.size() - offset) / 2};
  std::vector<char16_t> units(count);
  std::memcpy(units.data(), bytes.data() + offset, count * sizeof(char16_t));
  return units;
}

// Lazily transcode the whole input through `adaptor` once per iteration,
// folding the produced code units into a checksum so the work can't be
// optimized away. Reports throughput in terms of input bytes consumed.
template <typename Input, typename Adaptor>
auto make_transcode_benchmark(Input input, Adaptor adaptor, std::size_t input_bytes) {
  return [input = std::move(input), adaptor, input_bytes](benchmark::State& state) {
    for (auto _ : state) {
      std::size_t checksum{0};
      for (auto code_unit : input | adaptor) {
        checksum += static_cast<std::size_t>(code_unit);
      }
      benchmark::DoNotOptimize(checksum);
    }
    state.SetBytesProcessed(static_cast<std::int64_t>(state.iterations()) *
                            static_cast<std::int64_t>(input_bytes));
  };
}

} // namespace

int main(int argc, char** argv) {
  using beman::utf_view::to_utf16;
  using beman::utf_view::to_utf8;

  for (std::string_view lang : languages) {
    std::vector<char8_t> utf8{load_utf8(lang)};
    std::vector<char16_t> utf16{load_utf16le(lang)};

    std::size_t const utf8_bytes{utf8.size()};
    std::size_t const utf16_bytes{utf16.size() * sizeof(char16_t)};

    benchmark::RegisterBenchmark(
        std::string(lang) + "/utf8_to_utf16",
        make_transcode_benchmark(std::move(utf8), to_utf16, utf8_bytes));
    benchmark::RegisterBenchmark(
        std::string(lang) + "/utf16_to_utf8",
        make_transcode_benchmark(std::move(utf16), to_utf8, utf16_bytes));
  }

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
