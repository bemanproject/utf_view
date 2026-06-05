// SPDX-License-Identifier: BSL-1.0
//
// Independent correctness tests for the std::simd UTF-16 -> UTF-8 chunk kernel.
// Not wired into utf_view; this exercises the kernel in isolation:
//   * hardcoded expected byte sequences for each UTF-8 length class,
//   * boundary-trim, output-capacity, and ill-formed-input contracts,
//   * a SIMD-vs-scalar differential over the unicode_lipsum corpora,
//   * the same checks again at compile time via static_assert.

#include <beman/utf_view/detail/simd_transcode.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace sk = beman::utf_view::detail::simd_kernel;

namespace {

int g_failures = 0;

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);              \
      ++g_failures;                                                            \
    }                                                                          \
  } while (0)

// Transcode `in` fully (generous output buffer) and compare bytes + result.
bool expect_bytes(std::u16string_view in, std::span<unsigned const> want,
                  bool want_ok = true) {
  std::vector<char8_t> out(in.size() * 4 + 8);
  auto r = sk::transcode_utf16_to_utf8(in.data(), in.size(), out.data(), out.size());
  bool ok = (r.ok == want_ok) && (r.output_produced == want.size());
  if (ok) {
    std::size_t k = 0;
    for (unsigned b : want)
      ok = ok && (static_cast<unsigned>(static_cast<std::uint8_t>(out[k++])) == b);
  }
  if (!ok) {
    std::printf("  expect_bytes mismatch: ok=%d produced=%zu consumed=%zu\n",
                r.ok, r.output_produced, r.input_consumed);
  }
  return ok;
}
bool expect_bytes(std::u16string_view in, std::initializer_list<unsigned> want,
                  bool want_ok = true) {
  return expect_bytes(in, std::span<unsigned const>(want.begin(), want.size()), want_ok);
}

std::vector<char> read_file(std::string const& path) {
  std::ifstream f(path, std::ios::binary);
  return std::vector<char>(std::istreambuf_iterator<char>(f),
                           std::istreambuf_iterator<char>{});
}

std::vector<char16_t> load_utf16le(std::string const& path) {
  std::vector<char> bytes = read_file(path);
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

// --- compile-time checks ---------------------------------------------------
// Drive a fixed input through both paths inside a constant expression.
consteval bool ce_basic() {
  char16_t const in[] = {u'A', 0x00E9 /*2B*/, 0x4E2D /*3B*/};
  char8_t out[16] = {};
  auto r = sk::transcode_utf16_to_utf8(in, 3, out, sizeof(out));
  // 'A' -> 41 ; é -> C3 A9 ; 中 -> E4 B8 AD  => 6 bytes, all consumed
  unsigned const want[] = {0x41, 0xC3, 0xA9, 0xE4, 0xB8, 0xAD};
  if (!r.ok || r.input_consumed != 3 || r.output_produced != 6)
    return false;
  for (std::size_t k = 0; k < 6; ++k)
    if (static_cast<unsigned>(static_cast<std::uint8_t>(out[k])) != want[k])
      return false;
  return true;
}

// All-ASCII block longer than a SIMD register, exercising the vector fast path
// in a constant expression.
consteval bool ce_ascii_block() {
  char16_t in[64];
  char8_t out[64];
  for (int k = 0; k < 64; ++k)
    in[k] = static_cast<char16_t>('a' + (k % 26));
  auto r = sk::transcode_utf16_to_utf8(in, 64, out, sizeof(out));
  if (!r.ok || r.input_consumed != 64 || r.output_produced != 64)
    return false;
  for (int k = 0; k < 64; ++k)
    if (static_cast<unsigned>(static_cast<std::uint8_t>(out[k])) !=
        static_cast<unsigned>('a' + (k % 26)))
      return false;
  return true;
}

static_assert(ce_basic(), "constexpr UTF-16->UTF-8 basic mixed-length");
static_assert(ce_ascii_block(), "constexpr UTF-16->UTF-8 ASCII SIMD block");

} // namespace

