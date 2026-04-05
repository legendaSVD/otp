#include <asmjit/core/api-build_p.h>
#include <asmjit/core/cpuinfo.h>
#include <asmjit/support/support.h>
#include <atomic>
#if ASMJIT_ARCH_X86
  #if defined(_MSC_VER)
    #include <intrin.h>
  #endif
#endif
#if ASMJIT_ARCH_ARM
  #if !defined(_WIN32)
    #include <errno.h>
    #include <sys/utsname.h>
  #endif
  #if defined(__APPLE__)
    #include <mach/machine.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
  #endif
  #if (defined(__linux__) || defined(__FreeBSD__))
    #include <sys/auxv.h>
    #define ASMJIT_ARM_DETECT_VIA_HWCAPS
  #endif
  #if ASMJIT_ARCH_ARM >= 64 && defined(__GNUC__) && defined(__linux__) && 0
    #define ASMJIT_ARM_DETECT_VIA_CPUID
  #endif
  #if ASMJIT_ARCH_ARM >= 64 && defined(__OpenBSD__)
    #include <machine/cpu.h>
    #include <sys/sysctl.h>
  #endif
  #if ASMJIT_ARCH_ARM >= 64 && defined(__NetBSD__)
    #include <sys/sysctl.h>
  #endif
#endif
#if !defined(_WIN32) && (ASMJIT_ARCH_X86 || ASMJIT_ARCH_ARM)
  #include <unistd.h>
