// SPDX-License-Identifier: BSL-1.0
//
// Standalone UTF-16 -> UTF-8 chunk-kernel benchmark. Mirrors utf_view's refill
// model: the corpus is transcoded in fixed-size windows into a reused output
// buffer (like buf_), advancing by input_consumed each call, with a checksum
// fold so nothing is elided. We sweep the window size to find the "knee" -- the
// smallest chunk at which a kernel reaches its throughput ceiling -- which is
// the property the std::simd experiment is really chasing (see the prompt's
// section 3). Three chunk kernels are compared at each size:
//
//   scalar   : transcode_utf16_to_utf8_scalar      (the per-code-point baseline)
//   stdsimd  : transcode_utf16_to_utf8_simd         (the std::simd kernel)
//   simdutf  : simdutf::convert_utf16le_to_utf8     (the hand-tuned reference)
//
// plus a single bulk simdutf call over the whole corpus (the kernel ceiling,
// no per-chunk overhead). No Google Benchmark dependency -- std::chrono only.

#include <beman/utf_view/detail/simd_transcode.hpp>
#include <simdutf.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace sk = beman::utf_view::detail::simd_kernel;
using clk = std::chrono::steady_clock;

namespace {

#ifndef UNICODE_LIPSUM_DIR
#  error "UNICODE_LIPSUM_DIR must be defined"
#endif

std::vector<char16_t> load_utf16le(std::string const& path) {
  std::ifstream f(path, std::ios::binary);
  std::vector<char> bytes(std::istreambuf_iterator<char>(f),
                          std::istreambuf_iterator<char>{});
  std::size_t off = (bytes.size() >= 2 &&
                     static_cast<unsigned char>(bytes[0]) == 0xFF &&
                     static_cast<unsigned char>(bytes[1]) == 0xFE)
                        ? 2
                        : 0;
  std::size_t n = (bytes.size() - off) / 2;
  std::vector<char16_t> u(n);
  std::memcpy(u.data(), bytes.data() + off, n * sizeof(char16_t));
  return u;
}

// Trim a window so it ends on a complete code point: a trailing high surrogate
// needs its low half from the next chunk. (utf_view's refill_chunk_simd does the
// same before handing simdutf a window; the std::simd kernel does it internally.)
std::size_t trim_high_surrogate(char16_t const* in, std::size_t window) {
  if (window > 0) {
    char16_t last = in[window - 1];
    if (last >= 0xD800 && last <= 0xDBFF)
      --window;
  }
  return window;
}

enum class kernel { scalar, stdsimd, simdutf };

// Drive `in` through fixed `chunk`-sized windows into a reused output buffer,
// returning a checksum (folded so the work cannot be optimized away).
template <kernel K>
std::uint64_t run_chunked(char16_t const* in, std::size_t n, std::size_t chunk,
                          char8_t* out, std::size_t out_cap) {
  std::uint64_t checksum = 0;
  std::size_t pos = 0;
  while (pos < n) {
    std::size_t window = std::min(chunk, n - pos);
    std::size_t produced = 0;
    if constexpr (K == kernel::scalar) {
      auto r = sk::transcode_utf16_to_utf8_scalar(in + pos, window, out, out_cap);
      produced = r.output_produced;
      pos += r.input_consumed ? r.input_consumed : window; // never stall
    } else if constexpr (K == kernel::stdsimd) {
      auto r = sk::transcode_utf16_to_utf8_simd(in + pos, window, out, out_cap);
      produced = r.output_produced;
      pos += r.input_consumed ? r.input_consumed : window;
    } else {
      std::size_t w = trim_high_surrogate(in + pos, window);
      if (w == 0) w = window; // degenerate; keep moving
      produced = simdutf::convert_utf16le_to_utf8(
          in + pos, w, reinterpret_cast<char*>(out));
      pos += w;
    }
    for (std::size_t k = 0; k < produced; ++k)
      checksum += static_cast<std::uint8_t>(out[k]);
  }
  return checksum;
}

struct timing {
  double gibps;
  std::uint64_t checksum;
};

template <typename F>
timing measure(F&& f, std::size_t input_bytes, double min_seconds = 0.30) {
  // warm up + auto-scale repetitions to ~min_seconds, report best.
  std::uint64_t cs = f();
  double best = 1e30;
  auto t_end = clk::now() + std::chrono::duration_cast<clk::duration>(
                                std::chrono::duration<double>(min_seconds));
  int reps = 0;
  while (clk::now() < t_end || reps < 3) {
    auto t0 = clk::now();
    cs ^= f();
    auto t1 = clk::now();
    double s = std::chrono::duration<double>(t1 - t0).count();
    best = std::min(best, s);
    ++reps;
  }
  double gibps = (double)input_bytes / best / (1024.0 * 1024.0 * 1024.0);
  return {gibps, cs};
}

} // namespace

int main(int argc, char** argv) {
  std::string_view langs_default[] = {"Latin",    "Arabic", "Chinese",
                                      "Japanese", "Korean", "Emoji"};
  std::vector<std::string_view> langs(std::begin(langs_default),
                                      std::end(langs_default));
  if (argc > 1) { langs.assign(argv + 1, argv + argc); }

  std::size_t const chunks[] = {64, 128, 256, 512, 1024, 2048, 4096, 8192};

  std::printf("%-9s %8s | %10s %10s %10s | %12s\n", "lang", "chunk", "scalar",
              "stdsimd", "simdutf", "simdutf_bulk");
  std::printf("%-9s %8s | %10s %10s %10s | %12s\n", "", "(units)", "GiB/s",
              "GiB/s", "GiB/s", "GiB/s");
  std::printf("------------------------------------------------------------------------------\n");

  for (std::string_view lang : langs) {
    std::string path =
        std::string(UNICODE_LIPSUM_DIR) + "/" + std::string(lang) + "-Lipsum.utf16.txt";
    std::vector<char16_t> u = load_utf16le(path);
    if (u.empty()) { std::printf("%-9s (no data)\n", std::string(lang).c_str()); continue; }
    std::size_t const input_bytes = u.size() * sizeof(char16_t);

    // Per-chunk output buffer: worst case 3 bytes per BMP unit (4 per surrogate
    // pair == 2/unit), so 3*chunk is a safe reused buffer.
    std::vector<char8_t> out(3 * 8192 + 16);

    // Bulk simdutf ceiling: one call over the whole corpus.
    std::vector<char8_t> bulk(u.size() * 3 + 16);
    auto bulk_t = measure(
        [&] {
          return (std::uint64_t)simdutf::convert_utf16le_to_utf8(
              u.data(), u.size(), reinterpret_cast<char*>(bulk.data()));
        },
        input_bytes);

    for (std::size_t chunk : chunks) {
      auto sc = measure([&] { return run_chunked<kernel::scalar>(u.data(), u.size(), chunk, out.data(), out.size()); }, input_bytes);
      auto si = measure([&] { return run_chunked<kernel::stdsimd>(u.data(), u.size(), chunk, out.data(), out.size()); }, input_bytes);
      auto su = measure([&] { return run_chunked<kernel::simdutf>(u.data(), u.size(), chunk, out.data(), out.size()); }, input_bytes);
      std::printf("%-9s %8zu | %10.3f %10.3f %10.3f | %12.3f\n",
                  std::string(lang).c_str(), chunk, sc.gibps, si.gibps, su.gibps,
                  bulk_t.gibps);
    }
    std::printf("------------------------------------------------------------------------------\n");
  }
  return 0;
}