int main() {
  // ---- length classes ----
  CHECK(expect_bytes(u"A", {0x41}));
  CHECK(expect_bytes(u"é", {0xC3, 0xA9}));                 // é, 2-byte
  CHECK(expect_bytes(u"中", {0xE4, 0xB8, 0xAD}));           // 中, 3-byte
  CHECK(expect_bytes(u"\U0001F600", {0xF0, 0x9F, 0x98, 0x80})); // 😀, 4-byte (surrogate pair)
  CHECK(expect_bytes(u"Aé中\U0001F600",
                     {0x41, 0xC3, 0xA9, 0xE4, 0xB8, 0xAD, 0xF0, 0x9F, 0x98, 0x80}));

  // ---- homogeneous blocks longer than a register (force each SIMD path) ----
  {
    std::u16string ascii(100, u'x');
    std::vector<unsigned> want(100, 0x78);
    CHECK(expect_bytes(ascii, std::span<unsigned const>(want)));
  }
  {
    std::u16string two(100, u'é');
    std::vector<unsigned> want;
    for (int k = 0; k < 100; ++k) { want.push_back(0xC3); want.push_back(0xA9); }
    CHECK(expect_bytes(two, std::span<unsigned const>(want)));
  }
  {
    std::u16string three(100, u'中');
    std::vector<unsigned> want;
    for (int k = 0; k < 100; ++k) { want.push_back(0xE4); want.push_back(0xB8); want.push_back(0xAD); }
    CHECK(expect_bytes(three, std::span<unsigned const>(want)));
  }

  // ---- boundary trim: trailing lone high surrogate is carried, not an error ----
  {
    char16_t const in[] = {u'A', 0xD83D /* high half of 😀 */};
    char8_t out[16];
    auto r = sk::transcode_utf16_to_utf8(in, 2, out, sizeof(out));
    CHECK(r.ok);
    CHECK(r.input_consumed == 1); // 'A' only; high surrogate carried to next chunk
    CHECK(r.output_produced == 1);
  }

  // ---- output capacity: stop on the last code point that fits ----
  {
    char16_t const in[] = {u'A', u'B', 0x4E2D}; // 1 + 1 + 3 bytes
    char8_t out[4];                              // room for AB only (中 needs 3)
    auto r = sk::transcode_utf16_to_utf8(in, 3, out, 4);
    CHECK(r.ok);
    CHECK(r.input_consumed == 2);
    CHECK(r.output_produced == 2);
  }

  // ---- ill-formed: lone low surrogate, and high not followed by low ----
  {
    char16_t const in[] = {u'A', 0xDC00}; // lone low surrogate at offset 1
    char8_t out[16];
    auto r = sk::transcode_utf16_to_utf8(in, 2, out, sizeof(out));
    CHECK(!r.ok);
    CHECK(r.input_consumed == 1); // valid prefix 'A'
    CHECK(r.output_produced == 1);
  }
  {
    char16_t const in[] = {0xD83D, u'A'}; // high surrogate followed by non-low
    char8_t out[16];
    auto r = sk::transcode_utf16_to_utf8(in, 2, out, sizeof(out));
    CHECK(!r.ok);
    CHECK(r.input_consumed == 0);
  }

  std::printf("hand-written cases done (failures so far: %d)\n", g_failures);

  // ---- SIMD vs scalar differential over the unicode_lipsum corpora ----
#ifdef UNICODE_LIPSUM_DIR
  for (std::string_view lang :
       {"Latin", "Arabic", "Chinese", "Japanese", "Korean", "Emoji"}) {
    std::string path = std::string(UNICODE_LIPSUM_DIR) + "/" + std::string(lang) +
                       "-Lipsum.utf16.txt";
    std::vector<char16_t> u = load_utf16le(path);
    if (u.empty()) {
      std::printf("  (skip %s: no data)\n", std::string(lang).c_str());
      continue;
    }
    std::vector<char8_t> a(u.size() * 4 + 8), b(u.size() * 4 + 8);
    auto rs = sk::transcode_utf16_to_utf8_scalar(u.data(), u.size(), a.data(), a.size());
#if BEMAN_UTF_VIEW_HAS_STD_SIMD
    auto rv = sk::transcode_utf16_to_utf8_simd(u.data(), u.size(), b.data(), b.size());
    bool agree = rs.ok == rv.ok && rs.input_consumed == rv.input_consumed &&
                 rs.output_produced == rv.output_produced &&
                 std::memcmp(a.data(), b.data(), rs.output_produced) == 0;
    CHECK(agree);
    std::printf("  %-9s consumed=%zu produced=%zu ok=%d  SIMD==scalar=%d\n",
                std::string(lang).c_str(), rs.input_consumed, rs.output_produced,
                rs.ok, agree);
#else
    std::printf("  %-9s consumed=%zu produced=%zu ok=%d  (std::simd unavailable; scalar only)\n",
                std::string(lang).c_str(), rs.input_consumed, rs.output_produced, rs.ok);
#endif
  }
#else
  std::printf("  (UNICODE_LIPSUM_DIR not defined; differential test skipped)\n");
#endif

  std::printf(g_failures ? "\nFAILED (%d)\n" : "\nPASSED\n", g_failures);
  return g_failures ? 1 : 0;
}