#endif
ASMJIT_BEGIN_NAMESPACE
#if defined(_WIN32)
static inline uint32_t detect_hw_thread_count() noexcept {
  SYSTEM_INFO info;
  ::GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
}
#elif defined(_SC_NPROCESSORS_ONLN)
static inline uint32_t detect_hw_thread_count() noexcept {
  long res = ::sysconf(_SC_NPROCESSORS_ONLN);
  return res <= 0 ? uint32_t(1) : uint32_t(res);
}
#else
static inline uint32_t detect_hw_thread_count() noexcept {
  return 1;
}
#endif
#if ASMJIT_ARCH_X86
namespace x86 {
using Ext = CpuFeatures::X86;
struct cpuid_t { uint32_t eax, ebx, ecx, edx; };
struct xgetbv_t { uint32_t eax, edx; };
static inline void cpuid_query(cpuid_t* out, uint32_t in_eax, uint32_t in_ecx = 0) noexcept {
#if defined(_MSC_VER)
  __cpuidex(reinterpret_cast<int*>(out), in_eax, in_ecx);
#elif defined(__GNUC__) && ASMJIT_ARCH_X86 == 32
  __asm__ __volatile__(
    "mov %%ebx, %%edi\n"
    "cpuid\n"
    "xchg %%edi, %%ebx\n" : "=a"(out->eax), "=D"(out->ebx), "=c"(out->ecx), "=d"(out->edx) : "a"(in_eax), "c"(in_ecx));
#elif defined(__GNUC__) && ASMJIT_ARCH_X86 == 64
  __asm__ __volatile__(
    "mov %%rbx, %%rdi\n"
    "cpuid\n"
    "xchg %%rdi, %%rbx\n" : "=a"(out->eax), "=D"(out->ebx), "=c"(out->ecx), "=d"(out->edx) : "a"(in_eax), "c"(in_ecx));
#else
  #error "[asmjit] x86::cpuid_query() - Unsupported compiler."
#endif
}
static inline void xgetbv_query(xgetbv_t* out, uint32_t in_ecx) noexcept {
#if defined(_MSC_VER)
  uint64_t value = _xgetbv(in_ecx);
  out->eax = uint32_t(value & 0xFFFFFFFFu);
  out->edx = uint32_t(value >> 32);
#elif defined(__GNUC__)
  uint32_t out_eax;
  uint32_t out_edx;
  __asm__ __volatile__(".byte 0x0F, 0x01, 0xD0" : "=a"(out_eax), "=d"(out_edx) : "c"(in_ecx));
  out->eax = out_eax;
  out->edx = out_edx;
#else
  out->eax = 0;
  out->edx = 0;
#endif
}
static inline void simplify_cpu_vendor(CpuInfo& cpu, uint32_t d0, uint32_t d1, uint32_t d2) noexcept {
  struct Vendor {
    char normalized[8];
    union { char text[12]; uint32_t d[3]; };
  };
  static const Vendor table[] = {
    { { 'A', 'M', 'D'                     }, {{ 'A', 'u', 't', 'h', 'e', 'n', 't', 'i', 'c', 'A', 'M', 'D' }} },
    { { 'I', 'N', 'T', 'E', 'L'           }, {{ 'G', 'e', 'n', 'u', 'i', 'n', 'e', 'I', 'n', 't', 'e', 'l' }} },
    { { 'V', 'I', 'A'                     }, {{ 'C', 'e', 'n', 't', 'a', 'u', 'r', 'H', 'a', 'u', 'l', 's' }} },
    { { 'V', 'I', 'A'                     }, {{ 'V', 'I', 'A',  0 , 'V', 'I', 'A',  0 , 'V', 'I', 'A',  0  }} },
    { { 'U', 'N', 'K', 'N', 'O', 'W', 'N' }, {{ 0                                                          }} }
  };
  uint32_t i;
  for (i = 0; i < ASMJIT_ARRAY_SIZE(table) - 1; i++) {
    if (table[i].d[0] == d0 && table[i].d[1] == d1 && table[i].d[2] == d2) {
      break;
    }
  }
  memcpy(cpu._vendor.str, table[i].normalized, 8);
}
static ASMJIT_FAVOR_SIZE void simplify_cpu_brand(char* s) noexcept {
  char* d = s;
  char c = s[0];
  char prev = 0;
  s[0] = '\0';
  for (;;) {
    if (!c) {
      break;
    }
    if (!(c == ' ' && (prev == '@' || s[1] == ' ' || s[1] == '@' || s[1] == '\0'))) {
      *d++ = c;
      prev = c;
    }
    c = *++s;
    s[0] = '\0';
  }
  d[0] = '\0';
}
static ASMJIT_FAVOR_SIZE void detect_x86_cpu(CpuInfo& cpu) noexcept {
  using Support::bit_test;
  cpuid_t regs;
  xgetbv_t xcr0 { 0, 0 };
  CpuFeatures::X86& features = cpu.features().x86();
  cpu._was_detected = true;
  cpu._max_logical_processors = 1;
  features.add(Ext::kI486);
  cpuid_query(&regs, 0x0);
  uint32_t max_id = regs.eax;
  uint32_t max_sub_leaf_id_0x7 = 0;
  simplify_cpu_vendor(cpu, regs.ebx, regs.edx, regs.ecx);
  if (max_id >= 0x01u) {
    cpuid_query(&regs, 0x1);
    uint32_t model_id  = (regs.eax >> 4) & 0x0F;
    uint32_t family_id = (regs.eax >> 8) & 0x0F;
    if (family_id == 0x06u || family_id == 0x0Fu) {
      model_id += (((regs.eax >> 16) & 0x0Fu) << 4);
    }
    if (family_id == 0x0Fu) {
      family_id += ((regs.eax >> 20) & 0xFFu);
    }
    cpu._model_id = model_id;
    cpu._family_id = family_id;
    cpu._brand_id = (regs.ebx) & 0xFF;
    cpu._processor_type = (regs.eax >> 12) & 0x03;
    cpu._max_logical_processors = (regs.ebx >> 16) & 0xFF;
    cpu._stepping = (regs.eax) & 0x0F;
    cpu._cache_line_size  = ((regs.ebx >> 8) & 0xFF) * 8;
    features.add_if(bit_test(regs.ecx,  0), Ext::kSSE3);
    features.add_if(bit_test(regs.ecx,  1), Ext::kPCLMULQDQ);
    features.add_if(bit_test(regs.ecx,  3), Ext::kMONITOR);
    features.add_if(bit_test(regs.ecx,  5), Ext::kVMX);
    features.add_if(bit_test(regs.ecx,  6), Ext::kSMX);
    features.add_if(bit_test(regs.ecx,  9), Ext::kSSSE3);
    features.add_if(bit_test(regs.ecx, 13), Ext::kCMPXCHG16B);
    features.add_if(bit_test(regs.ecx, 19), Ext::kSSE4_1);
    features.add_if(bit_test(regs.ecx, 20), Ext::kSSE4_2);
    features.add_if(bit_test(regs.ecx, 22), Ext::kMOVBE);
    features.add_if(bit_test(regs.ecx, 23), Ext::kPOPCNT);
    features.add_if(bit_test(regs.ecx, 25), Ext::kAESNI);
    features.add_if(bit_test(regs.ecx, 26), Ext::kXSAVE);
    features.add_if(bit_test(regs.ecx, 27), Ext::kOSXSAVE);
    features.add_if(bit_test(regs.ecx, 30), Ext::kRDRAND);
    features.add_if(bit_test(regs.edx,  0), Ext::kFPU);
    features.add_if(bit_test(regs.edx,  4), Ext::kRDTSC);
    features.add_if(bit_test(regs.edx,  5), Ext::kMSR);
    features.add_if(bit_test(regs.edx,  8), Ext::kCMPXCHG8B);
    features.add_if(bit_test(regs.edx, 15), Ext::kCMOV);
    features.add_if(bit_test(regs.edx, 19), Ext::kCLFLUSH);
    features.add_if(bit_test(regs.edx, 23), Ext::kMMX);
    features.add_if(bit_test(regs.edx, 24), Ext::kFXSR);
    features.add_if(bit_test(regs.edx, 25), Ext::kSSE, Ext::kMMX2);
    features.add_if(bit_test(regs.edx, 26), Ext::kSSE2, Ext::kSSE);
    features.add_if(bit_test(regs.edx, 28), Ext::kMT);
    if (features.has_xsave() && features.has_osxsave()) {
      xgetbv_query(&xcr0, 0);
    }
    if (bit_test(regs.ecx, 28)) {
      if ((xcr0.eax & 0x00000006u) == 0x00000006u) {
        features.add(Ext::kAVX);
        features.add_if(bit_test(regs.ecx, 12), Ext::kFMA);
        features.add_if(bit_test(regs.ecx, 29), Ext::kF16C);
      }
    }
  }
  constexpr uint32_t kXCR0_AMX_Bits = 0x3u << 17;
  bool amx_enabled = (xcr0.eax & kXCR0_AMX_Bits) == kXCR0_AMX_Bits;
#if defined(__APPLE__)
  bool avx512_enabled = true;
#else
  constexpr uint32_t kXCR0_AVX512_Bits = (0x3u << 1) | (0x7u << 5);
  bool avx512_enabled = (xcr0.eax & kXCR0_AVX512_Bits) == kXCR0_AVX512_Bits;
#endif
  bool avx10_enabled = false;
  if (max_id >= 0x07u) {
    cpuid_query(&regs, 0x7);
    max_sub_leaf_id_0x7 = regs.eax;
    features.add_if(bit_test(regs.ebx,  0), Ext::kFSGSBASE);
    features.add_if(bit_test(regs.ebx,  3), Ext::kBMI);
    features.add_if(bit_test(regs.ebx,  7), Ext::kSMEP);
    features.add_if(bit_test(regs.ebx,  8), Ext::kBMI2);
    features.add_if(bit_test(regs.ebx,  9), Ext::kERMS);
    features.add_if(bit_test(regs.ebx, 18), Ext::kRDSEED);
    features.add_if(bit_test(regs.ebx, 19), Ext::kADX);
    features.add_if(bit_test(regs.ebx, 20), Ext::kSMAP);
    features.add_if(bit_test(regs.ebx, 23), Ext::kCLFLUSHOPT);
    features.add_if(bit_test(regs.ebx, 24), Ext::kCLWB);
    features.add_if(bit_test(regs.ebx, 29), Ext::kSHA);
    features.add_if(bit_test(regs.ecx,  0), Ext::kPREFETCHWT1);
    features.add_if(bit_test(regs.ecx,  4), Ext::kOSPKE);
    features.add_if(bit_test(regs.ecx,  5), Ext::kWAITPKG);
    features.add_if(bit_test(regs.ecx,  7), Ext::kCET_SS);
    features.add_if(bit_test(regs.ecx,  8), Ext::kGFNI);
    features.add_if(bit_test(regs.ecx,  9), Ext::kVAES);
    features.add_if(bit_test(regs.ecx, 10), Ext::kVPCLMULQDQ);
    features.add_if(bit_test(regs.ecx, 22), Ext::kRDPID);
    features.add_if(bit_test(regs.ecx, 23), Ext::kKL);
    features.add_if(bit_test(regs.ecx, 25), Ext::kCLDEMOTE);
    features.add_if(bit_test(regs.ecx, 27), Ext::kMOVDIRI);
    features.add_if(bit_test(regs.ecx, 28), Ext::kMOVDIR64B);
    features.add_if(bit_test(regs.ecx, 29), Ext::kENQCMD);
    features.add_if(bit_test(regs.edx,  4), Ext::kFSRM);
    features.add_if(bit_test(regs.edx,  5), Ext::kUINTR);
    features.add_if(bit_test(regs.edx, 14), Ext::kSERIALIZE);
    features.add_if(bit_test(regs.edx, 16), Ext::kTSXLDTRK);
    features.add_if(bit_test(regs.edx, 18), Ext::kPCONFIG);
    features.add_if(bit_test(regs.edx, 20), Ext::kCET_IBT);
    if (bit_test(regs.ebx, 5) && features.has_avx()) {
      features.add(Ext::kAVX2);
    }
    if (avx512_enabled && bit_test(regs.ebx, 16)) {
      features.add(Ext::kAVX512_F);
      features.add_if(bit_test(regs.ebx, 17), Ext::kAVX512_DQ);
      features.add_if(bit_test(regs.ebx, 21), Ext::kAVX512_IFMA);
      features.add_if(bit_test(regs.ebx, 28), Ext::kAVX512_CD);
      features.add_if(bit_test(regs.ebx, 30), Ext::kAVX512_BW);
      features.add_if(bit_test(regs.ebx, 31), Ext::kAVX512_VL);
      features.add_if(bit_test(regs.ecx,  1), Ext::kAVX512_VBMI);
      features.add_if(bit_test(regs.ecx,  6), Ext::kAVX512_VBMI2);
      features.add_if(bit_test(regs.ecx, 11), Ext::kAVX512_VNNI);
      features.add_if(bit_test(regs.ecx, 12), Ext::kAVX512_BITALG);
      features.add_if(bit_test(regs.ecx, 14), Ext::kAVX512_VPOPCNTDQ);
      features.add_if(bit_test(regs.edx,  8), Ext::kAVX512_VP2INTERSECT);
      features.add_if(bit_test(regs.edx, 23), Ext::kAVX512_FP16);
    }
    if (amx_enabled) {
      features.add_if(bit_test(regs.edx, 22), Ext::kAMX_BF16);
      features.add_if(bit_test(regs.edx, 24), Ext::kAMX_TILE);
      features.add_if(bit_test(regs.edx, 25), Ext::kAMX_INT8);
    }
  }
  if (max_sub_leaf_id_0x7 >= 1) {
    cpuid_query(&regs, 0x7, 1);
    features.add_if(bit_test(regs.eax,  0), Ext::kSHA512);
    features.add_if(bit_test(regs.eax,  1), Ext::kSM3);
    features.add_if(bit_test(regs.eax,  2), Ext::kSM4);
    features.add_if(bit_test(regs.eax,  3), Ext::kRAO_INT);
    features.add_if(bit_test(regs.eax,  7), Ext::kCMPCCXADD);
    features.add_if(bit_test(regs.eax, 10), Ext::kFZRM);
    features.add_if(bit_test(regs.eax, 11), Ext::kFSRS);
    features.add_if(bit_test(regs.eax, 12), Ext::kFSRC);
    features.add_if(bit_test(regs.eax, 19), Ext::kWRMSRNS);
    features.add_if(bit_test(regs.eax, 22), Ext::kHRESET);
    features.add_if(bit_test(regs.eax, 26), Ext::kLAM);
    features.add_if(bit_test(regs.eax, 27), Ext::kMSRLIST);
    features.add_if(bit_test(regs.eax, 31), Ext::kMOVRS);
    features.add_if(bit_test(regs.ecx,  5), Ext::kMSR_IMM);
    features.add_if(bit_test(regs.ebx,  1), Ext::kTSE);
    features.add_if(bit_test(regs.edx, 14), Ext::kPREFETCHI);
    features.add_if(bit_test(regs.edx, 18), Ext::kCET_SSS);
    features.add_if(bit_test(regs.edx, 21), Ext::kAPX_F);
    if (features.has_avx2()) {
      features.add_if(bit_test(regs.eax,  4), Ext::kAVX_VNNI);
      features.add_if(bit_test(regs.eax, 23), Ext::kAVX_IFMA);
      features.add_if(bit_test(regs.edx,  4), Ext::kAVX_VNNI_INT8);
      features.add_if(bit_test(regs.edx,  5), Ext::kAVX_NE_CONVERT);
      features.add_if(bit_test(regs.edx, 10), Ext::kAVX_VNNI_INT16);
    }
    if (features.has_avx512_f()) {
      features.add_if(bit_test(regs.eax,  5), Ext::kAVX512_BF16);
    }
    if (features.has_avx512_f()) {
      avx10_enabled = Support::bit_test(regs.edx, 19);
    }
    if (amx_enabled) {
      features.add_if(bit_test(regs.eax, 21), Ext::kAMX_FP16);
      features.add_if(bit_test(regs.edx,  8), Ext::kAMX_COMPLEX);
    }
  }
  if (max_id >= 0x0Du) {
    cpuid_query(&regs, 0xD, 1);
    features.add_if(bit_test(regs.eax, 0), Ext::kXSAVEOPT);
    features.add_if(bit_test(regs.eax, 1), Ext::kXSAVEC);
    features.add_if(bit_test(regs.eax, 3), Ext::kXSAVES);
  }
  if (max_id >= 0x0Eu) {
    cpuid_query(&regs, 0x0E, 0);
    features.add_if(bit_test(regs.ebx, 4), Ext::kPTWRITE);
  }
  if (max_id >= 0x19u && features.has_kl()) {
    cpuid_query(&regs, 0x19, 0);
    features.add_if(bit_test(regs.ebx, 0), Ext::kAESKLE);
    features.add_if(bit_test(regs.ebx, 0) && bit_test(regs.ebx, 2), Ext::kAESKLEWIDE_KL);
  }
  if (max_id >= 0x1Eu && features.has_amx_tile()) {
    cpuid_query(&regs, 0x1E, 1);
    features.add_if(bit_test(regs.eax, 0), Ext::kAMX_INT8);
    features.add_if(bit_test(regs.eax, 1), Ext::kAMX_BF16);
    features.add_if(bit_test(regs.eax, 2), Ext::kAMX_COMPLEX);
    features.add_if(bit_test(regs.eax, 3), Ext::kAMX_FP16);
    features.add_if(bit_test(regs.eax, 4), Ext::kAMX_FP8);
    features.add_if(bit_test(regs.eax, 5), Ext::kAMX_TRANSPOSE);
    features.add_if(bit_test(regs.eax, 6), Ext::kAMX_TF32);
    features.add_if(bit_test(regs.eax, 7), Ext::kAMX_AVX512);
    features.add_if(bit_test(regs.eax, 8), Ext::kAMX_MOVRS);
  }
  if (max_id >= 0x24u && avx10_enabled) {
    cpuid_query(&regs, 0x24, 0);
    uint32_t ver = regs.ebx & 0xFFu;
    features.add_if(ver >= 1u, Ext::kAVX10_1);
    features.add_if(ver >= 2u, Ext::kAVX10_2);
  }
  max_id = 0x80000000u;
  uint32_t i = max_id;
  constexpr uint32_t kHighestProcessedEAX = 0x8000001Fu;
  uint32_t* brand = cpu._brand.u32;
  do {
    cpuid_query(&regs, i);
    switch (i) {
      case 0x80000000u:
        max_id = Support::min<uint32_t>(regs.eax, kHighestProcessedEAX);
        break;
      case 0x80000001u:
        features.add_if(bit_test(regs.ecx,  0), Ext::kLAHFSAHF);
        features.add_if(bit_test(regs.ecx,  2), Ext::kSVM);
        features.add_if(bit_test(regs.ecx,  5), Ext::kLZCNT);
        features.add_if(bit_test(regs.ecx,  6), Ext::kSSE4A);
        features.add_if(bit_test(regs.ecx,  7), Ext::kMSSE);
        features.add_if(bit_test(regs.ecx,  8), Ext::kPREFETCHW);
        features.add_if(bit_test(regs.ecx, 12), Ext::kSKINIT);
        features.add_if(bit_test(regs.ecx, 15), Ext::kLWP);
        features.add_if(bit_test(regs.ecx, 21), Ext::kTBM);
        features.add_if(bit_test(regs.ecx, 29), Ext::kMONITORX);
        features.add_if(bit_test(regs.edx, 20), Ext::kNX);
        features.add_if(bit_test(regs.edx, 21), Ext::kFXSROPT);
        features.add_if(bit_test(regs.edx, 22), Ext::kMMX2);
        features.add_if(bit_test(regs.edx, 27), Ext::kRDTSCP);
        features.add_if(bit_test(regs.edx, 29), Ext::kPREFETCHW);
        features.add_if(bit_test(regs.edx, 30), Ext::k3DNOW2, Ext::kMMX2);
        features.add_if(bit_test(regs.edx, 31), Ext::kPREFETCHW);
        if (features.has_avx()) {
          features.add_if(bit_test(regs.ecx, 11), Ext::kXOP);
          features.add_if(bit_test(regs.ecx, 16), Ext::kFMA4);
        }
        if (cpu.is_vendor("AMD")) {
          features.add_if(bit_test(regs.ecx,  4), Ext::kALTMOVCR8);
        }
        break;
      case 0x80000002u:
      case 0x80000003u:
      case 0x80000004u:
        *brand++ = regs.eax;
        *brand++ = regs.ebx;
        *brand++ = regs.ecx;
        *brand++ = regs.edx;
        if (i == 0x80000004u)
          i = 0x80000008u - 1;
        break;
      case 0x80000008u:
        features.add_if(bit_test(regs.ebx,  0), Ext::kCLZERO);
        features.add_if(bit_test(regs.ebx,  0), Ext::kRDPRU);
        features.add_if(bit_test(regs.ebx,  8), Ext::kMCOMMIT);
        features.add_if(bit_test(regs.ebx,  9), Ext::kWBNOINVD);
        i = 0x8000001Fu - 1;
        break;
      case 0x8000001Fu:
        features.add_if(bit_test(regs.eax,  0), Ext::kSME);
        features.add_if(bit_test(regs.eax,  1), Ext::kSEV);
        features.add_if(bit_test(regs.eax,  3), Ext::kSEV_ES);
        features.add_if(bit_test(regs.eax,  4), Ext::kSEV_SNP);
        features.add_if(bit_test(regs.eax,  6), Ext::kRMPQUERY);
        break;
    }
  } while (++i <= max_id);
  simplify_cpu_brand(cpu._brand.str);
}
static ASMJIT_FAVOR_SIZE CpuHints recalculate_hints(const CpuInfo& cpu_info, const CpuFeatures::X86& features) noexcept {
  CpuHints hints {};
  if (features.has_avx2()) {
    hints |= CpuHints::kVecMaskedOps32 | CpuHints::kVecMaskedOps64;
  }
  if (features.has_avx512_bw()) {
    hints |= CpuHints::kVecMaskedOps8 | CpuHints::kVecMaskedOps16 | CpuHints::kVecMaskedOps32 | CpuHints::kVecMaskedOps64;
  }
  if (cpu_info.is_vendor("AMD")) {
    if (cpu_info.family_id() >= 0x19u) {
      hints |= CpuHints::kVecFastGather;
    }
    if (features.has_avx2()) {
      hints |= CpuHints::kVecFastIntMul32;
    }
    if (features.has_avx512_dq()) {
      hints |= CpuHints::kVecFastIntMul64;
    }
    if (features.has_avx512_f()) {
      hints |= CpuHints::kVecMaskedStore;
    }
  }
  if (cpu_info.is_vendor("INTEL")) {
    if (features.has_avx2()) {
      uint32_t family_id = cpu_info.family_id();
      uint32_t model_id = cpu_info.model_id();
      if (family_id == 0x06u) {
        switch (model_id) {
          case 0x8Fu:
          case 0x96u:
          case 0x97u:
          case 0x9Au:
          case 0x9Cu:
          case 0xAAu:
          case 0xACu:
          case 0xADu:
          case 0xAEu:
          case 0xAFu:
          case 0xBAu:
          case 0xB5u:
          case 0xB6u:
          case 0xB7u:
          case 0xBDu:
          case 0xBEu:
          case 0xBFu:
          case 0xC5u:
          case 0xC6u:
          case 0xCFu:
          case 0xDDu:
            hints |= CpuHints::kVecFastGather;
            break;
          default:
            break;
        }
      }
    }
  }
  return hints;
}
}
#endif
#if ASMJIT_ARCH_ARM
namespace arm {
using Ext = CpuFeatures::ARM;
#if defined(__linux__)
struct UNameKernelVersion {
  int parts[3];
  inline bool at_least(int major, int minor, int patch = 0) const noexcept {
    if (parts[0] >= major) {
      if (parts[0] > major) {
        return true;
      }
      if (parts[1] >= minor) {
        return parts[1] > minor ? true : parts[2] >= patch;
      }
    }
    return false;
  }
};
[[maybe_unused]]
static UNameKernelVersion get_kernel_version_via_uname() noexcept {
  UNameKernelVersion ver{};
  ver.parts[0] = -1;
  utsname buffer;
  if (uname(&buffer) != 0) {
    return ver;
  }
  size_t count = 0;
  char* p = buffer.release;
  while (*p) {
    uint32_t c = uint8_t(*p);
    if (c >= uint32_t('0') && c <= uint32_t('9')) {
      ver.parts[count] = int(strtol(p, &p, 10));
      if (++count == 3) {
        break;
      }
    }
    else if (c == '.' || c == '-') {
      p++;
    }
    else {
      break;
    }
  }
  return ver;
}
#endif
[[maybe_unused]]
static inline void populate_base_aarch32_features(CpuFeatures::ARM& features) noexcept {
  Support::maybe_unused(features);
}
[[maybe_unused]]
static inline void populate_base_aarch64_features(CpuFeatures::ARM& features) noexcept {
  features.add(Ext::kARMv6);
  features.add(Ext::kARMv7);
  features.add(Ext::kARMv8a);
  features.add(Ext::kASIMD);
  features.add(Ext::kFP);
  features.add(Ext::kIDIVA);
}
static inline void populate_base_arm_features(CpuInfo& cpu) noexcept {
#if ASMJIT_ARCH_ARM == 32
  populate_base_aarch32_features(cpu.features().arm());
#else
  populate_base_aarch64_features(cpu.features().arm());
#endif
}
[[maybe_unused]]
static ASMJIT_NOINLINE void populate_armv8a_features(CpuFeatures::ARM& features, uint32_t v) noexcept {
  switch (v) {
    default:
      [[fallthrough]];
    case 9:
      features.add(Ext::kCLRBHB, Ext::kCSSC, Ext::kPRFMSLC, Ext::kSPECRES2, Ext::kRAS2);
      [[fallthrough]];
    case 8:
      features.add(Ext::kHBC, Ext::kMOPS, Ext::kNMI);
      [[fallthrough]];
    case 7:
      features.add(Ext::kHCX, Ext::kPAN3, Ext::kWFXT, Ext::kXS);
      [[fallthrough]];
    case 6:
      features.add(Ext::kAMU1_1, Ext::kBF16, Ext::kECV, Ext::kFGT, Ext::kI8MM);
      [[fallthrough]];
    case 5:
      features.add(Ext::kBTI, Ext::kCSV2, Ext::kDPB2, Ext::kFLAGM2, Ext::kFRINTTS, Ext::kSB, Ext::kSPECRES, Ext::kSSBS);
      [[fallthrough]];
    case 4:
      features.add(Ext::kAMU1, Ext::kDIT, Ext::kDOTPROD, Ext::kFLAGM,
                   Ext::kLRCPC2, Ext::kLSE2, Ext::kMPAM, Ext::kNV,
                   Ext::kSEL2, Ext::kTLBIOS, Ext::kTLBIRANGE, Ext::kTRF);
      [[fallthrough]];
    case 3:
      features.add(Ext::kCCIDX, Ext::kFCMA, Ext::kJSCVT, Ext::kLRCPC, Ext::kPAUTH);
      [[fallthrough]];
    case 2:
      features.add(Ext::kDPB, Ext::kPAN2, Ext::kRAS, Ext::kUAO);
      [[fallthrough]];
    case 1:
      features.add(Ext::kCRC32, Ext::kLOR, Ext::kLSE, Ext::kPAN, Ext::kRDM, Ext::kVHE);
      [[fallthrough]];
    case 0:
      features.add(Ext::kASIMD, Ext::kFP, Ext::kIDIVA, Ext::kVFP_D32);
      break;
  }
}
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void populate_armv9a_features(CpuFeatures::ARM& features, uint32_t v) noexcept {
  populate_armv8a_features(features, v <= 4u ? 5u + v : 9u);
  switch (v) {
    default:
      [[fallthrough]];
    case 4:
      [[fallthrough]];
    case 3:
      [[fallthrough]];
    case 2:
      [[fallthrough]];
    case 1:
      [[fallthrough]];
    case 0:
      features.add(Ext::kRME, Ext::kSVE, Ext::kSVE2);
      break;
  }
}
[[maybe_unused]]
static ASMJIT_INLINE void merge_aarch64_cpuid_feature_na(
  CpuFeatures::ARM& features, uint64_t reg_bits, uint32_t offset,
  Ext::Id f0,
  Ext::Id f1 = Ext::kNone,
  Ext::Id f2 = Ext::kNone,
  Ext::Id f3 = Ext::kNone) noexcept {
  uint32_t val = uint32_t((reg_bits >> offset) & 0xFu);
  if (val == 0xFu) {
    return;
  }
  features.add_if(f0 != Ext::kNone, f0);
  features.add_if(f1 != Ext::kNone && val >= 1, f1);
  features.add_if(f2 != Ext::kNone && val >= 2, f2);
  features.add_if(f3 != Ext::kNone && val >= 3, f3);
}
[[maybe_unused]]
static ASMJIT_INLINE void merge_aarch64_cpuid_feature_1b(CpuFeatures::ARM& features, uint64_t reg_bits, uint32_t offset, Ext::Id f1) noexcept {
  features.add_if((reg_bits & (uint64_t(1) << offset)) != 0, f1);
}
[[maybe_unused]]
static ASMJIT_INLINE void merge_aarch64_cpuid_feature_2b(CpuFeatures::ARM& features, uint64_t reg_bits, uint32_t offset, Ext::Id f1, Ext::Id f2, Ext::Id f3) noexcept {
  uint32_t val = uint32_t((reg_bits >> offset) & 0x3u);
  features.add_if(f1 != Ext::kNone && val >= 1, f1);
  features.add_if(f2 != Ext::kNone && val >= 2, f2);
  features.add_if(f3 != Ext::kNone && val == 3, f3);
}
[[maybe_unused]]
static ASMJIT_INLINE void merge_aarch64_cpuid_feature_4b(CpuFeatures::ARM& features, uint64_t reg_bits, uint32_t offset,
  Ext::Id f1,
  Ext::Id f2 = Ext::kNone,
  Ext::Id f3 = Ext::kNone,
  Ext::Id f4 = Ext::kNone) noexcept {
  uint32_t val = uint32_t((reg_bits >> offset) & 0xFu);
  features.add_if(f1 != Ext::kNone && val >= 1, f1);
  features.add_if(f2 != Ext::kNone && val >= 2, f2);
  features.add_if(f3 != Ext::kNone && val >= 3, f3);
  features.add_if(f4 != Ext::kNone && val >= 4, f4);
}
[[maybe_unused]]
static ASMJIT_INLINE void merge_aarch64_cpuid_feature_4s(CpuFeatures::ARM& features, uint64_t reg_bits, uint32_t offset, uint32_t value, Ext::Id f1) noexcept {
  features.add_if(uint32_t((reg_bits >> offset) & 0xFu) == value, f1);
}
#define MERGE_FEATURE_NA(identifier, reg, offset, ...) merge_aarch64_cpuid_feature_na(cpu.features().arm(), reg, offset, __VA_ARGS__)
#define MERGE_FEATURE_1B(identifier, reg, offset, ...) merge_aarch64_cpuid_feature_1b(cpu.features().arm(), reg, offset, __VA_ARGS__)
#define MERGE_FEATURE_2B(identifier, reg, offset, ...) merge_aarch64_cpuid_feature_2b(cpu.features().arm(), reg, offset, __VA_ARGS__)
#define MERGE_FEATURE_4B(identifier, reg, offset, ...) merge_aarch64_cpuid_feature_4b(cpu.features().arm(), reg, offset, __VA_ARGS__)
#define MERGE_FEATURE_4S(identifier, reg, offset, ...) merge_aarch64_cpuid_feature_4s(cpu.features().arm(), reg, offset, __VA_ARGS__)
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64pfr0_aa64pfr1(CpuInfo& cpu, uint64_t fpr0, uint64_t fpr1) noexcept {
  MERGE_FEATURE_NA("FP bits [19:16]"          , fpr0, 16, Ext::kFP, Ext::kFP16);
  MERGE_FEATURE_NA("AdvSIMD bits [23:20]"     , fpr0, 20, Ext::kASIMD, Ext::kFP16);
  MERGE_FEATURE_4B("RAS bits [31:28]"         , fpr0, 28, Ext::kRAS, Ext::kRAS1_1, Ext::kRAS2);
  MERGE_FEATURE_4B("SVE bits [35:32]"         , fpr0, 32, Ext::kSVE);
  MERGE_FEATURE_4B("SEL2 bits [39:36]"        , fpr0, 36, Ext::kSEL2);
  MERGE_FEATURE_4B("MPAM bits [43:40]"        , fpr0, 40, Ext::kMPAM);
  MERGE_FEATURE_4B("AMU bits [47:44]"         , fpr0, 44, Ext::kAMU1, Ext::kAMU1_1);
  MERGE_FEATURE_4B("DIT bits [51:48]"         , fpr0, 48, Ext::kDIT);
  MERGE_FEATURE_4B("RME bits [55:52]"         , fpr0, 52, Ext::kRME);
  MERGE_FEATURE_4B("CSV2 bits [59:56]"        , fpr0, 56, Ext::kCSV2, Ext::kCSV2, Ext::kCSV2, Ext::kCSV2_3);
  MERGE_FEATURE_4B("CSV3 bits [63:60]"        , fpr0, 60, Ext::kCSV3);
  MERGE_FEATURE_4B("BT bits [3:0]"            , fpr1,  0, Ext::kBTI);
  MERGE_FEATURE_4B("SSBS bits [7:4]"          , fpr1,  4, Ext::kSSBS, Ext::kSSBS2);
  MERGE_FEATURE_4B("MTE bits [11:8]"          , fpr1,  8, Ext::kMTE, Ext::kMTE2, Ext::kMTE3);
  MERGE_FEATURE_4B("SME bits [27:24]"         , fpr1, 24, Ext::kSME, Ext::kSME2);
  MERGE_FEATURE_4B("RNDR_trap bits [31:28]"   , fpr1, 28, Ext::kRNG_TRAP);
  MERGE_FEATURE_4B("NMI bits [39:36]"         , fpr1, 36, Ext::kNMI);
  MERGE_FEATURE_4B("GCS bits [47:44]"         , fpr1, 44, Ext::kGCS);
  MERGE_FEATURE_4B("THE bits [51:48]"         , fpr1, 48, Ext::kTHE);
  if (cpu.features().arm().has_mte3())
    MERGE_FEATURE_4B("MTEX bits [55:52]"      , fpr1, 52, Ext::kMTE4);
  MERGE_FEATURE_4B("PFAR bits [63:60]"        , fpr1, 60, Ext::kPFAR);
  uint32_t ras_main = uint32_t((fpr0 >> 28) & 0xFu);
  uint32_t ras_frac = uint32_t((fpr1 >> 12) & 0xFu);
  if (ras_main == 1 && ras_frac == 1) {
    cpu.features().arm().add(Ext::kRAS1_1);
  }
  uint32_t mpam_main = uint32_t((fpr0 >> 40) & 0xFu);
  uint32_t mpam_frac = uint32_t((fpr1 >> 16) & 0xFu);
  if (mpam_main || mpam_frac) {
    cpu.features().arm().add(Ext::kMPAM);
  }
}
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64isar0_aa64isar1(CpuInfo& cpu, uint64_t isar0, uint64_t isar1) noexcept {
  MERGE_FEATURE_4B("AES bits [7:4]"           , isar0,  4, Ext::kAES, Ext::kPMULL);
  MERGE_FEATURE_4B("SHA1 bits [11:8]"         , isar0,  8, Ext::kSHA1);
  MERGE_FEATURE_4B("SHA2 bits [15:12]"        , isar0, 12, Ext::kSHA256, Ext::kSHA512);
  MERGE_FEATURE_4B("CRC32 bits [19:16]"       , isar0, 16, Ext::kCRC32);
  MERGE_FEATURE_4B("Atomic bits [23:20]"      , isar0, 20, Ext::kNone, Ext::kLSE, Ext::kLSE128);
  MERGE_FEATURE_4B("TME bits [27:24]"         , isar0, 24, Ext::kTME);
  MERGE_FEATURE_4B("RDM bits [31:28]"         , isar0, 28, Ext::kRDM);
  MERGE_FEATURE_4B("SHA3 bits [35:32]"        , isar0, 32, Ext::kSHA3);
  MERGE_FEATURE_4B("SM3 bits [39:36]"         , isar0, 36, Ext::kSM3);
  MERGE_FEATURE_4B("SM4 bits [43:40]"         , isar0, 40, Ext::kSM4);
  MERGE_FEATURE_4B("DP bits [47:44]"          , isar0, 44, Ext::kDOTPROD);
  MERGE_FEATURE_4B("FHM bits [51:48]"         , isar0, 48, Ext::kFHM);
  MERGE_FEATURE_4B("TS bits [55:52]"          , isar0, 52, Ext::kFLAGM, Ext::kFLAGM2);
  MERGE_FEATURE_4B("RNDR bits [63:60]"        , isar0, 60, Ext::kFLAGM, Ext::kRNG);
  MERGE_FEATURE_4B("DPB bits [3:0]"           , isar1,  0, Ext::kDPB, Ext::kDPB2);
  MERGE_FEATURE_4B("JSCVT bits [15:12]"       , isar1, 12, Ext::kJSCVT);
  MERGE_FEATURE_4B("FCMA bits [19:16]"        , isar1, 16, Ext::kFCMA);
  MERGE_FEATURE_4B("LRCPC bits [23:20]"       , isar1, 20, Ext::kLRCPC, Ext::kLRCPC2, Ext::kLRCPC3);
  MERGE_FEATURE_4B("FRINTTS bits [35:32]"     , isar1, 32, Ext::kFRINTTS);
  MERGE_FEATURE_4B("SB bits [39:36]"          , isar1, 36, Ext::kSB);
  MERGE_FEATURE_4B("SPECRES bits [43:40]"     , isar1, 40, Ext::kSPECRES, Ext::kSPECRES2);
  MERGE_FEATURE_4B("BF16 bits [47:44]"        , isar1, 44, Ext::kBF16, Ext::kEBF16);
  MERGE_FEATURE_4B("DGH bits [51:48]"         , isar1, 48, Ext::kDGH);
  MERGE_FEATURE_4B("I8MM bits [55:52]"        , isar1, 52, Ext::kI8MM);
  MERGE_FEATURE_4B("XS bits [59:56]"          , isar1, 56, Ext::kXS);
  MERGE_FEATURE_4B("LS64 bits [63:60]"        , isar1, 60, Ext::kLS64, Ext::kLS64_V, Ext::kLS64_ACCDATA);
}
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64isar2(CpuInfo& cpu, uint64_t isar2) noexcept {
  MERGE_FEATURE_4B("WFxT bits [3:0]"          , isar2,  0, Ext::kNone, Ext::kWFXT);
  MERGE_FEATURE_4B("RPRES bits [7:4]"         , isar2,  4, Ext::kRPRES);
  MERGE_FEATURE_4B("MOPS bits [19:16]"        , isar2, 16, Ext::kMOPS);
  MERGE_FEATURE_4B("BC bits [23:20]"          , isar2, 20, Ext::kHBC);
  MERGE_FEATURE_4B("PAC_frac bits [27:24]"    , isar2, 24, Ext::kCONSTPACFIELD);
  MERGE_FEATURE_4B("CLRBHB bits [31:28]"      , isar2, 28, Ext::kCLRBHB);
  MERGE_FEATURE_4B("SYSREG128 bits [35:32]"   , isar2, 32, Ext::kSYSREG128);
  MERGE_FEATURE_4B("SYSINSTR128 bits [39:36]" , isar2, 36, Ext::kSYSINSTR128);
  MERGE_FEATURE_4B("PRFMSLC bits [43:40]"     , isar2, 40, Ext::kPRFMSLC);
  MERGE_FEATURE_4B("RPRFM bits [51:48]"       , isar2, 48, Ext::kRPRFM);
  MERGE_FEATURE_4B("CSSC bits [55:52]"        , isar2, 52, Ext::kCSSC);
  MERGE_FEATURE_4B("LUT bits [59:56]"         , isar2, 56, Ext::kLUT);
}
#if 0
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64isar3(CpuInfo& cpu, uint64_t isar3) noexcept {
  MERGE_FEATURE_4B("CPA bits [3:0]"           , isar3,  0, Ext::kCPA, Ext::kCPA2);
  MERGE_FEATURE_4B("FAMINMAX bits [7:4]"      , isar3,  4, Ext::kFAMINMAX);
  MERGE_FEATURE_4B("TLBIW bits [11:8]"        , isar3,  8, Ext::kTLBIW);
  MERGE_FEATURE_4B("LSFE bits [19:16]"        , isar3, 16, Ext::kLSFE);
  MERGE_FEATURE_4B("OCCMO bits [23:20]"       , isar3, 20, Ext::kOCCMO);
  MERGE_FEATURE_4B("LSUI bits [27:24]"        , isar3, 24, Ext::kLSUI);
}
#endif
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64mmfr0(CpuInfo& cpu, uint64_t mmfr0) noexcept {
  MERGE_FEATURE_4B("FGT bits [59:56]"         , mmfr0, 56, Ext::kFGT, Ext::kFGT2);
  MERGE_FEATURE_4B("ECV bits [63:60]"         , mmfr0, 60, Ext::kECV);
}
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64mmfr1(CpuInfo& cpu, uint64_t mmfr1) noexcept {
  MERGE_FEATURE_4B("HAFDBS bits [3:0]"        , mmfr1,  0, Ext::kHAFDBS, Ext::kNone, Ext::kHAFT, Ext::kHDBSS);
  MERGE_FEATURE_4B("VMIDBits bits [7:4]"      , mmfr1,  4, Ext::kVMID16);
  MERGE_FEATURE_4B("VH bits [11:8]"           , mmfr1,  8, Ext::kVHE);
  MERGE_FEATURE_4B("HPDS bits [15:12]"        , mmfr1, 12, Ext::kHPDS, Ext::kHPDS2);
  MERGE_FEATURE_4B("LO bits [19:16]"          , mmfr1, 16, Ext::kLOR);
  MERGE_FEATURE_4B("PAN bits [23:20]"         , mmfr1, 20, Ext::kPAN, Ext::kPAN2, Ext::kPAN3);
  MERGE_FEATURE_4B("XNX bits [31:28]"         , mmfr1, 28, Ext::kXNX);
  MERGE_FEATURE_4B("HCX bits [43:40]"         , mmfr1, 40, Ext::kHCX);
  MERGE_FEATURE_4B("AFP bits [47:44]"         , mmfr1, 44, Ext::kAFP);
  MERGE_FEATURE_4B("CMOW bits [59:56]"        , mmfr1, 56, Ext::kCMOW);
  MERGE_FEATURE_4B("ECBHB bits [63:60]"       , mmfr1, 60, Ext::kECBHB);
}
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64mmfr2(CpuInfo& cpu, uint64_t mmfr2) noexcept {
  MERGE_FEATURE_4B("UAO bits [7:4]"           , mmfr2,  4, Ext::kUAO);
  MERGE_FEATURE_4B("VARange bits [19:16]"     , mmfr2, 16, Ext::kLVA, Ext::kLVA3);
  MERGE_FEATURE_4B("CCIDX bits [23:20]"       , mmfr2, 20, Ext::kCCIDX);
  MERGE_FEATURE_4B("NV bits [27:24]"          , mmfr2, 24, Ext::kNV, Ext::kNV2);
  MERGE_FEATURE_4B("AT bits [35:32]"          , mmfr2, 32, Ext::kLSE2);
}
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64zfr0(CpuInfo& cpu, uint64_t zfr0) noexcept {
  MERGE_FEATURE_4B("SVEver bits [3:0]"        , zfr0,  0, Ext::kSVE2, Ext::kSVE2_1, Ext::kSVE2_2);
  MERGE_FEATURE_4B("AES bits [7:4]"           , zfr0,  4, Ext::kSVE_AES, Ext::kSVE_PMULL128);
  MERGE_FEATURE_4B("EltPerm bits [15:12]"     , zfr0, 12, Ext::kSVE_ELTPERM);
  MERGE_FEATURE_4B("BitPerm bits [19:16]"     , zfr0, 16, Ext::kSVE_BITPERM);
  MERGE_FEATURE_4B("BF16 bits [23:20]"        , zfr0, 20, Ext::kSVE_BF16, Ext::kSVE_EBF16);
  MERGE_FEATURE_4B("B16B16 bits [27:24]"      , zfr0, 24, Ext::kSVE_B16B16);
  MERGE_FEATURE_4B("SHA3 bits [35:32]"        , zfr0, 32, Ext::kSVE_SHA3);
  MERGE_FEATURE_4B("SM4 bits [43:40]"         , zfr0, 40, Ext::kSVE_SM4);
  MERGE_FEATURE_4B("I8MM bits [47:44]"        , zfr0, 44, Ext::kSVE_I8MM);
  MERGE_FEATURE_4B("F32MM bits [55:52]"       , zfr0, 52, Ext::kSVE_F32MM);
  MERGE_FEATURE_4B("F64MM bits [59:56]"       , zfr0, 56, Ext::kSVE_F64MM);
}
[[maybe_unused]]
static inline void detect_aarch64_features_via_cpuid_aa64smfr0(CpuInfo& cpu, uint64_t smfr0) noexcept {
  MERGE_FEATURE_1B("SMOP4 bit [0]"            , smfr0,  0, Ext::kSME_MOP4);
  MERGE_FEATURE_1B("STMOP bit [16]"           , smfr0, 16, Ext::kSME_TMOP);
  MERGE_FEATURE_1B("SFEXPA bit [23]"          , smfr0, 23, Ext::kSSVE_FEXPA);
  MERGE_FEATURE_1B("AES bit [24]"             , smfr0, 24, Ext::kSSVE_AES);
  MERGE_FEATURE_1B("SBitPerm bit [25]"        , smfr0, 25, Ext::kSSVE_BITPERM);
  MERGE_FEATURE_1B("SF8DP2 bit [28]"          , smfr0, 28, Ext::kSSVE_FP8DOT2);
  MERGE_FEATURE_1B("SF8DP4 bit [29]"          , smfr0, 29, Ext::kSSVE_FP8DOT4);
  MERGE_FEATURE_1B("SF8FMA bit [30]"          , smfr0, 30, Ext::kSSVE_FP8FMA);
  MERGE_FEATURE_1B("F32F32 bit [32]"          , smfr0, 32, Ext::kSME_F32F32);
  MERGE_FEATURE_1B("BI32I32 bit [33]"         , smfr0, 33, Ext::kSME_BI32I32);
  MERGE_FEATURE_1B("B16F32 bit [34]"          , smfr0, 34, Ext::kSME_B16F32);
  MERGE_FEATURE_1B("F16F32 bit [35]"          , smfr0, 35, Ext::kSME_F16F32);
  MERGE_FEATURE_4S("I8I32 bits [39:36]"       , smfr0, 36, 0xF, Ext::kSME_I8I32);
  MERGE_FEATURE_1B("F8F32 bit [40]"           , smfr0, 40, Ext::kSME_F8F32);
  MERGE_FEATURE_1B("F8F16 bit [41]"           , smfr0, 41, Ext::kSME_F8F16);
  MERGE_FEATURE_1B("F16F16 bit [42]"          , smfr0, 42, Ext::kSME_F16F16);
  MERGE_FEATURE_1B("B16B16 bit [43]"          , smfr0, 43, Ext::kSME_B16B16);
  MERGE_FEATURE_4S("I16I32 bits [47:44]"      , smfr0, 44, 0x5, Ext::kSME_I16I32);
  MERGE_FEATURE_1B("F64F64 bit [48]"          , smfr0, 48, Ext::kSME_F64F64);
  MERGE_FEATURE_4S("I16I64 bits [55:52]"      , smfr0, 52, 0xF, Ext::kSME_I16I64);
  MERGE_FEATURE_4B("SMEver bits [59:56]"      , smfr0, 56, Ext::kSME2, Ext::kSME2_1, Ext::kSME2_2);
  MERGE_FEATURE_1B("LUTv2 bit [60]"           , smfr0, 60, Ext::kSME_LUTv2);
  MERGE_FEATURE_1B("FA64 bit [63]"            , smfr0, 63, Ext::kSME_FA64);
}
#undef MERGE_FEATURE_4S
#undef MERGE_FEATURE_4B
#undef MERGE_FEATURE_2B
#undef MERGE_FEATURE_1B
#undef MERGE_FEATURE_NA
enum class AppleFamilyId : uint32_t {
  kSWIFT              = 0x1E2D6381u,
  kCYCLONE            = 0x37A09642u,
  kTYPHOON            = 0x2C91A47Eu,
  kTWISTER            = 0x92FB37C8u,
  kHURRICANE          = 0x67CEEE93u,
  kMONSOON_MISTRAL    = 0xE81E7EF6u,
  kVORTEX_TEMPEST     = 0x07D34B9Fu,
  kLIGHTNING_THUNDER  = 0x462504D2u,
  kFIRESTORM_ICESTORM = 0x1B588BB3u,
  kAVALANCHE_BLIZZARD = 0XDA33D83Du,
  kEVEREST_SAWTOOTH   = 0X8765EDEAu,
  kIBIZA              = 0xFA33415Eu,
  kPALMA              = 0x72015832u,
  kLOBOS              = 0x5F4DEA93u,
  kCOLL               = 0x2876F5B5u,
  kDONAN              = 0x6F5129ACu,
  kBRAVA              = 0x17D5B93Au,
  kTUPAI              = 0x204526D0u,
  kTAHITI             = 0x75D4ACB9u
};
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE bool detect_aarch64_features_via_apple_family_id(CpuInfo& cpu) noexcept {
  using Id = AppleFamilyId;
  CpuFeatures::ARM& features = cpu.features().arm();
  switch (cpu.family_id()) {
    case uint32_t(Id::kCYCLONE):
    case uint32_t(Id::kTYPHOON):
    case uint32_t(Id::kTWISTER):
      populate_armv8a_features(features, 0);
      features.add(
        Ext::kPMU,
        Ext::kAES, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256
      );
      return true;
    case uint32_t(Id::kHURRICANE):
      populate_armv8a_features(features, 0);
      features.add(
        Ext::kLOR, Ext::kPAN, Ext::kPMU, Ext::kVHE,
        Ext::kAES, Ext::kCRC32, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256, Ext::kRDM
      );
      return true;
    case uint32_t(Id::kMONSOON_MISTRAL):
      populate_armv8a_features(features, 2);
      features.add(
        Ext::kPMU,
        Ext::kAES, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256,
        Ext::kFP16, Ext::kFP16CONV
      );
      return true;
    case uint32_t(Id::kVORTEX_TEMPEST):
      populate_armv8a_features(features, 3);
      features.add(
        Ext::kPMU,
        Ext::kAES, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256,
        Ext::kFP16, Ext::kFP16CONV
      );
      return true;
    case uint32_t(Id::kLIGHTNING_THUNDER):
      populate_armv8a_features(features, 4);
      features.add(
        Ext::kPMU,
        Ext::kFP16, Ext::kFP16CONV, Ext::kFHM,
        Ext::kAES, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256, Ext::kSHA3, Ext::kSHA512
      );
      return true;
    case uint32_t(Id::kFIRESTORM_ICESTORM):
      populate_armv8a_features(features, 4);
      features.add(
        Ext::kCSV2, Ext::kCSV3, Ext::kDPB2, Ext::kECV, Ext::kFLAGM2, Ext::kPMU, Ext::kSB, Ext::kSSBS,
        Ext::kFP16, Ext::kFP16CONV, Ext::kFHM,
        Ext::kFRINTTS,
        Ext::kAES, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256, Ext::kSHA3, Ext::kSHA512);
      return true;
    case uint32_t(Id::kAVALANCHE_BLIZZARD):
      populate_armv8a_features(features, 6);
      features.add(
        Ext::kPMU,
        Ext::kFP16, Ext::kFP16CONV, Ext::kFHM,
        Ext::kAES, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256, Ext::kSHA3, Ext::kSHA512);
      return true;
    case uint32_t(Id::kEVEREST_SAWTOOTH):
    case uint32_t(Id::kIBIZA):
    case uint32_t(Id::kPALMA):
    case uint32_t(Id::kLOBOS):
      populate_armv8a_features(features, 6);
      features.add(
        Ext::kHCX, Ext::kPMU,
        Ext::kFP16, Ext::kFP16CONV, Ext::kFHM,
        Ext::kAES, Ext::kPMULL, Ext::kSHA1, Ext::kSHA256, Ext::kSHA3, Ext::kSHA512
      );
      return true;
    case uint32_t(Id::kCOLL):
    case uint32_t(Id::kDONAN):
    case uint32_t(Id::kBRAVA):
      populate_armv8a_features(features, 7);
      features.add(
        Ext::kPMU,
        Ext::kFP16, Ext::kFP16CONV, Ext::kFHM,
        Ext::kAES, Ext::kSHA1, Ext::kSHA3, Ext::kSHA256, Ext::kSHA512,
        Ext::kSME, Ext::kSME2, Ext::kSME_F64F64, Ext::kSME_I16I64);
      return true;
    default:
      return false;
  }
}
#if ASMJIT_ARCH_ARM == 32
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void detect_aarch32_features_via_compiler_flags(CpuInfo& cpu) noexcept {
  Support::maybe_unused(cpu);
#if defined(__ARM_ARCH_7A__)
  cpu.add_feature(CpuFeatures::ARM::kARMv7);
#endif
#if defined(__ARM_ARCH_8A__)
  cpu.add_feature(CpuFeatures::ARM::kARMv8a);
#endif
#if defined(__TARGET_ARCH_THUMB)
  cpu.add_feature(CpuFeatures::ARM::kTHUMB);
#if __TARGET_ARCH_THUMB >= 4
  cpu.add_feature(CpuFeatures::ARM::kTHUMBv2);
#endif
#endif
#if defined(__ARM_FEATURE_FMA)
  cpu.add_feature(Ext::kFP);
#endif
#if defined(__ARM_NEON)
  cpu.add_feature(Ext::kASIMD);
#endif
#if defined(__ARM_FEATURE_IDIV) && defined(__TARGET_ARCH_THUMB)
  cpu.add_feature(Ext::kIDIVT);
#endif
#if defined(__ARM_FEATURE_IDIV) && !defined(__TARGET_ARCH_THUMB)
  cpu.add_feature(Ext::kIDIVA);
#endif
}
#endif
#if ASMJIT_ARCH_ARM == 64
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void detect_aarch64_features_via_compiler_flags(CpuInfo& cpu) noexcept {
  Support::maybe_unused(cpu);
#if defined(__ARM_ARCH_9_5A__)
  populate_armv9a_features(cpu.features().arm(), 5);
#elif defined(__ARM_ARCH_9_4A__)
  populate_armv9a_features(cpu.features().arm(), 4);
#elif defined(__ARM_ARCH_9_3A__)
  populate_armv9a_features(cpu.features().arm(), 3);
#elif defined(__ARM_ARCH_9_2A__)
  populate_armv9a_features(cpu.features().arm(), 2);
#elif defined(__ARM_ARCH_9_1A__)
  populate_armv9a_features(cpu.features().arm(), 1);
#elif defined(__ARM_ARCH_9A__)
  populate_armv9a_features(cpu.features().arm(), 0);
#elif defined(__ARM_ARCH_8_9A__)
  populate_armv8a_features(cpu.features().arm(), 9);
#elif defined(__ARM_ARCH_8_8A__)
  populate_armv8a_features(cpu.features().arm(), 8);
#elif defined(__ARM_ARCH_8_7A__)
  populate_armv8a_features(cpu.features().arm(), 7);
#elif defined(__ARM_ARCH_8_6A__)
  populate_armv8a_features(cpu.features().arm(), 6);
#elif defined(__ARM_ARCH_8_5A__)
  populate_armv8a_features(cpu.features().arm(), 5);
#elif defined(__ARM_ARCH_8_4A__)
  populate_armv8a_features(cpu.features().arm(), 4);
#elif defined(__ARM_ARCH_8_3A__)
  populate_armv8a_features(cpu.features().arm(), 3);
#elif defined(__ARM_ARCH_8_2A__)
  populate_armv8a_features(cpu.features().arm(), 2);
#elif defined(__ARM_ARCH_8_1A__)
  populate_armv8a_features(cpu.features().arm(), 1);
#else
  populate_armv8a_features(cpu.features().arm(), 0);
#endif
#if defined(__ARM_FEATURE_AES)
  cpu.add_feature(Ext::kAES);
#endif
#if defined(__ARM_FEATURE_BF16_SCALAR_ARITHMETIC) && defined(__ARM_FEATURE_BF16_VECTOR_ARITHMETIC)
  cpu.add_feature(Ext::kBF16);
#endif
#if defined(__ARM_FEATURE_CRC32)
  cpu.add_feature(Ext::kCRC32);
#endif
#if defined(__ARM_FEATURE_CRYPTO)
  cpu.add_feature(Ext::kAES, Ext::kSHA1, Ext::kSHA256);
#endif
#if defined(__ARM_FEATURE_DOTPROD)
  cpu.add_feature(Ext::kDOTPROD);
#endif
#if defined(__ARM_FEATURE_FP16FML) || defined(__ARM_FEATURE_FP16_FML)
  cpu.add_feature(Ext::kFHM);
#endif
#if defined(__ARM_FEATURE_FP16_SCALAR_ARITHMETIC)
  cpu.add_feature(Ext::kFP16);
#endif
#if defined(__ARM_FEATURE_FRINT)
  cpu.add_feature(Ext::kFRINTTS);
#endif
#if defined(__ARM_FEATURE_JCVT)
  cpu.add_feature(Ext::kJSCVT);
#endif
#if defined(__ARM_FEATURE_MATMUL_INT8)
  cpu.add_feature(Ext::kI8MM);
#endif
#if defined(__ARM_FEATURE_ATOMICS)
  cpu.add_feature(Ext::kLSE);
#endif
#if defined(__ARM_FEATURE_MEMORY_TAGGING)
  cpu.add_feature(Ext::kMTE);
#endif
#if defined(__ARM_FEATURE_QRDMX)
  cpu.add_feature(Ext::kRDM);
#endif
#if defined(__ARM_FEATURE_RNG)
  cpu.add_feature(Ext::kRNG);
#endif
#if defined(__ARM_FEATURE_SHA2)
  cpu.add_feature(Ext::kSHA256);
#endif
#if defined(__ARM_FEATURE_SHA3)
  cpu.add_feature(Ext::kSHA3);
#endif
#if defined(__ARM_FEATURE_SHA512)
  cpu.add_feature(Ext::kSHA512);
#endif
#if defined(__ARM_FEATURE_SM3)
  cpu.add_feature(Ext::kSM3);
#endif
#if defined(__ARM_FEATURE_SM4)
  cpu.add_feature(Ext::kSM4);
#endif
#if defined(__ARM_FEATURE_SVE) || defined(__ARM_FEATURE_SVE_VECTOR_OPERATORS)
  cpu.add_feature(Ext::kSVE);
#endif
#if defined(__ARM_FEATURE_SVE_MATMUL_INT8)
  cpu.add_feature(Ext::kSVE_I8MM);
#endif
#if defined(__ARM_FEATURE_SVE_MATMUL_FP32)
  cpu.add_feature(Ext::kSVE_F32MM);
#endif
#if defined(__ARM_FEATURE_SVE_MATMUL_FP64)
  cpu.add_feature(Ext::kSVE_F64MM);
#endif
#if defined(__ARM_FEATURE_SVE2)
  cpu.add_feature(Ext::kSVE2);
#endif
#if defined(__ARM_FEATURE_SVE2_AES)
  cpu.add_feature(Ext::kSVE_AES);
#endif
#if defined(__ARM_FEATURE_SVE2_BITPERM)
  cpu.add_feature(Ext::kSVE_BITPERM);
#endif
#if defined(__ARM_FEATURE_SVE2_SHA3)
  cpu.add_feature(Ext::kSVE_SHA3);
#endif
#if defined(__ARM_FEATURE_SVE2_SM4)
  cpu.add_feature(Ext::kSVE_SM4);
#endif
#if defined(__ARM_FEATURE_TME)
  cpu.add_feature(Ext::kTME);
#endif
}
#endif
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void detect_arm_features_via_compiler_flags(CpuInfo& cpu) noexcept {
#if ASMJIT_ARCH_ARM == 32
  detect_aarch32_features_via_compiler_flags(cpu);
#else
  detect_aarch64_features_via_compiler_flags(cpu);
#endif
}
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void post_process_aarch32_features(CpuFeatures::ARM& features) noexcept {
  Support::maybe_unused(features);
}
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void post_process_aarch64_features(CpuFeatures::ARM& features) noexcept {
  if (features.has_fp16()) {
    features.add(Ext::kFP16CONV);
  }
  if (features.has_mte3()) {
    features.add(Ext::kMTE2);
  }
  if (features.has_mte2()) {
    features.add(Ext::kMTE);
  }
  if (features.has_ssbs2()) {
    features.add(Ext::kSSBS);
  }
}
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void post_process_arm_cpu_info(CpuInfo& cpu) noexcept {
#if ASMJIT_ARCH_ARM == 32
  post_process_aarch32_features(cpu.features().arm());
#else
  post_process_aarch64_features(cpu.features().arm());
#endif
}
#if defined(ASMJIT_ARM_DETECT_VIA_CPUID)
#define ASMJIT_AARCH64_DEFINE_CPUID_READ_FN(func, reg_id)  \
[[maybe_unused]]                                           \
static inline uint64_t func() noexcept {                   \
  uint64_t output;                                         \
  __asm__ __volatile__("mrs %0, " #reg_id : "=r"(output)); \
  return output;                                           \
}
ASMJIT_AARCH64_DEFINE_CPUID_READ_FN(aarch64_read_pfr0, ID_AA64PFR0_EL1)
ASMJIT_AARCH64_DEFINE_CPUID_READ_FN(aarch64_read_pfr1, ID_AA64PFR1_EL1)
ASMJIT_AARCH64_DEFINE_CPUID_READ_FN(aarch64_read_isar0, ID_AA64ISAR0_EL1)
ASMJIT_AARCH64_DEFINE_CPUID_READ_FN(aarch64_read_isar1, ID_AA64ISAR1_EL1)
ASMJIT_AARCH64_DEFINE_CPUID_READ_FN(aarch64_read_isar2, S3_0_C0_C6_2)
ASMJIT_AARCH64_DEFINE_CPUID_READ_FN(aarch64_read_zfr0, S3_0_C0_C4_4)
#undef ASMJIT_AARCH64_DEFINE_CPUID_READ_FN
[[maybe_unused]]
static ASMJIT_FAVOR_SIZE void detect_aarch64_features_via_cpuid(CpuInfo& cpu) noexcept {
  populate_base_arm_features(cpu);
  detect_aarch64_features_via_cpuid_aa64pfr0_aa64pfr1(cpu, aarch64_read_pfr0(), aarch64_read_pfr1());
  detect_aarch64_features_via_cpuid_aa64isar0_aa64isar1(cpu, aarch64_read_isar0(), aarch64_read_isar1());
#if defined(__linux__)
  UNameKernelVersion kVer = get_kernel_version_via_uname();
  if (kVer.at_least(4, 20)) {
    detect_aarch64_features_via_cpuid_aa64isar2(cpu, aarch64_read_isar2());
  }
  if (kVer.at_least(5, 11) && cpu.features().arm().has_any(Ext::kSVE, Ext::kSME)) {
    detect_aarch64_features_via_cpuid_aa64zfr0(cpu, aarch64_read_zfr0());
  }
#endif
}
#endif
#if defined(_WIN32)
struct WinPFPMapping {
  uint8_t feature_id;
  uint8_t pfp_feature_id;
};
static ASMJIT_FAVOR_SIZE void detect_pfp_features(CpuInfo& cpu, const WinPFPMapping* mapping, size_t size) noexcept {
  for (size_t i = 0; i < size; i++) {
    if (::IsProcessorFeaturePresent(mapping[i].pfp_feature_id)) {
      cpu.add_feature(mapping[i].feature_id);
    }
  }
}
static ASMJIT_FAVOR_SIZE void detect_arm_cpu(CpuInfo& cpu) noexcept {
  cpu._was_detected = true;
  populate_base_arm_features(cpu);
  CpuFeatures::ARM& features = cpu.features().arm();
#if ASMJIT_ARCH_ARM == 32
  features.add(Ext::kTHUMB);
  features.add(Ext::kTHUMBv2);
  features.add(Ext::kARMv6);
  features.add(Ext::kARMv7);
  features.add(Ext::kEDSP);
#endif
  features.add(Ext::kFP);
  features.add(Ext::kASIMD);
  static const WinPFPMapping mapping[] = {
#if ASMJIT_ARCH_ARM == 32
    { uint8_t(Ext::kVFP_D32)  , 18 },
    { uint8_t(Ext::kIDIVT)    , 24 },
    { uint8_t(Ext::kFMAC)     , 27 },
    { uint8_t(Ext::kARMv8a)   , 29 },
#endif
    { uint8_t(Ext::kAES)      , 30 },
    { uint8_t(Ext::kCRC32)    , 31 },
    { uint8_t(Ext::kLSE)      , 34 },
    { uint8_t(Ext::kDOTPROD)  , 43 },
    { uint8_t(Ext::kJSCVT)    , 44 },
    { uint8_t(Ext::kLRCPC)    , 45 }
  };
  detect_pfp_features(cpu, mapping, ASMJIT_ARRAY_SIZE(mapping));
  if (features.has_armv8a()) {
    populate_armv8a_features(cpu.features().arm(), 0);
  }
  if (features.has_aes()) {
    features.add(Ext::kPMULL, Ext::kSHA1, Ext::kSHA256);
  }
  post_process_arm_cpu_info(cpu);
}
#elif defined(ASMJIT_ARM_DETECT_VIA_HWCAPS)
#ifndef AT_HWCAP
  #define AT_HWCAP 16
#endif
#ifndef AT_HWCAP2
  #define AT_HWCAP2 26
#endif
#if defined(__linux__)
static void get_aux_values(unsigned long* vals, const unsigned long* tags, size_t count) noexcept {
  for (size_t i = 0; i < count; i++) {
    vals[i] = getauxval(tags[i]);
  }
}
#elif defined(__FreeBSD__)
static void get_aux_values(unsigned long* vals, const unsigned long* tags, size_t count) noexcept {
  for (size_t i = 0; i < count; i++) {
    unsigned long result = 0;
    if (elf_aux_info(int(tags[i]), &result, int(sizeof(unsigned long))) != 0)
      result = 0;
    vals[i] = result;
  }
}
#else
#error "[asmjit] get_aux_values() - Unsupported OS."
#endif
static const unsigned long hw_cap_tags_table[2] = { AT_HWCAP, AT_HWCAP2 };
struct HWCapMapping32 { uint8_t feature_id[32]; };
struct HWCapMapping64 { uint8_t feature_id[64]; };
template<typename Mask, typename Map>
static ASMJIT_FAVOR_SIZE void merge_hw_caps(CpuInfo& cpu, const Mask& mask, const Map& map) noexcept {
  static_assert(sizeof(Mask) * 8u == sizeof(Map));
  Support::BitWordIterator<Mask> it(mask);
  while (it.has_next()) {
    uint32_t feature_id = map.feature_id[it.next()];
    cpu.features().add(feature_id);
  }
  cpu.features().remove(0xFFu);
}
#if ASMJIT_ARCH_ARM == 32
static constexpr HWCapMapping32 hw_cap1_mapping_table = {{
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(Ext::kEDSP)         ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(Ext::kASIMD)        ,
  uint8_t(Ext::kFP)           ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(Ext::kFMAC)         ,
  uint8_t(Ext::kIDIVA)        ,
  uint8_t(Ext::kIDIVT)        ,
  uint8_t(Ext::kVFP_D32)      ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(Ext::kFP16CONV)     ,
  uint8_t(Ext::kFP16)         ,
  uint8_t(Ext::kDOTPROD)      ,
  uint8_t(Ext::kFHM)          ,
  uint8_t(Ext::kBF16)         ,
  uint8_t(Ext::kI8MM)         ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)
}};
static constexpr HWCapMapping32 hw_cap2_mapping_table = {{
  uint8_t(Ext::kAES)          ,
  uint8_t(Ext::kPMULL)        ,
  uint8_t(Ext::kSHA1)         ,
  uint8_t(Ext::kSHA256)       ,
  uint8_t(Ext::kCRC32)        ,
  uint8_t(Ext::kSB)           ,
  uint8_t(Ext::kSSBS)         ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)
}};
static ASMJIT_FAVOR_SIZE void detect_arm_cpu(CpuInfo& cpu) noexcept {
  cpu._was_detected = true;
  populate_base_arm_features(cpu);
  unsigned long hw_cap_masks[2] {};
  get_aux_values(hw_cap_masks, hw_cap_tags_table, 2u);
  merge_hw_caps(cpu, hw_cap_masks[0], hw_cap1_mapping_table);
  merge_hw_caps(cpu, hw_cap_masks[1], hw_cap2_mapping_table);
  CpuFeatures::ARM& features = cpu.features().arm();
  if (features.has_fp() || features.has_asimd()) {
    features.add(CpuFeatures::ARM::kARMv7);
  }
  if (features.has_aes() || features.has_crc32() || features.has_pmull() || features.has_sha1() || features.has_sha256()) {
    features.add(CpuFeatures::ARM::kARMv8a);
  }
  post_process_arm_cpu_info(cpu);
}
#else
static constexpr HWCapMapping64 hw_cap1_mapping_table = {{
  uint8_t(Ext::kFP)           ,
  uint8_t(Ext::kASIMD)        ,
  uint8_t(0xFF)               ,
  uint8_t(Ext::kAES)          ,
  uint8_t(Ext::kPMULL)        ,
  uint8_t(Ext::kSHA1)         ,
  uint8_t(Ext::kSHA256)       ,
  uint8_t(Ext::kCRC32)        ,
  uint8_t(Ext::kLSE)          ,
  uint8_t(Ext::kFP16CONV)     ,
  uint8_t(Ext::kFP16)         ,
  uint8_t(Ext::kCPUID)        ,
  uint8_t(Ext::kRDM)          ,
  uint8_t(Ext::kJSCVT)        ,
  uint8_t(Ext::kFCMA)         ,
  uint8_t(Ext::kLRCPC)        ,
  uint8_t(Ext::kDPB)          ,
  uint8_t(Ext::kSHA3)         ,
  uint8_t(Ext::kSM3)          ,
  uint8_t(Ext::kSM4)          ,
  uint8_t(Ext::kDOTPROD)      ,
  uint8_t(Ext::kSHA512)       ,
  uint8_t(Ext::kSVE)          ,
  uint8_t(Ext::kFHM)          ,
  uint8_t(Ext::kDIT)          ,
  uint8_t(Ext::kLSE2)         ,
  uint8_t(Ext::kLRCPC2)       ,
  uint8_t(Ext::kFLAGM)        ,
  uint8_t(Ext::kSSBS)         ,
  uint8_t(Ext::kSB)           ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(Ext::kGCS)          ,
  uint8_t(Ext::kCMPBR)        ,
  uint8_t(Ext::kFPRCVT)       ,
  uint8_t(Ext::kF8F32MM)      ,
  uint8_t(Ext::kF8F16MM)      ,
  uint8_t(Ext::kSVE_F16MM)    ,
  uint8_t(Ext::kSVE_ELTPERM)  ,
  uint8_t(Ext::kSVE_AES2)     ,
  uint8_t(Ext::kSVE_BFSCALE)  ,
  uint8_t(Ext::kSVE2_2)       ,
  uint8_t(Ext::kSME2_2)       ,
  uint8_t(Ext::kSSVE_BITPERM) ,
  uint8_t(Ext::kSME_AES)      ,
  uint8_t(Ext::kSSVE_FEXPA)   ,
  uint8_t(Ext::kSME_TMOP)     ,
  uint8_t(Ext::kSME_MOP4)     ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)               ,
  uint8_t(0xFF)
}};
static constexpr HWCapMapping64 hw_cap2_mapping_table = {{
  uint8_t(Ext::kDPB2)         ,
  uint8_t(Ext::kSVE2)         ,
  uint8_t(Ext::kSVE_AES)      ,
  uint8_t(Ext::kSVE_PMULL128) ,
  uint8_t(Ext::kSVE_BITPERM)  ,
  uint8_t(Ext::kSVE_SHA3)     ,
  uint8_t(Ext::kSVE_SM4)      ,
  uint8_t(Ext::kFLAGM2)       ,
  uint8_t(Ext::kFRINTTS)      ,
  uint8_t(Ext::kSVE_I8MM)     ,
  uint8_t(Ext::kSVE_F32MM)    ,
  uint8_t(Ext::kSVE_F64MM)    ,
  uint8_t(Ext::kSVE_BF16)     ,
  uint8_t(Ext::kI8MM)         ,
  uint8_t(Ext::kBF16)         ,
  uint8_t(Ext::kDGH)          ,
  uint8_t(Ext::kRNG)          ,
  uint8_t(Ext::kBTI)          ,
  uint8_t(Ext::kMTE)          ,
  uint8_t(Ext::kECV)          ,
  uint8_t(Ext::kAFP)          ,
  uint8_t(Ext::kRPRES)        ,
  uint8_t(Ext::kMTE3)         ,
  uint8_t(Ext::kSME)          ,
  uint8_t(Ext::kSME_I16I64)   ,
  uint8_t(Ext::kSME_F64F64)   ,
  uint8_t(Ext::kSME_I8I32)    ,
  uint8_t(Ext::kSME_F16F32)   ,
  uint8_t(Ext::kSME_B16F32)   ,
  uint8_t(Ext::kSME_F32F32)   ,
  uint8_t(Ext::kSME_FA64)     ,
  uint8_t(Ext::kWFXT)         ,
  uint8_t(Ext::kEBF16)        ,
  uint8_t(Ext::kSVE_EBF16)    ,
  uint8_t(Ext::kCSSC)         ,
  uint8_t(Ext::kRPRFM)        ,
  uint8_t(Ext::kSVE2_1)       ,
  uint8_t(Ext::kSME2)         ,
  uint8_t(Ext::kSME2_1)       ,
  uint8_t(Ext::kSME_I16I32)   ,
  uint8_t(Ext::kSME_BI32I32)  ,
  uint8_t(Ext::kSME_B16B16)   ,
  uint8_t(Ext::kSME_F16F16)   ,
  uint8_t(Ext::kMOPS)         ,
  uint8_t(Ext::kHBC)          ,
  uint8_t(Ext::kSVE_B16B16)   ,
  uint8_t(Ext::kLRCPC3)       ,
  uint8_t(Ext::kLSE128)       ,
  uint8_t(Ext::kFPMR)         ,
  uint8_t(Ext::kLUT)          ,
  uint8_t(Ext::kFAMINMAX)     ,
  uint8_t(Ext::kFP8)          ,
  uint8_t(Ext::kFP8FMA)       ,
  uint8_t(Ext::kFP8DOT4)      ,
  uint8_t(Ext::kFP8DOT2)      ,
  uint8_t(Ext::kF8E4M3)       ,
  uint8_t(Ext::kF8E5M2)       ,
  uint8_t(Ext::kSME_LUTv2)    ,
  uint8_t(Ext::kSME_F8F16)    ,
  uint8_t(Ext::kSME_F8F32)    ,
  uint8_t(Ext::kSSVE_FP8FMA)  ,
  uint8_t(Ext::kSSVE_FP8DOT4) ,
  uint8_t(Ext::kSSVE_FP8DOT2) ,
  uint8_t(0xFF)
}};
static ASMJIT_FAVOR_SIZE void detect_arm_cpu(CpuInfo& cpu) noexcept {
  cpu._was_detected = true;
  populate_base_arm_features(cpu);
  unsigned long hw_cap_masks[2] {};
  get_aux_values(hw_cap_masks, hw_cap_tags_table, 2u);
  merge_hw_caps(cpu, hw_cap_masks[0], hw_cap1_mapping_table);
  merge_hw_caps(cpu, hw_cap_masks[1], hw_cap2_mapping_table);
#if defined(ASMJIT_ARM_DETECT_VIA_CPUID)
  if (cpu.features().arm().has_cpuid()) {
    detect_aarch64_features_via_cpuid(cpu);
    return;
  }
#endif
  post_process_arm_cpu_info(cpu);
}
#endif
#elif defined(__NetBSD__) && ASMJIT_ARCH_ARM >= 64
struct NetBSDAArch64Regs {
  enum ID : uint32_t {
    k64_MIDR      = 0,
    k64_REVIDR    = 8,
    k64_MPIDR     = 16,
    k64_AA64DFR0  = 24,
    k64_AA64DFR1  = 32,
    k64_AA64ISAR0 = 40,
    k64_AA64ISAR1 = 48,
    k64_AA64MMFR0 = 56,
    k64_AA64MMFR1 = 64,
    k64_AA64MMFR2 = 72,
    k64_AA64PFR0  = 80,
    k64_AA64PFR1  = 88,
    k64_AA64ZFR0  = 96,
    k32_MVFR0     = 104,
    k32_MVFR1     = 108,
    k32_MVFR2     = 112,
    k32_PAD       = 116,
    k64_CLIDR     = 120,
    k64_CTR       = 128
  };
  enum Limits : uint32_t {
    kBufferSize   = 136
  };
  uint64_t data[kBufferSize / 8u];
  ASMJIT_INLINE_NODEBUG uint64_t r64(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index % 8u == 0u);
    return data[index / 8u];
  }
  ASMJIT_INLINE_NODEBUG uint32_t r32(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index % 4u == 0u);
    uint32_t shift = (index % 8) * 8;
    return uint32_t((r64(index) >> shift) & 0xFFFFFFFFu);
  }
};
static ASMJIT_FAVOR_SIZE void detect_arm_cpu(CpuInfo& cpu) noexcept {
  using Regs = NetBSDAArch64Regs;
  populate_base_arm_features(cpu);
  Regs regs {};
  size_t len = sizeof(regs);
  const char sysctl_cpu_path[] = "machdep.cpu0.cpu_id";
  if (sysctlbyname(sysctl_cpu_path, &regs, &len, nullptr, 0) == 0) {
    detect_aarch64_features_via_cpuid_aa64pfr0_aa64pfr1(cpu, regs.r64(Regs::k64_AA64PFR0), regs.r64(Regs::k64_AA64PFR1));
    detect_aarch64_features_via_cpuid_aa64isar0_aa64isar1(cpu, regs.r64(Regs::k64_AA64ISAR0), regs.r64(Regs::k64_AA64ISAR1));
    detect_aarch64_features_via_cpuid_aa64mmfr0(cpu, regs.r64(Regs::k64_AA64MMFR0));
    detect_aarch64_features_via_cpuid_aa64mmfr1(cpu, regs.r64(Regs::k64_AA64MMFR1));
    detect_aarch64_features_via_cpuid_aa64mmfr2(cpu, regs.r64(Regs::k64_AA64MMFR2));
    if (cpu.features().arm().has_any(Ext::kSVE, Ext::kSME)) {
      detect_aarch64_features_via_cpuid_aa64zfr0(cpu, regs.r64(Regs::k64_AA64ZFR0));
    }
  }
  post_process_arm_cpu_info(cpu);
}
#elif defined(__OpenBSD__) && ASMJIT_ARCH_ARM >= 64
enum class OpenBSDAArch64CPUID {
  kAA64ISAR0 = 2,
  kAA64ISAR1 = 3,
  kAA64ISAR2 = 4,
  kAA64MMFR0 = 5,
  kAA64MMFR1 = 6,
  kAA64MMFR2 = 7,
  kAA64PFR0 = 8,
  kAA64PFR1 = 9,
  kAA64SMFR0 = 10,
  kAA64ZFR0 = 11
};
static uint64_t openbsd_read_aarch64_cpuid(OpenBSDAArch64CPUID id) noexcept {
  uint64_t bits = 0;
  size_t size = sizeof(bits);
  int name[2] = { CTL_MACHDEP, int(id) };
  return (sysctl(name, 2, &bits, &size, NULL, 0) < 0) ? uint64_t(0) : bits;
}
static ASMJIT_FAVOR_SIZE void detect_arm_cpu(CpuInfo& cpu) noexcept {
  using ID = OpenBSDAArch64CPUID;
  populate_base_arm_features(cpu);
  detect_aarch64_features_via_cpuid_aa64pfr0_aa64pfr1(cpu, openbsd_read_aarch64_cpuid(ID::kAA64PFR0), openbsd_read_aarch64_cpuid(ID::kAA64PFR1));
  detect_aarch64_features_via_cpuid_aa64isar0_aa64isar1(cpu, openbsd_read_aarch64_cpuid(ID::kAA64ISAR0), openbsd_read_aarch64_cpuid(ID::kAA64ISAR1));
  detect_aarch64_features_via_cpuid_aa64isar2(cpu, openbsd_read_aarch64_cpuid(ID::kAA64ISAR2));
  detect_aarch64_features_via_cpuid_aa64mmfr0(cpu, openbsd_read_aarch64_cpuid(ID::kAA64MMFR0));
  detect_aarch64_features_via_cpuid_aa64mmfr1(cpu, openbsd_read_aarch64_cpuid(ID::kAA64MMFR1));
  detect_aarch64_features_via_cpuid_aa64mmfr2(cpu, openbsd_read_aarch64_cpuid(ID::kAA64MMFR2));
  if (cpu.features().arm().has_any(Ext::kSVE, Ext::kSME)) {
    detect_aarch64_features_via_cpuid_aa64zfr0(cpu, openbsd_read_aarch64_cpuid(ID::kAA64ZFR0));
    if (cpu.features().arm().has_sme()) {
      detect_aarch64_features_via_cpuid_aa64smfr0(cpu, openbsd_read_aarch64_cpuid(ID::kAA64SMFR0));
    }
  }
  post_process_arm_cpu_info(cpu);
}
#elif defined(__APPLE__)
enum class AppleFeatureType : uint8_t {
  kHWOptional,
  kHWOptionalArmFEAT
};
struct AppleFeatureMapping {
  AppleFeatureType type;
  char name[18];
  uint8_t feature_id;
};
template<typename T>
static inline bool invoke_sysctl_by_name(const char* sysctl_name, T* dst, size_t size = sizeof(T)) noexcept {
  return sysctlbyname(sysctl_name, dst, &size, nullptr, 0) == 0;
}
static ASMJIT_FAVOR_SIZE long apple_detect_aarch64_feature_via_sysctl(AppleFeatureType type, const char* feature_name) noexcept {
  static const char hw_optional_prefix[] = "hw.optional.";
  static const char hw_optional_arm_feat_prefix[] = "hw.optional.arm.FEAT_";
  char sysctl_name[128];
  const char* prefix = type == AppleFeatureType::kHWOptional ? hw_optional_prefix : hw_optional_arm_feat_prefix;
  size_t prefix_size = (type == AppleFeatureType::kHWOptional ? sizeof(hw_optional_prefix) : sizeof(hw_optional_arm_feat_prefix)) - 1u;
  size_t feature_name_size = strlen(feature_name);
  if (feature_name_size < 128 - prefix_size) {
    memcpy(sysctl_name, prefix, prefix_size);
    memcpy(sysctl_name + prefix_size, feature_name, feature_name_size + 1u);
    long val = 0;
    if (invoke_sysctl_by_name<long>(sysctl_name, &val)) {
      return val;
    }
  }
  return 0;
}
static ASMJIT_FAVOR_SIZE void apple_detect_aarch64_features_via_sysctl(CpuInfo& cpu) noexcept {
  using FT = AppleFeatureType;
  static const AppleFeatureMapping mappings[] = {
    { FT::kHWOptional       , "AdvSIMD_HPFPCvt", uint8_t(Ext::kFP16CONV)  },
    { FT::kHWOptional       , "neon_hpfp"      , uint8_t(Ext::kFP16CONV)  },
    { FT::kHWOptionalArmFEAT, "BF16"           , uint8_t(Ext::kBF16)      },
    { FT::kHWOptionalArmFEAT, "DotProd"        , uint8_t(Ext::kDOTPROD)   },
    { FT::kHWOptionalArmFEAT, "FCMA"           , uint8_t(Ext::kFCMA)      },
    { FT::kHWOptional       , "armv8_3_compnum", uint8_t(Ext::kFCMA)      },
    { FT::kHWOptionalArmFEAT, "FHM"            , uint8_t(Ext::kFHM)       },
    { FT::kHWOptional       , "armv8_2_fhm"    , uint8_t(Ext::kFHM)       },
    { FT::kHWOptionalArmFEAT, "FP16"           , uint8_t(Ext::kFP16)      },
    { FT::kHWOptional       , "neon_fp16"      , uint8_t(Ext::kFP16)      },
    { FT::kHWOptionalArmFEAT, "FRINTTS"        , uint8_t(Ext::kFRINTTS)   },
    { FT::kHWOptionalArmFEAT, "I8MM"           , uint8_t(Ext::kI8MM)      },
    { FT::kHWOptionalArmFEAT, "JSCVT"          , uint8_t(Ext::kJSCVT)     },
    { FT::kHWOptionalArmFEAT, "RDM"            , uint8_t(Ext::kRDM)       },
    { FT::kHWOptional       , "armv8_crc32"    , uint8_t(Ext::kCRC32)     },
    { FT::kHWOptionalArmFEAT, "FlagM"          , uint8_t(Ext::kFLAGM)     },
    { FT::kHWOptionalArmFEAT, "FlagM2"         , uint8_t(Ext::kFLAGM2)    },
    { FT::kHWOptionalArmFEAT, "LRCPC"          , uint8_t(Ext::kLRCPC)     },
    { FT::kHWOptionalArmFEAT, "LRCPC2"         , uint8_t(Ext::kLRCPC2)    },
    { FT::kHWOptional       , "armv8_1_atomics", uint8_t(Ext::kLSE)       },
    { FT::kHWOptionalArmFEAT, "LSE"            , uint8_t(Ext::kLSE)       },
    { FT::kHWOptionalArmFEAT, "LSE2"           , uint8_t(Ext::kLSE2)      },
    { FT::kHWOptionalArmFEAT, "AES"            , uint8_t(Ext::kAES)       },
    { FT::kHWOptionalArmFEAT, "PMULL"          , uint8_t(Ext::kPMULL)     },
    { FT::kHWOptionalArmFEAT, "SHA1"           , uint8_t(Ext::kSHA1)      },
    { FT::kHWOptionalArmFEAT, "SHA256"         , uint8_t(Ext::kSHA256)    },
    { FT::kHWOptionalArmFEAT, "SHA512"         , uint8_t(Ext::kSHA512)    },
    { FT::kHWOptional       , "armv8_2_sha512" , uint8_t(Ext::kSHA512)    },
    { FT::kHWOptionalArmFEAT, "SHA3"           , uint8_t(Ext::kSHA3)      },
    { FT::kHWOptional       , "armv8_2_sha3"   , uint8_t(Ext::kSHA3)      },
    { FT::kHWOptionalArmFEAT, "BTI"            , uint8_t(Ext::kBTI)       },
    { FT::kHWOptionalArmFEAT, "DPB"            , uint8_t(Ext::kDPB)       },
    { FT::kHWOptionalArmFEAT, "DPB2"           , uint8_t(Ext::kDPB2)      },
    { FT::kHWOptionalArmFEAT, "ECV"            , uint8_t(Ext::kECV)       },
    { FT::kHWOptionalArmFEAT, "SB"             , uint8_t(Ext::kSB)        },
    { FT::kHWOptionalArmFEAT, "SSBS"           , uint8_t(Ext::kSSBS)      }
  };
  for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(mappings); i++) {
    const AppleFeatureMapping& mapping = mappings[i];
    if (!cpu.features().arm().has(mapping.feature_id) && apple_detect_aarch64_feature_via_sysctl(mapping.type, mapping.name)) {
      cpu.features().arm().add(mapping.feature_id);
    }
  }
}
static ASMJIT_FAVOR_SIZE void detect_arm_cpu(CpuInfo& cpu) noexcept {
  cpu._was_detected = true;
  populate_base_arm_features(cpu);
  invoke_sysctl_by_name<uint32_t>("hw.cpufamily", &cpu._family_id);
  invoke_sysctl_by_name<uint32_t>("hw.cachelinesize", &cpu._cache_line_size);
  invoke_sysctl_by_name<uint32_t>("machdep.cpu.logical_per_package", &cpu._max_logical_processors);
  invoke_sysctl_by_name<char>("machdep.cpu.brand_string", cpu._brand.str, sizeof(cpu._brand.str));
  memcpy(cpu._vendor.str, "APPLE", 6);
  bool cpu_features_populated = detect_aarch64_features_via_apple_family_id(cpu);
  if (!cpu_features_populated) {
    apple_detect_aarch64_features_via_sysctl(cpu);
  }
  post_process_arm_cpu_info(cpu);
}
#else
#if ASMJIT_ARCH_ARM == 32
  #pragma message("[asmjit] Disabling runtime CPU detection - unsupported OS/CPU combination (Unknown OS with AArch32 CPU)")
