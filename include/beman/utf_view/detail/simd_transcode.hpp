// SPDX-License-Identifier: BSL-1.0
//
// std::simd UTF transcoding kernel -- chunk-by-chunk, header-only, inline,
// constexpr-friendly. PROOF OF CONCEPT: UTF-16 -> UTF-8 only.
//
// This is the standardizable replacement for the simdutf stand-in currently
// wired into utf_view's refill_chunk_simd(). It is NOT yet integrated into
// utf_view; see simd_kernel/test and simd_kernel/bench for independent
// correctness + throughput exercises.
//
// Interface B ("chunk-native"): the kernel owns the chunk boundary trim and the
// output-capacity bound. It transcodes a *prefix* of the input that ends on a
// complete code point and fits the output buffer, reporting how much input it
// consumed and how much output it produced. utf_view would then set
// to_increment_ = input_consumed and buf_.resize(output_produced).
//
// Error contract (Unicode 3.9.6 Substitution of Maximal Subparts is NOT done
// here): on the first ill-formed unit the kernel STOPS and returns ok == false
// with input_consumed == offset of the first bad unit, so the caller can
// scalar-handle the SMS replacement and resync. A high surrogate with no room
// for its low half in this chunk is NOT an error -- it is left unconsumed
// (boundary trim) so the next chunk picks it up.

#ifndef BEMAN_UTF_VIEW_DETAIL_SIMD_TRANSCODE_HPP
#define BEMAN_UTF_VIEW_DETAIL_SIMD_TRANSCODE_HPP

#include <cstddef>
#include <cstdint>
#include <span>

#if !defined(BEMAN_UTF_VIEW_SIMD_NO_STD_SIMD) && __has_include(<simd>)
#  include <simd>
#  if defined(__glibcxx_simd) || defined(__cpp_lib_simd)
#    define BEMAN_UTF_VIEW_HAS_STD_SIMD 1
#  endif
#endif
#ifndef BEMAN_UTF_VIEW_HAS_STD_SIMD
#  define BEMAN_UTF_VIEW_HAS_STD_SIMD 0
#endif

namespace beman::utf_view::detail::simd_kernel {

struct transcode_result {
  std::size_t input_consumed{};  // input code units consumed (== a complete-code-point boundary)
  std::size_t output_produced{}; // output code units written to out[0..output_produced)
  bool ok{true};                 // false iff stopped on an ill-formed input unit
};

// ---------------------------------------------------------------------------
// Scalar reference path. Also the constexpr/fallback path and the per-code-point
// tail the SIMD path defers to. Correctness lives here; the SIMD path only
// accelerates whole homogeneous blocks and hands everything else back to this.
// ---------------------------------------------------------------------------
constexpr transcode_result transcode_utf16_to_utf8_scalar(
    char16_t const* in, std::size_t in_len, char8_t* out, std::size_t out_cap) noexcept {
  std::size_t i = 0;
  std::size_t o = 0;
  while (i < in_len) {
    std::uint32_t const u = in[i];
    if (u < 0x80u) {
      if (o + 1 > out_cap) break;
      out[o++] = static_cast<char8_t>(u);
      i += 1;
    } else if (u < 0x800u) {
      if (o + 2 > out_cap) break;
      out[o++] = static_cast<char8_t>(0xC0u | (u >> 6));
      out[o++] = static_cast<char8_t>(0x80u | (u & 0x3Fu));
      i += 1;
    } else if (u < 0xD800u || u > 0xDFFFu) { // BMP, not a surrogate
      if (o + 3 > out_cap) break;
      out[o++] = static_cast<char8_t>(0xE0u | (u >> 12));
      out[o++] = static_cast<char8_t>(0x80u | ((u >> 6) & 0x3Fu));
      out[o++] = static_cast<char8_t>(0x80u | (u & 0x3Fu));
      i += 1;
    } else if (u <= 0xDBFFu) { // high surrogate -> expect a low surrogate next
      if (i + 1 >= in_len) break;          // low half not in this chunk: carry it
      std::uint32_t const u2 = in[i + 1];
      if (u2 < 0xDC00u || u2 > 0xDFFFu)     // not a low surrogate: ill-formed
        return {.input_consumed = i, .output_produced = o, .ok = false};
      if (o + 4 > out_cap) break;
      std::uint32_t const cp = 0x10000u + ((u - 0xD800u) << 10) + (u2 - 0xDC00u);
      out[o++] = static_cast<char8_t>(0xF0u | (cp >> 18));
      out[o++] = static_cast<char8_t>(0x80u | ((cp >> 12) & 0x3Fu));
      out[o++] = static_cast<char8_t>(0x80u | ((cp >> 6) & 0x3Fu));
      out[o++] = static_cast<char8_t>(0x80u | (cp & 0x3Fu));
      i += 2;
    } else { // lone low surrogate: ill-formed
      return {.input_consumed = i, .output_produced = o, .ok = false};
    }
  }
  return {.input_consumed = i, .output_produced = o, .ok = true};
}

#if BEMAN_UTF_VIEW_HAS_STD_SIMD

namespace sd = std::simd;

// SIMD-accelerated path. Homogeneous blocks (all-ASCII, all-2-byte, all-3-byte
// BMP) are transcoded with std::simd; any block that is mixed-length or contains
// a surrogate is handed to the scalar reference for a single code point, then the
// block loop resumes. This is constexpr-clean (libstdc++ std::simd evaluates in
// constant expressions), so there is no separate consteval fence.
//
// What is intentionally NOT here yet: the general mixed-length SIMD compaction
// (the shuffle-table core of simdutf). Homogeneous-block detection is what lets
// us avoid per-lane compaction; runs of a single byte-length vectorize, and the
// boundaries fall to scalar. CJK ideograph runs (3-byte) and ASCII runs benefit;
// text that alternates lengths every few units stays near scalar speed.
constexpr transcode_result transcode_utf16_to_utf8_simd(
    char16_t const* in, std::size_t in_len, char8_t* out, std::size_t out_cap) noexcept {
  using V16 = sd::vec<char16_t>;
  constexpr std::size_t W = V16::size();
  using V8 = sd::vec<char8_t, W>;

  // Per-lane index map for an N-way interleave of cat(b0, b1, ...): output lane k
  // comes from byte (k % N) of code unit (k / N), i.e. source lane
  // (k % N) * W + (k / N) in the concatenated [b0 | b1 | ... ] vector.
  auto const ascii_byte = [](char16_t const* p) constexpr {
    return V8(sd::unchecked_load<V16>(p, static_cast<std::ptrdiff_t>(W), sd::flag_default));
  };

  std::size_t i = 0;
  std::size_t o = 0;
  while (i + W <= in_len) {
    V16 const v = sd::unchecked_load<V16>(in + i, static_cast<std::ptrdiff_t>(W), sd::flag_default);

    // ---- all-ASCII block: 1 byte per unit, just narrow + store ----
    if (sd::all_of(v < V16(char16_t{0x80}))) {
      if (o + W > out_cap) break;
      sd::unchecked_store(ascii_byte(in + i), std::span<char8_t>(out + o, W), sd::flag_default);
      i += W;
      o += W;
      continue;
    }

    // Surrogate present anywhere in the block -> the homogeneous SIMD encoders
    // below assume BMP scalars; let the scalar reader handle the surrogate (and
    // any pair that may straddle the block end) one code point at a time.
    bool const has_surrogate =
        sd::any_of((v & V16(char16_t{0xF800})) == V16(char16_t{0xD800}));

    if (!has_surrogate) {
      // ---- all-2-byte block: every unit in [0x80, 0x800) -> exactly 2 bytes ----
      if (sd::all_of(v >= V16(char16_t{0x80})) && sd::all_of(v < V16(char16_t{0x800}))) {
        if (o + 2 * W > out_cap) break;
        V8 const b0 = V8((v >> 6) | V16(char16_t{0xC0}));
        V8 const b1 = V8((v & V16(char16_t{0x3F})) | V16(char16_t{0x80}));
        auto const pair = sd::cat(b0, b1); // [b0_0..b0_{W-1} | b1_0..b1_{W-1}]
        // interleave: out lane k -> (k%2)*W + (k/2)
        auto const woven = sd::permute<2 * W>(pair, [](auto k) constexpr {
          return (int(k) % 2) * int(W) + int(k) / 2;
        });
        sd::unchecked_store(woven, std::span<char8_t>(out + o, 2 * W), sd::flag_default);
        i += W;
        o += 2 * W;
        continue;
      }

      // ---- all-3-byte BMP block: every unit in [0x800, 0x10000) -> 3 bytes ----
      if (sd::all_of(v >= V16(char16_t{0x800}))) {
        if (o + 3 * W > out_cap) break;
        V8 const b0 = V8((v >> 12) | V16(char16_t{0xE0}));
        V8 const b1 = V8(((v >> 6) & V16(char16_t{0x3F})) | V16(char16_t{0x80}));
        V8 const b2 = V8((v & V16(char16_t{0x3F})) | V16(char16_t{0x80}));
        auto const triple = sd::cat(b0, b1, b2);
        auto const woven = sd::permute<3 * W>(triple, [](auto k) constexpr {
          return (int(k) % 3) * int(W) + int(k) / 3;
        });
        sd::unchecked_store(woven, std::span<char8_t>(out + o, 3 * W), sd::flag_default);
        i += W;
        o += 3 * W;
        continue;
      }
    }

    // Mixed-length block (or one containing a surrogate): emit exactly one code
    // point with the scalar reader, then re-enter the block loop. A scalar stop
    // (output full, ill-formed unit, or straddling pair at chunk end) ends here.
    transcode_result const step =
        transcode_utf16_to_utf8_scalar(in + i, in_len - i, out + o, out_cap - o);
    o += step.output_produced;
    i += step.input_consumed;
    if (!step.ok)
      return {.input_consumed = i, .output_produced = o, .ok = false};
    if (step.input_consumed == 0) // made no progress: output full or carry -> stop
      return {.input_consumed = i, .output_produced = o, .ok = true};
  }

  // Tail (< W input units left): finish with the scalar reference.
  transcode_result const tail =
      transcode_utf16_to_utf8_scalar(in + i, in_len - i, out + o, out_cap - o);
  return {.input_consumed = i + tail.input_consumed,
          .output_produced = o + tail.output_produced,
          .ok = tail.ok};
}

#else // !BEMAN_UTF_VIEW_HAS_STD_SIMD

// std::simd is unavailable on this target (e.g. GCC 16 libstdc++ gates <simd> on
// __SSE2__, so it is absent on aarch64). Provide the same symbol so callers and
// the A/B benchmark stay source-identical; it just forwards to the scalar path.
constexpr transcode_result transcode_utf16_to_utf8_simd(
    char16_t const* in, std::size_t in_len, char8_t* out, std::size_t out_cap) noexcept {
  return transcode_utf16_to_utf8_scalar(in, in_len, out, out_cap);
}

#endif // BEMAN_UTF_VIEW_HAS_STD_SIMD

// Default entry point: SIMD if available, else scalar. Same signature/contract
// either way so callers and the A/B benchmark are source-identical.
constexpr transcode_result transcode_utf16_to_utf8(
    char16_t const* in, std::size_t in_len, char8_t* out, std::size_t out_cap) noexcept {
  return transcode_utf16_to_utf8_simd(in, in_len, out, out_cap);
}

} // namespace beman::utf_view::detail::simd_kernel

#endif // BEMAN_UTF_VIEW_DETAIL_SIMD_TRANSCODE_HPP