#else
  #pragma message("[asmjit] Disabling runtime CPU detection - unsupported OS/CPU combination (Unknown OS with AArch64 CPU)")
#endif
static ASMJIT_FAVOR_SIZE void detect_arm_cpu(CpuInfo& cpu) noexcept {
  populate_base_arm_features(cpu);
  detect_arm_features_via_compiler_flags(cpu);
  post_process_arm_cpu_info(cpu);
}
#endif
static ASMJIT_FAVOR_SIZE CpuHints recalculate_hints(const CpuInfo& cpu_info, const CpuFeatures::ARM& features) noexcept {
  Support::maybe_unused(cpu_info, features);
  CpuHints hints = CpuHints::kVecFastIntMul32;
  return hints;
}
}
#endif
const CpuInfo& CpuInfo::host() noexcept {
  static std::atomic<uint32_t> cpu_info_initialized_flag;
  static CpuInfo cpu_info_global(Globals::NoInit);
  if (!cpu_info_initialized_flag.load(std::memory_order_relaxed)) {
    CpuInfo cpu_info_local;
    cpu_info_local._arch = Arch::kHost;
    cpu_info_local._sub_arch = SubArch::kHost;
#if ASMJIT_ARCH_X86
    x86::detect_x86_cpu(cpu_info_local);
#elif ASMJIT_ARCH_ARM
    arm::detect_arm_cpu(cpu_info_local);
#endif
    cpu_info_local._hw_thread_count = detect_hw_thread_count();
    cpu_info_local.update_hints();
    cpu_info_global = cpu_info_local;
    cpu_info_initialized_flag.store(1, std::memory_order_seq_cst);
  }
  return cpu_info_global;
}
CpuHints CpuInfo::recalculate_hints(const CpuInfo& info, const CpuFeatures& features) noexcept {
#if ASMJIT_ARCH_X86
  return x86::recalculate_hints(info, features.x86());
#elif ASMJIT_ARCH_ARM
  return arm::recalculate_hints(info, features.arm());
#else
  Support::maybe_unused(info, features);
  return CpuHints::kNone;
#endif
}
ASMJIT_END_NAMESPACE