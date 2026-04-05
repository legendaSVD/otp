#ifndef ASMJIT_CORE_CPUINFO_H_INCLUDED
#define ASMJIT_CORE_CPUINFO_H_INCLUDED
#include <asmjit/core/archtraits.h>
#include <asmjit/core/environment.h>
#include <asmjit/core/globals.h>
#include <asmjit/core/string.h>
#include <asmjit/support/support.h>
ASMJIT_BEGIN_NAMESPACE
class CpuFeatures {
public:
  static inline constexpr uint32_t kMaxFeatures = 256;
  static inline constexpr uint32_t kNumBitWords = kMaxFeatures / Support::bit_size_of<Support::BitWord>;
  using BitWord = Support::BitWord;
  using Iterator = Support::BitVectorIterator<BitWord>;
  using Bits = Support::Array<BitWord, kNumBitWords>;
  struct Data {
    Bits _bits;
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool operator==(const Data& other) const noexcept { return  equals(other); }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool operator!=(const Data& other) const noexcept { return !equals(other); }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _bits.aggregate<Support::Or>(0) == 0; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG BitWord* bits() noexcept { return _bits.data(); }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG const BitWord* bits() const noexcept { return _bits.data(); }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t bit_word_count() const noexcept { return kNumBitWords; }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG Iterator iterator() const noexcept { return Iterator(_bits.as_span()); }
    template<typename FeatureId>
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has(const FeatureId& feature_id) const noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);
      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;
      return bool((_bits[idx] >> bit) & 0x1);
    }
    template<typename FeatureId>
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has_any(const FeatureId& feature_id) const noexcept {
      return has(feature_id);
    }
    template<typename FeatureId, typename... Args>
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has_any(const FeatureId& feature_id, Args&&... other_feature_ids) const noexcept {
      return bool(unsigned(has(feature_id)) | unsigned(has_any(std::forward<Args>(other_feature_ids)...)));
    }
    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool has_all(const Data& other) const noexcept {
      uint32_t result = 1;
      for (uint32_t i = 0; i < kNumBitWords; i++)
        result &= uint32_t((_bits[i] & other._bits[i]) == other._bits[i]);
      return bool(result);
    }
    ASMJIT_INLINE_NODEBUG void reset() noexcept { _bits.fill(0); }
    template<typename FeatureId>
    inline void add(const FeatureId& feature_id) noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);
      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;
      _bits[idx] |= BitWord(1) << bit;
    }
    template<typename FeatureId, typename... Args>
    inline void add(const FeatureId& feature_id, Args&&... other_feature_ids) noexcept {
      add(feature_id);
      add(std::forward<Args>(other_feature_ids)...);
    }
    template<typename FeatureId>
    inline void add_if(bool condition, const FeatureId& feature_id) noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);
      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;
      _bits[idx] |= BitWord(condition) << bit;
    }
    template<typename FeatureId, typename... Args>
    inline void add_if(bool condition, const FeatureId& feature_id, Args&&... other_feature_ids) noexcept {
      add_if(condition, feature_id);
      add_if(condition, std::forward<Args>(other_feature_ids)...);
    }
    template<typename FeatureId>
    inline void remove(const FeatureId& feature_id) noexcept {
      ASMJIT_ASSERT(uint32_t(feature_id) < kMaxFeatures);
      uint32_t idx = uint32_t(feature_id) / Support::bit_size_of<BitWord>;
      uint32_t bit = uint32_t(feature_id) % Support::bit_size_of<BitWord>;
      _bits[idx] &= ~(BitWord(1) << bit);
    }
    template<typename FeatureId, typename... Args>
    inline void remove(const FeatureId& feature_id, Args&&... other_feature_ids) noexcept {
      remove(feature_id);
      remove(std::forward<Args>(other_feature_ids)...);
    }
    ASMJIT_INLINE_NODEBUG bool equals(const Data& other) const noexcept { return _bits == other._bits; }
  };
  struct X86 : public Data {
    enum Id : uint8_t {
      kNone,
      kMT,
      kNX,
      kADX,
      kALTMOVCR8,
      kAPX_F,
      kBMI,
      kBMI2,
      kCET_IBT,
      kCET_SS,
      kCET_SSS,
      kCLDEMOTE,
      kCLFLUSH,
      kCLFLUSHOPT,
      kCLWB,
      kCLZERO,
      kCMOV,
      kCMPCCXADD,
      kCMPXCHG16B,
      kCMPXCHG8B,
      kENCLV,
      kENQCMD,
      kERMS,
      kFSGSBASE,
      kFSRM,
      kFSRC,
      kFSRS,
      kFXSR,
      kFXSROPT,
      kFZRM,
      kHRESET,
      kI486,
      kINVLPGB,
      kLAHFSAHF,
      kLAM,
      kLWP,
      kLZCNT,
      kMCOMMIT,
      kMONITOR,
      kMONITORX,
      kMOVBE,
      kMOVDIR64B,
      kMOVDIRI,
      kMOVRS,
      kMPX,
      kMSR,
      kMSRLIST,
      kMSR_IMM,
      kMSSE,
      kOSXSAVE,
      kOSPKE,
      kPCONFIG,
      kPOPCNT,
      kPREFETCHI,
      kPREFETCHW,
      kPREFETCHWT1,
      kPTWRITE,
      kRAO_INT,
      kRMPQUERY,
      kRDPID,
      kRDPRU,
      kRDRAND,
      kRDSEED,
      kRDTSC,
      kRDTSCP,
      kRTM,
      kSEAM,
      kSERIALIZE,
      kSEV,
      kSEV_ES,
      kSEV_SNP,
      kSKINIT,
      kSMAP,
      kSME,
      kSMEP,
      kSMX,
      kSVM,
      kTBM,
      kTSE,
      kTSXLDTRK,
      kUINTR,
      kVMX,
      kWAITPKG,
      kWBNOINVD,
      kWRMSRNS,
      kXSAVE,
      kXSAVEC,
      kXSAVEOPT,
      kXSAVES,
      kFPU,
      kMMX,
      kMMX2,
      k3DNOW,
      k3DNOW2,
      kGEODE,
      kSSE,
      kSSE2,
      kSSE3,
      kSSSE3,
      kSSE4_1,
      kSSE4_2,
      kSSE4A,
      kPCLMULQDQ,
      kAVX,
      kAVX2,
      kAVX_IFMA,
      kAVX_NE_CONVERT,
      kAVX_VNNI,
      kAVX_VNNI_INT16,
      kAVX_VNNI_INT8,
      kF16C,
      kFMA,
      kFMA4,
      kXOP,
      kAVX512_BF16,
      kAVX512_BITALG,
      kAVX512_BW,
      kAVX512_CD,
      kAVX512_DQ,
      kAVX512_F,
      kAVX512_FP16,
      kAVX512_IFMA,
      kAVX512_VBMI,
      kAVX512_VBMI2,
      kAVX512_VL,
      kAVX512_VNNI,
      kAVX512_VP2INTERSECT,
      kAVX512_VPOPCNTDQ,
      kAESNI,
      kGFNI,
      kSHA,
      kSHA512,
      kSM3,
      kSM4,
      kVAES,
      kVPCLMULQDQ,
      kKL,
      kAESKLE,
      kAESKLEWIDE_KL,
      kAVX10_1,
      kAVX10_2,
      kAMX_AVX512,
      kAMX_BF16,
      kAMX_COMPLEX,
      kAMX_FP16,
      kAMX_FP8,
      kAMX_INT8,
      kAMX_MOVRS,
      kAMX_TF32,
      kAMX_TILE,
      kAMX_TRANSPOSE,
      kMaxValue = kAMX_TILE
    };
    #define ASMJIT_X86_FEATURE(accessor, feature) \
       \
      ASMJIT_INLINE_NODEBUG bool accessor() const noexcept { return has(X86::feature); }
    ASMJIT_X86_FEATURE(has_mt, kMT)
    ASMJIT_X86_FEATURE(has_nx, kNX)
    ASMJIT_X86_FEATURE(has_adx, kADX)
    ASMJIT_X86_FEATURE(has_altmovcr8, kALTMOVCR8)
    ASMJIT_X86_FEATURE(has_apx_f, kAPX_F)
    ASMJIT_X86_FEATURE(has_bmi, kBMI)
    ASMJIT_X86_FEATURE(has_bmi2, kBMI2)
    ASMJIT_X86_FEATURE(has_cet_ibt, kCET_IBT)
    ASMJIT_X86_FEATURE(has_cet_ss, kCET_SS)
    ASMJIT_X86_FEATURE(has_cet_sss, kCET_SSS)
    ASMJIT_X86_FEATURE(has_cldemote, kCLDEMOTE)
    ASMJIT_X86_FEATURE(has_clflush, kCLFLUSH)
    ASMJIT_X86_FEATURE(has_clflushopt, kCLFLUSHOPT)
    ASMJIT_X86_FEATURE(has_clwb, kCLWB)
    ASMJIT_X86_FEATURE(has_clzero, kCLZERO)
    ASMJIT_X86_FEATURE(has_cmov, kCMOV)
    ASMJIT_X86_FEATURE(has_cmpxchg16b, kCMPXCHG16B)
    ASMJIT_X86_FEATURE(has_cmpxchg8b, kCMPXCHG8B)
    ASMJIT_X86_FEATURE(has_enclv, kENCLV)
    ASMJIT_X86_FEATURE(has_enqcmd, kENQCMD)
    ASMJIT_X86_FEATURE(has_erms, kERMS)
    ASMJIT_X86_FEATURE(has_fsgsbase, kFSGSBASE)
    ASMJIT_X86_FEATURE(has_fsrm, kFSRM)
    ASMJIT_X86_FEATURE(has_fsrc, kFSRC)
    ASMJIT_X86_FEATURE(has_fsrs, kFSRS)
    ASMJIT_X86_FEATURE(has_fxsr, kFXSR)
    ASMJIT_X86_FEATURE(has_fxsropt, kFXSROPT)
    ASMJIT_X86_FEATURE(has_fzrm, kFZRM)
    ASMJIT_X86_FEATURE(has_hreset, kHRESET)
    ASMJIT_X86_FEATURE(has_i486, kI486)
    ASMJIT_X86_FEATURE(has_invlpgb, kINVLPGB)
    ASMJIT_X86_FEATURE(has_lahfsahf, kLAHFSAHF)
    ASMJIT_X86_FEATURE(has_lam, kLAM)
    ASMJIT_X86_FEATURE(has_lwp, kLWP)
    ASMJIT_X86_FEATURE(has_lzcnt, kLZCNT)
    ASMJIT_X86_FEATURE(has_mcommit, kMCOMMIT)
    ASMJIT_X86_FEATURE(has_monitor, kMONITOR)
    ASMJIT_X86_FEATURE(has_monitorx, kMONITORX)
    ASMJIT_X86_FEATURE(has_movbe, kMOVBE)
    ASMJIT_X86_FEATURE(has_movdir64b, kMOVDIR64B)
    ASMJIT_X86_FEATURE(has_movdiri, kMOVDIRI)
    ASMJIT_X86_FEATURE(has_movrs, kMOVRS)
    ASMJIT_X86_FEATURE(has_mpx, kMPX)
    ASMJIT_X86_FEATURE(has_msr, kMSR)
    ASMJIT_X86_FEATURE(has_msrlist, kMSRLIST)
    ASMJIT_X86_FEATURE(has_msr_imm, kMSR_IMM)
    ASMJIT_X86_FEATURE(has_msse, kMSSE)
    ASMJIT_X86_FEATURE(has_osxsave, kOSXSAVE)
    ASMJIT_X86_FEATURE(has_ospke, kOSPKE)
    ASMJIT_X86_FEATURE(has_pconfig, kPCONFIG)
    ASMJIT_X86_FEATURE(has_popcnt, kPOPCNT)
    ASMJIT_X86_FEATURE(has_prefetchi, kPREFETCHI)
    ASMJIT_X86_FEATURE(has_prefetchw, kPREFETCHW)
    ASMJIT_X86_FEATURE(has_prefetchwt1, kPREFETCHWT1)
    ASMJIT_X86_FEATURE(has_ptwrite, kPTWRITE)
    ASMJIT_X86_FEATURE(has_rao_int, kRAO_INT)
    ASMJIT_X86_FEATURE(has_rmpquery, kRMPQUERY)
    ASMJIT_X86_FEATURE(has_rdpid, kRDPID)
    ASMJIT_X86_FEATURE(has_rdpru, kRDPRU)
    ASMJIT_X86_FEATURE(has_rdrand, kRDRAND)
    ASMJIT_X86_FEATURE(has_rdseed, kRDSEED)
    ASMJIT_X86_FEATURE(has_rdtsc, kRDTSC)
    ASMJIT_X86_FEATURE(has_rdtscp, kRDTSCP)
    ASMJIT_X86_FEATURE(has_rtm, kRTM)
    ASMJIT_X86_FEATURE(has_seam, kSEAM)
    ASMJIT_X86_FEATURE(has_serialize, kSERIALIZE)
    ASMJIT_X86_FEATURE(has_sev, kSEV)
    ASMJIT_X86_FEATURE(has_sev_es, kSEV_ES)
    ASMJIT_X86_FEATURE(has_sev_snp, kSEV_SNP)
    ASMJIT_X86_FEATURE(has_skinit, kSKINIT)
    ASMJIT_X86_FEATURE(has_smap, kSMAP)
    ASMJIT_X86_FEATURE(has_smep, kSMEP)
    ASMJIT_X86_FEATURE(has_smx, kSMX)
    ASMJIT_X86_FEATURE(has_svm, kSVM)
    ASMJIT_X86_FEATURE(has_tbm, kTBM)
    ASMJIT_X86_FEATURE(has_tse, kTSE)
    ASMJIT_X86_FEATURE(has_tsxldtrk, kTSXLDTRK)
    ASMJIT_X86_FEATURE(has_uintr, kUINTR)
    ASMJIT_X86_FEATURE(has_vmx, kVMX)
    ASMJIT_X86_FEATURE(has_waitpkg, kWAITPKG)
    ASMJIT_X86_FEATURE(has_wbnoinvd, kWBNOINVD)
    ASMJIT_X86_FEATURE(has_wrmsrns, kWRMSRNS)
    ASMJIT_X86_FEATURE(has_xsave, kXSAVE)
    ASMJIT_X86_FEATURE(has_xsavec, kXSAVEC)
    ASMJIT_X86_FEATURE(has_xsaveopt, kXSAVEOPT)
    ASMJIT_X86_FEATURE(has_xsaves, kXSAVES)
    ASMJIT_X86_FEATURE(has_fpu, kFPU)
    ASMJIT_X86_FEATURE(has_mmx, kMMX)
    ASMJIT_X86_FEATURE(has_mmx2, kMMX2)
    ASMJIT_X86_FEATURE(has_3dnow, k3DNOW)
    ASMJIT_X86_FEATURE(has_3dnow2, k3DNOW2)
    ASMJIT_X86_FEATURE(has_geode, kGEODE)
    ASMJIT_X86_FEATURE(has_sse, kSSE)
    ASMJIT_X86_FEATURE(has_sse2, kSSE2)
    ASMJIT_X86_FEATURE(has_sse3, kSSE3)
    ASMJIT_X86_FEATURE(has_ssse3, kSSSE3)
    ASMJIT_X86_FEATURE(has_sse4_1, kSSE4_1)
    ASMJIT_X86_FEATURE(has_sse4_2, kSSE4_2)
    ASMJIT_X86_FEATURE(has_sse4a, kSSE4A)
    ASMJIT_X86_FEATURE(has_pclmulqdq, kPCLMULQDQ)
    ASMJIT_X86_FEATURE(has_avx, kAVX)
    ASMJIT_X86_FEATURE(has_avx2, kAVX2)
    ASMJIT_X86_FEATURE(has_avx_ifma, kAVX_IFMA)
    ASMJIT_X86_FEATURE(has_avx_ne_convert, kAVX_NE_CONVERT)
    ASMJIT_X86_FEATURE(has_avx_vnni, kAVX_VNNI)
    ASMJIT_X86_FEATURE(has_avx_vnni_int16, kAVX_VNNI_INT16)
    ASMJIT_X86_FEATURE(has_avx_vnni_int8, kAVX_VNNI_INT8)
    ASMJIT_X86_FEATURE(has_f16c, kF16C)
    ASMJIT_X86_FEATURE(has_fma, kFMA)
    ASMJIT_X86_FEATURE(has_fma4, kFMA4)
    ASMJIT_X86_FEATURE(has_xop, kXOP)
    ASMJIT_X86_FEATURE(has_avx512_bf16, kAVX512_BF16)
    ASMJIT_X86_FEATURE(has_avx512_bitalg, kAVX512_BITALG)
    ASMJIT_X86_FEATURE(has_avx512_bw, kAVX512_BW)
    ASMJIT_X86_FEATURE(has_avx512_cd, kAVX512_CD)
    ASMJIT_X86_FEATURE(has_avx512_dq, kAVX512_DQ)
    ASMJIT_X86_FEATURE(has_avx512_f, kAVX512_F)
    ASMJIT_X86_FEATURE(has_avx512_fp16, kAVX512_FP16)
    ASMJIT_X86_FEATURE(has_avx512_ifma, kAVX512_IFMA)
    ASMJIT_X86_FEATURE(has_avx512_vbmi, kAVX512_VBMI)
    ASMJIT_X86_FEATURE(has_avx512_vbmi2, kAVX512_VBMI2)
    ASMJIT_X86_FEATURE(has_avx512_vl, kAVX512_VL)
    ASMJIT_X86_FEATURE(has_avx512_vnni, kAVX512_VNNI)
    ASMJIT_X86_FEATURE(has_avx512_vp2intersect, kAVX512_VP2INTERSECT)
    ASMJIT_X86_FEATURE(has_avx512_vpopcntdq, kAVX512_VPOPCNTDQ)
    ASMJIT_X86_FEATURE(has_aesni, kAESNI)
    ASMJIT_X86_FEATURE(has_gfni, kGFNI)
    ASMJIT_X86_FEATURE(has_sha, kSHA)
    ASMJIT_X86_FEATURE(has_sha512, kSHA512)
    ASMJIT_X86_FEATURE(has_sm3, kSM3)
    ASMJIT_X86_FEATURE(has_sm4, kSM4)
    ASMJIT_X86_FEATURE(has_vaes, kVAES)
    ASMJIT_X86_FEATURE(has_vpclmulqdq, kVPCLMULQDQ)
    ASMJIT_X86_FEATURE(has_kl, kKL)
    ASMJIT_X86_FEATURE(has_aeskle, kAESKLE)
    ASMJIT_X86_FEATURE(has_aesklewide_kl, kAESKLEWIDE_KL)
    ASMJIT_X86_FEATURE(has_avx10_1, kAVX10_1)
    ASMJIT_X86_FEATURE(has_avx10_2, kAVX10_2)
    ASMJIT_X86_FEATURE(has_amx_avx512, kAMX_AVX512)
    ASMJIT_X86_FEATURE(has_amx_bf16, kAMX_BF16)
    ASMJIT_X86_FEATURE(has_amx_complex, kAMX_COMPLEX)
    ASMJIT_X86_FEATURE(has_amx_fp16, kAMX_FP16)
    ASMJIT_X86_FEATURE(has_amx_fp8, kAMX_FP8)
    ASMJIT_X86_FEATURE(has_amx_int8, kAMX_INT8)
    ASMJIT_X86_FEATURE(has_amx_movrs, kAMX_MOVRS)
    ASMJIT_X86_FEATURE(has_amx_tf32, kAMX_TF32)
    ASMJIT_X86_FEATURE(has_amx_tile, kAMX_TILE)
    ASMJIT_X86_FEATURE(has_amx_transpose, kAMX_TRANSPOSE)
    #undef ASMJIT_X86_FEATURE
    ASMJIT_INLINE void remove_avx() noexcept {
      remove(kAVX                 ,
             kAVX2                ,
             kAVX_IFMA            ,
             kAVX_NE_CONVERT      ,
             kAVX_VNNI            ,
             kAVX_VNNI_INT16      ,
             kAVX_VNNI_INT8       ,
             kF16C                ,
             kFMA                 ,
             kFMA4                ,
             kVAES                ,
             kVPCLMULQDQ          ,
             kXOP);
      remove_avx512();
    }
    ASMJIT_INLINE void remove_avx512() noexcept {
      remove(kAVX512_BF16         ,
             kAVX512_BITALG       ,
             kAVX512_BW           ,
             kAVX512_CD           ,
             kAVX512_DQ           ,
             kAVX512_F            ,
             kAVX512_FP16         ,
             kAVX512_IFMA         ,
             kAVX512_VBMI         ,
             kAVX512_VBMI2        ,
             kAVX512_VL           ,
             kAVX512_VNNI         ,
             kAVX512_VP2INTERSECT ,
             kAVX512_VPOPCNTDQ    ,
             kAMX_AVX512);
      remove_avx10();
    }
    ASMJIT_INLINE void remove_avx10() noexcept {
      remove(kAVX10_1 | kAVX10_2);
    }
    ASMJIT_INLINE void remove_amx() noexcept {
      remove(kAMX_AVX512          ,
             kAMX_BF16            ,
             kAMX_COMPLEX         ,
             kAMX_FP16            ,
             kAMX_FP8             ,
             kAMX_INT8            ,
             kAMX_MOVRS           ,
             kAMX_TF32            ,
             kAMX_TILE            ,
             kAMX_TRANSPOSE);
    }
  };
  struct ARM : public Data {
    enum Id : uint8_t {
      kNone = 0,
      kARMv6,
      kARMv7,
      kARMv8a,
      kTHUMB,
      kTHUMBv2,
      kABLE,
      kADERR,
      kAES,
      kAFP,
      kAIE,
      kAMU1,
      kAMU1_1,
      kANERR,
      kASIMD,
      kBF16,
      kBRBE,
      kBTI,
      kBWE,
      kCCIDX,
      kCHK,
      kCLRBHB,
      kCMOW,
      kCMPBR,
      kCONSTPACFIELD,
      kCPA,
      kCPA2,
      kCPUID,
      kCRC32,
      kCSSC,
      kCSV2,
      kCSV2_3,
      kCSV3,
      kD128,
      kDGH,
      kDIT,
      kDOTPROD,
      kDPB,
      kDPB2,
      kEBEP,
      kEBF16,
      kECBHB,
      kECV,
      kEDHSR,
      kEDSP,
      kF8E4M3,
      kF8E5M2,
      kF8F16MM,
      kF8F32MM,
      kFAMINMAX,
      kFCMA,
      kFGT,
      kFGT2,
      kFHM,
      kFLAGM,
      kFLAGM2,
      kFMAC,
      kFP,
      kFP16,
      kFP16CONV,
      kFP8,
      kFP8DOT2,
      kFP8DOT4,
      kFP8FMA,
      kFPMR,
      kFPRCVT,
      kFRINTTS,
      kGCS,
      kHACDBS,
      kHAFDBS,
      kHAFT,
      kHDBSS,
      kHBC,
      kHCX,
      kHPDS,
      kHPDS2,
      kI8MM,
      kIDIVA,
      kIDIVT,
      kITE,
      kJSCVT,
      kLOR,
      kLRCPC,
      kLRCPC2,
      kLRCPC3,
      kLS64,
      kLS64_ACCDATA,
      kLS64_V,
      kLS64WB,
      kLSE,
      kLSE128,
      kLSE2,
      kLSFE,
      kLSUI,
      kLUT,
      kLVA,
      kLVA3,
      kMEC,
      kMOPS,
      kMPAM,
      kMTE,
      kMTE2,
      kMTE3,
      kMTE4,
      kMTE_ASYM_FAULT,
      kMTE_ASYNC,
      kMTE_CANONICAL_TAGS,
      kMTE_NO_ADDRESS_TAGS,
      kMTE_PERM_S1,
      kMTE_STORE_ONLY,
      kMTE_TAGGED_FAR,
      kMTPMU,
      kNMI,
      kNV,
      kNV2,
      kOCCMO,
      kPAN,
      kPAN2,
      kPAN3,
      kPAUTH,
      kPFAR,
      kPMU,
      kPMULL,
      kPRFMSLC,
      kRAS,
      kRAS1_1,
      kRAS2,
      kRASSA2,
      kRDM,
      kRME,
      kRNG,
      kRNG_TRAP,
      kRPRES,
      kRPRFM,
      kS1PIE,
      kS1POE,
      kS2PIE,
      kS2POE,
      kSB,
      kSCTLR2,
      kSEBEP,
      kSEL2,
      kSHA1,
      kSHA256,
      kSHA3,
      kSHA512,
      kSM3,
      kSM4,
      kSME,
      kSME2,
      kSME2_1,
      kSME2_2,
      kSME_AES,
      kSME_B16B16,
      kSME_B16F32,
      kSME_BI32I32,
      kSME_F16F16,
      kSME_F16F32,
      kSME_F32F32,
      kSME_F64F64,
      kSME_F8F16,
      kSME_F8F32,
      kSME_FA64,
      kSME_I16I32,
      kSME_I16I64,
      kSME_I8I32,
      kSME_LUTv2,
      kSME_MOP4,
      kSME_TMOP,
      kSPE,
      kSPE1_1,
      kSPE1_2,
      kSPE1_3,
      kSPE1_4,
      kSPE_ALTCLK,
      kSPE_CRR,
      kSPE_EFT,
      kSPE_FDS,
      kSPE_FPF,
      kSPE_SME,
      kSPECRES,
      kSPECRES2,
      kSPMU,
      kSSBS,
      kSSBS2,
      kSSVE_AES,
      kSSVE_BITPERM,
      kSSVE_FEXPA,
      kSSVE_FP8DOT2,
      kSSVE_FP8DOT4,
      kSSVE_FP8FMA,
      kSVE,
      kSVE2,
      kSVE2_1,
      kSVE2_2,
      kSVE_AES,
      kSVE_AES2,
      kSVE_B16B16,
      kSVE_BF16,
      kSVE_BFSCALE,
      kSVE_BITPERM,
      kSVE_EBF16,
      kSVE_ELTPERM,
      kSVE_F16MM,
      kSVE_F32MM,
      kSVE_F64MM,
      kSVE_I8MM,
      kSVE_PMULL128,
      kSVE_SHA3,
      kSVE_SM4,
      kSYSINSTR128,
      kSYSREG128,
      kTHE,
      kTLBIOS,
      kTLBIRANGE,
      kTLBIW,
      kTME,
      kTRF,
      kUAO,
      kVFP_D32,
      kVHE,
      kVMID16,
      kWFXT,
      kXNX,
      kXS,
      kMaxValue = kXS
    };
    #define ASMJIT_ARM_FEATURE(accessor, feature) \
       \
      ASMJIT_INLINE_NODEBUG bool accessor() const noexcept { return has(ARM::feature); }
    ASMJIT_ARM_FEATURE(has_thumb, kTHUMB)
    ASMJIT_ARM_FEATURE(has_thumb_v2, kTHUMBv2)
    ASMJIT_ARM_FEATURE(has_armv6, kARMv6)
    ASMJIT_ARM_FEATURE(has_armv7, kARMv7)
    ASMJIT_ARM_FEATURE(has_armv8a, kARMv8a)
    ASMJIT_ARM_FEATURE(has_able, kABLE)
    ASMJIT_ARM_FEATURE(has_aderr, kADERR)
    ASMJIT_ARM_FEATURE(has_aes, kAES)
    ASMJIT_ARM_FEATURE(has_afp, kAFP)
    ASMJIT_ARM_FEATURE(has_aie, kAIE)
    ASMJIT_ARM_FEATURE(has_amu1, kAMU1)
    ASMJIT_ARM_FEATURE(has_amu1_1, kAMU1_1)
    ASMJIT_ARM_FEATURE(has_anerr, kANERR)
    ASMJIT_ARM_FEATURE(has_asimd, kASIMD)
    ASMJIT_ARM_FEATURE(has_bf16, kBF16)
    ASMJIT_ARM_FEATURE(has_brbe, kBRBE)
    ASMJIT_ARM_FEATURE(has_bti, kBTI)
    ASMJIT_ARM_FEATURE(has_bwe, kBWE)
    ASMJIT_ARM_FEATURE(has_ccidx, kCCIDX)
    ASMJIT_ARM_FEATURE(has_chk, kCHK)
    ASMJIT_ARM_FEATURE(has_clrbhb, kCLRBHB)
    ASMJIT_ARM_FEATURE(has_cmow, kCMOW)
    ASMJIT_ARM_FEATURE(has_cmpbr, kCMPBR)
    ASMJIT_ARM_FEATURE(has_constpacfield, kCONSTPACFIELD)
    ASMJIT_ARM_FEATURE(has_cpa, kCPA)
    ASMJIT_ARM_FEATURE(has_cpa2, kCPA2)
    ASMJIT_ARM_FEATURE(has_cpuid, kCPUID)
    ASMJIT_ARM_FEATURE(has_crc32, kCRC32)
    ASMJIT_ARM_FEATURE(has_cssc, kCSSC)
    ASMJIT_ARM_FEATURE(has_csv2, kCSV2)
    ASMJIT_ARM_FEATURE(has_csv2_3, kCSV2_3)
    ASMJIT_ARM_FEATURE(has_csv3, kCSV3)
    ASMJIT_ARM_FEATURE(has_d128, kD128)
    ASMJIT_ARM_FEATURE(has_dgh, kDGH)
    ASMJIT_ARM_FEATURE(has_dit, kDIT)
    ASMJIT_ARM_FEATURE(has_dotprod, kDOTPROD)
    ASMJIT_ARM_FEATURE(has_dpb, kDPB)
    ASMJIT_ARM_FEATURE(has_dpb2, kDPB2)
    ASMJIT_ARM_FEATURE(has_ebep, kEBEP)
    ASMJIT_ARM_FEATURE(has_ebf16, kEBF16)
    ASMJIT_ARM_FEATURE(has_ecbhb, kECBHB)
    ASMJIT_ARM_FEATURE(has_ecv, kECV)
    ASMJIT_ARM_FEATURE(has_edhsr, kEDHSR)
    ASMJIT_ARM_FEATURE(has_edsp, kEDSP)
    ASMJIT_ARM_FEATURE(has_f8e4m3, kF8E4M3)
    ASMJIT_ARM_FEATURE(has_f8e5m2, kF8E5M2)
    ASMJIT_ARM_FEATURE(has_f8f16mm, kF8F16MM)
    ASMJIT_ARM_FEATURE(has_f8f32mm, kF8F32MM)
    ASMJIT_ARM_FEATURE(has_faminmax, kFAMINMAX)
    ASMJIT_ARM_FEATURE(has_fcma, kFCMA)
    ASMJIT_ARM_FEATURE(has_fgt, kFGT)
    ASMJIT_ARM_FEATURE(has_fgt2, kFGT2)
    ASMJIT_ARM_FEATURE(has_fhm, kFHM)
    ASMJIT_ARM_FEATURE(has_flagm, kFLAGM)
    ASMJIT_ARM_FEATURE(has_flagm2, kFLAGM2)
    ASMJIT_ARM_FEATURE(has_fmac, kFMAC)
    ASMJIT_ARM_FEATURE(has_fp, kFP)
    ASMJIT_ARM_FEATURE(has_fp16, kFP16)
    ASMJIT_ARM_FEATURE(has_fp16conv, kFP16CONV)
    ASMJIT_ARM_FEATURE(has_fp8, kFP8)
    ASMJIT_ARM_FEATURE(has_fp8dot2, kFP8DOT2)
    ASMJIT_ARM_FEATURE(has_fp8dot4, kFP8DOT4)
    ASMJIT_ARM_FEATURE(has_fp8fma, kFP8FMA)
    ASMJIT_ARM_FEATURE(has_fpmr, kFPMR)
    ASMJIT_ARM_FEATURE(has_fprcvt, kFPRCVT)
    ASMJIT_ARM_FEATURE(has_frintts, kFRINTTS)
    ASMJIT_ARM_FEATURE(has_gcs, kGCS)
    ASMJIT_ARM_FEATURE(has_hacdbs, kHACDBS)
    ASMJIT_ARM_FEATURE(has_hafdbs, kHAFDBS)
    ASMJIT_ARM_FEATURE(has_haft, kHAFT)
    ASMJIT_ARM_FEATURE(has_hdbss, kHDBSS)
    ASMJIT_ARM_FEATURE(has_hbc, kHBC)
    ASMJIT_ARM_FEATURE(has_hcx, kHCX)
    ASMJIT_ARM_FEATURE(has_hpds, kHPDS)
    ASMJIT_ARM_FEATURE(has_hpds2, kHPDS2)
    ASMJIT_ARM_FEATURE(has_i8mm, kI8MM)
    ASMJIT_ARM_FEATURE(has_idiva, kIDIVA)
    ASMJIT_ARM_FEATURE(has_idivt, kIDIVT)
    ASMJIT_ARM_FEATURE(has_ite, kITE)
    ASMJIT_ARM_FEATURE(has_jscvt, kJSCVT)
    ASMJIT_ARM_FEATURE(has_lor, kLOR)
    ASMJIT_ARM_FEATURE(has_lrcpc, kLRCPC)
    ASMJIT_ARM_FEATURE(has_lrcpc2, kLRCPC2)
    ASMJIT_ARM_FEATURE(has_lrcpc3, kLRCPC3)
    ASMJIT_ARM_FEATURE(has_ls64, kLS64)
    ASMJIT_ARM_FEATURE(has_ls64_accdata, kLS64_ACCDATA)
    ASMJIT_ARM_FEATURE(has_ls64_v, kLS64_V)
    ASMJIT_ARM_FEATURE(has_ls64wb, kLS64WB)
    ASMJIT_ARM_FEATURE(has_lse, kLSE)
    ASMJIT_ARM_FEATURE(has_lse128, kLSE128)
    ASMJIT_ARM_FEATURE(has_lse2, kLSE2)
    ASMJIT_ARM_FEATURE(has_lsfe, kLSFE)
    ASMJIT_ARM_FEATURE(has_lsui, kLSUI)
    ASMJIT_ARM_FEATURE(has_lut, kLUT)
    ASMJIT_ARM_FEATURE(has_lva, kLVA)
    ASMJIT_ARM_FEATURE(has_lva3, kLVA3)
    ASMJIT_ARM_FEATURE(has_mec, kMEC)
    ASMJIT_ARM_FEATURE(has_mops, kMOPS)
    ASMJIT_ARM_FEATURE(has_mpam, kMPAM)
    ASMJIT_ARM_FEATURE(has_mte, kMTE)
    ASMJIT_ARM_FEATURE(has_mte2, kMTE2)
    ASMJIT_ARM_FEATURE(has_mte3, kMTE3)
    ASMJIT_ARM_FEATURE(has_mte4, kMTE4)
    ASMJIT_ARM_FEATURE(has_mte_asym_fault, kMTE_ASYM_FAULT)
    ASMJIT_ARM_FEATURE(has_mte_async, kMTE_ASYNC)
    ASMJIT_ARM_FEATURE(has_mte_canonical_tags, kMTE_CANONICAL_TAGS)
    ASMJIT_ARM_FEATURE(has_mte_no_address_tags, kMTE_NO_ADDRESS_TAGS)
    ASMJIT_ARM_FEATURE(has_mte_perm_s1, kMTE_PERM_S1)
    ASMJIT_ARM_FEATURE(has_mte_store_only, kMTE_STORE_ONLY)
    ASMJIT_ARM_FEATURE(has_mte_tagged_far, kMTE_TAGGED_FAR)
    ASMJIT_ARM_FEATURE(has_mtpmu, kMTPMU)
    ASMJIT_ARM_FEATURE(has_nmi, kNMI)
    ASMJIT_ARM_FEATURE(has_nv, kNV)
    ASMJIT_ARM_FEATURE(has_nv2, kNV2)
    ASMJIT_ARM_FEATURE(has_occmo, kOCCMO)
    ASMJIT_ARM_FEATURE(has_pan, kPAN)
    ASMJIT_ARM_FEATURE(has_pan2, kPAN2)
    ASMJIT_ARM_FEATURE(has_pan3, kPAN3)
    ASMJIT_ARM_FEATURE(has_pauth, kPAUTH)
    ASMJIT_ARM_FEATURE(has_pfar, kPFAR)
    ASMJIT_ARM_FEATURE(has_pmu, kPMU)
    ASMJIT_ARM_FEATURE(has_pmull, kPMULL)
    ASMJIT_ARM_FEATURE(has_prfmslc, kPRFMSLC)
    ASMJIT_ARM_FEATURE(has_ras, kRAS)
    ASMJIT_ARM_FEATURE(has_ras1_1, kRAS1_1)
    ASMJIT_ARM_FEATURE(has_ras2, kRAS2)
    ASMJIT_ARM_FEATURE(has_rassa2, kRASSA2)
    ASMJIT_ARM_FEATURE(has_rdm, kRDM)
    ASMJIT_ARM_FEATURE(has_rme, kRME)
    ASMJIT_ARM_FEATURE(has_rng, kRNG)
    ASMJIT_ARM_FEATURE(has_rng_trap, kRNG_TRAP)
    ASMJIT_ARM_FEATURE(has_rpres, kRPRES)
    ASMJIT_ARM_FEATURE(has_rprfm, kRPRFM)
    ASMJIT_ARM_FEATURE(has_s1pie, kS1PIE)
    ASMJIT_ARM_FEATURE(has_s1poe, kS1POE)
    ASMJIT_ARM_FEATURE(has_s2pie, kS2PIE)
    ASMJIT_ARM_FEATURE(has_s2poe, kS2POE)
    ASMJIT_ARM_FEATURE(has_sb, kSB)
    ASMJIT_ARM_FEATURE(has_sctlr2, kSCTLR2)
    ASMJIT_ARM_FEATURE(has_sebep, kSEBEP)
    ASMJIT_ARM_FEATURE(has_sel2, kSEL2)
    ASMJIT_ARM_FEATURE(has_sha1, kSHA1)
    ASMJIT_ARM_FEATURE(has_sha256, kSHA256)
    ASMJIT_ARM_FEATURE(has_sha3, kSHA3)
    ASMJIT_ARM_FEATURE(has_sha512, kSHA512)
    ASMJIT_ARM_FEATURE(has_sm3, kSM3)
    ASMJIT_ARM_FEATURE(has_sm4, kSM4)
    ASMJIT_ARM_FEATURE(has_sme, kSME)
    ASMJIT_ARM_FEATURE(has_sme2, kSME2)
    ASMJIT_ARM_FEATURE(has_sme2_1, kSME2_1)
    ASMJIT_ARM_FEATURE(has_sme2_2, kSME2_2)
    ASMJIT_ARM_FEATURE(has_sme_aes, kSME_AES)
    ASMJIT_ARM_FEATURE(has_sme_b16b16, kSME_B16B16)
    ASMJIT_ARM_FEATURE(has_sme_b16f32, kSME_B16F32)
    ASMJIT_ARM_FEATURE(has_sme_bi32i32, kSME_BI32I32)
    ASMJIT_ARM_FEATURE(has_sme_f16f16, kSME_F16F16)
    ASMJIT_ARM_FEATURE(has_sme_f16f32, kSME_F16F32)
    ASMJIT_ARM_FEATURE(has_sme_f32f32, kSME_F32F32)
    ASMJIT_ARM_FEATURE(has_sme_f64f64, kSME_F64F64)
    ASMJIT_ARM_FEATURE(has_sme_f8f16, kSME_F8F16)
    ASMJIT_ARM_FEATURE(has_sme_f8f32, kSME_F8F32)
    ASMJIT_ARM_FEATURE(has_sme_fa64, kSME_FA64)
    ASMJIT_ARM_FEATURE(has_sme_i16i32, kSME_I16I32)
    ASMJIT_ARM_FEATURE(has_sme_i16i64, kSME_I16I64)
    ASMJIT_ARM_FEATURE(has_sme_i8i32, kSME_I8I32)
    ASMJIT_ARM_FEATURE(has_sme_lutv2, kSME_LUTv2)
    ASMJIT_ARM_FEATURE(has_sme_mop4, kSME_MOP4)
    ASMJIT_ARM_FEATURE(has_sme_tmop, kSME_TMOP)
    ASMJIT_ARM_FEATURE(has_spe, kSPE)
    ASMJIT_ARM_FEATURE(has_spe1_1, kSPE1_1)
    ASMJIT_ARM_FEATURE(has_spe1_2, kSPE1_2)
    ASMJIT_ARM_FEATURE(has_spe1_3, kSPE1_3)
    ASMJIT_ARM_FEATURE(has_spe1_4, kSPE1_4)
    ASMJIT_ARM_FEATURE(has_spe_altclk, kSPE_ALTCLK)
    ASMJIT_ARM_FEATURE(has_spe_crr, kSPE_CRR)
    ASMJIT_ARM_FEATURE(has_spe_eft, kSPE_EFT)
    ASMJIT_ARM_FEATURE(has_spe_fds, kSPE_FDS)
    ASMJIT_ARM_FEATURE(has_spe_fpf, kSPE_FPF)
    ASMJIT_ARM_FEATURE(has_spe_sme, kSPE_SME)
    ASMJIT_ARM_FEATURE(has_specres, kSPECRES)
    ASMJIT_ARM_FEATURE(has_specres2, kSPECRES2)
    ASMJIT_ARM_FEATURE(has_spmu, kSPMU)
    ASMJIT_ARM_FEATURE(has_ssbs, kSSBS)
    ASMJIT_ARM_FEATURE(has_ssbs2, kSSBS2)
    ASMJIT_ARM_FEATURE(has_ssve_aes, kSSVE_AES)
    ASMJIT_ARM_FEATURE(has_ssve_bitperm, kSSVE_BITPERM)
    ASMJIT_ARM_FEATURE(has_ssve_fexpa, kSSVE_FEXPA)
    ASMJIT_ARM_FEATURE(has_ssve_fp8dot2, kSSVE_FP8DOT2)
    ASMJIT_ARM_FEATURE(has_ssve_fp8dot4, kSSVE_FP8DOT4)
    ASMJIT_ARM_FEATURE(has_ssve_fp8fma, kSSVE_FP8FMA)
    ASMJIT_ARM_FEATURE(has_sve, kSVE)
    ASMJIT_ARM_FEATURE(has_sve2, kSVE2)
    ASMJIT_ARM_FEATURE(has_sve2_1, kSVE2_1)
    ASMJIT_ARM_FEATURE(has_sve2_2, kSVE2_2)
    ASMJIT_ARM_FEATURE(has_sve_aes, kSVE_AES)
    ASMJIT_ARM_FEATURE(has_sve_aes2, kSVE_AES2)
    ASMJIT_ARM_FEATURE(has_sve_b16b16, kSVE_B16B16)
    ASMJIT_ARM_FEATURE(has_sve_bf16, kSVE_BF16)
    ASMJIT_ARM_FEATURE(has_sve_bfscale, kSVE_BFSCALE)
    ASMJIT_ARM_FEATURE(has_sve_bitperm, kSVE_BITPERM)
    ASMJIT_ARM_FEATURE(has_sve_ebf16, kSVE_EBF16)
    ASMJIT_ARM_FEATURE(has_sve_eltperm, kSVE_ELTPERM)
    ASMJIT_ARM_FEATURE(has_sve_f16mm, kSVE_F16MM)
    ASMJIT_ARM_FEATURE(has_sve_f32mm, kSVE_F32MM)
    ASMJIT_ARM_FEATURE(has_sve_f64mm, kSVE_F64MM)
    ASMJIT_ARM_FEATURE(has_sve_i8mm, kSVE_I8MM)
    ASMJIT_ARM_FEATURE(has_sve_pmull128, kSVE_PMULL128)
    ASMJIT_ARM_FEATURE(has_sve_sha3, kSVE_SHA3)
    ASMJIT_ARM_FEATURE(has_sve_sm4, kSVE_SM4)
    ASMJIT_ARM_FEATURE(has_sysinstr128, kSYSINSTR128)
    ASMJIT_ARM_FEATURE(has_sysreg128, kSYSREG128)
    ASMJIT_ARM_FEATURE(has_the, kTHE)
    ASMJIT_ARM_FEATURE(has_tlbios, kTLBIOS)
    ASMJIT_ARM_FEATURE(has_tlbirange, kTLBIRANGE)
    ASMJIT_ARM_FEATURE(has_tlbiw, kTLBIW)
    ASMJIT_ARM_FEATURE(has_tme, kTME)
    ASMJIT_ARM_FEATURE(has_trf, kTRF)
    ASMJIT_ARM_FEATURE(has_uao, kUAO)
    ASMJIT_ARM_FEATURE(has_vfp_d32, kVFP_D32)
    ASMJIT_ARM_FEATURE(has_vhe, kVHE)
    ASMJIT_ARM_FEATURE(has_vmid16, kVMID16)
    ASMJIT_ARM_FEATURE(has_wfxt, kWFXT)
    ASMJIT_ARM_FEATURE(has_xnx, kXNX)
    ASMJIT_ARM_FEATURE(has_xs, kXS)
    #undef ASMJIT_ARM_FEATURE
  };
  static_assert(uint32_t(X86::kMaxValue) < kMaxFeatures, "The number of X86 CPU features cannot exceed CpuFeatures::kMaxFeatures");
  static_assert(uint32_t(ARM::kMaxValue) < kMaxFeatures, "The number of ARM CPU features cannot exceed CpuFeatures::kMaxFeatures");
  Data _data {};
  ASMJIT_INLINE_NODEBUG CpuFeatures() noexcept {}
  ASMJIT_INLINE_NODEBUG CpuFeatures(const CpuFeatures& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG explicit CpuFeatures(const Data& other) noexcept : _data{other._bits} {}
  ASMJIT_INLINE_NODEBUG explicit CpuFeatures(Globals::NoInit_) noexcept {}
  ASMJIT_INLINE_NODEBUG CpuFeatures& operator=(const CpuFeatures& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG bool operator==(const CpuFeatures& other) const noexcept { return  equals(other); }
  ASMJIT_INLINE_NODEBUG bool operator!=(const CpuFeatures& other) const noexcept { return !equals(other); }
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _data.is_empty(); }
  template<typename T = Data>
  ASMJIT_INLINE_NODEBUG T& data() noexcept { return static_cast<T&>(_data); }
  template<typename T = Data>
  ASMJIT_INLINE_NODEBUG const T& data() const noexcept { return static_cast<const T&>(_data); }
  ASMJIT_INLINE_NODEBUG X86& x86() noexcept { return data<X86>(); }
  ASMJIT_INLINE_NODEBUG const X86& x86() const noexcept { return data<X86>(); }
  ASMJIT_INLINE_NODEBUG ARM& arm() noexcept { return data<ARM>(); }
  ASMJIT_INLINE_NODEBUG const ARM& arm() const noexcept { return data<ARM>(); }
  ASMJIT_INLINE_NODEBUG BitWord* bits() noexcept { return _data.bits(); }
  ASMJIT_INLINE_NODEBUG const BitWord* bits() const noexcept { return _data.bits(); }
  ASMJIT_INLINE_NODEBUG size_t bit_word_count() const noexcept { return _data.bit_word_count(); }
  ASMJIT_INLINE_NODEBUG Iterator iterator() const noexcept { return _data.iterator(); }
  template<typename FeatureId>
  ASMJIT_INLINE_NODEBUG bool has(const FeatureId& feature_id) const noexcept { return _data.has(feature_id); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG bool has_any(Args&&... args) const noexcept { return _data.has_any(std::forward<Args>(args)...); }
  ASMJIT_INLINE_NODEBUG bool has_all(const CpuFeatures& other) const noexcept { return _data.has_all(other._data); }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data.reset(); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void add(Args&&... args) noexcept { return _data.add(std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void add_if(bool condition, Args&&... args) noexcept { return _data.add_if(condition, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void remove(Args&&... args) noexcept { return _data.remove(std::forward<Args>(args)...); }
  ASMJIT_INLINE_NODEBUG bool equals(const CpuFeatures& other) const noexcept { return _data.equals(other._data); }
};
enum class CpuHints : uint32_t {
  kNone = 0x0u,
  kVecMaskedOps8 = 0x00000001u,
  kVecMaskedOps16 = 0x00000002u,
  kVecMaskedOps32 = 0x00000004u,
  kVecMaskedOps64 = 0x00000008u,
  kVecFastIntMul32 = 0x00000010u,
  kVecFastIntMul64 = 0x00000020u,
  kVecFastGather = 0x00000040u,
  kVecMaskedStore = 0x00000080u
};
ASMJIT_DEFINE_ENUM_FLAGS(CpuHints)
class CpuInfo {
public:
  Arch _arch {};
  SubArch _sub_arch {};
  bool _was_detected {};
  uint8_t _reserved {};
  uint32_t _family_id {};
  uint32_t _model_id {};
  uint32_t _brand_id {};
  uint32_t _stepping {};
  uint32_t _processor_type {};
  uint32_t _max_logical_processors {};
  uint32_t _cache_line_size {};
  uint32_t _hw_thread_count {};
  FixedString<16> _vendor {};
  FixedString<64> _brand {};
  CpuFeatures _features {};
  CpuHints _hints {};
  ASMJIT_INLINE_NODEBUG CpuInfo() noexcept {}
  ASMJIT_INLINE_NODEBUG CpuInfo(const CpuInfo& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG explicit CpuInfo(Globals::NoInit_) noexcept
    : _features(Globals::NoInit) {};
  [[nodiscard]]
  ASMJIT_API static const CpuInfo& host() noexcept;
  ASMJIT_API static CpuHints recalculate_hints(const CpuInfo& info, const CpuFeatures& features) noexcept;
  ASMJIT_INLINE_NODEBUG CpuInfo& operator=(const CpuInfo& other) noexcept = default;
  ASMJIT_INLINE_NODEBUG void init_arch(Arch arch, SubArch sub_arch = SubArch::kUnknown) noexcept {
    _arch = arch;
    _sub_arch = sub_arch;
  }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = CpuInfo{}; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return _sub_arch; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool was_detected() const noexcept { return _was_detected; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t family_id() const noexcept { return _family_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t model_id() const noexcept { return _model_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t brand_id() const noexcept { return _brand_id; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t stepping() const noexcept { return _stepping; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t processor_type() const noexcept { return _processor_type; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t max_logical_processors() const noexcept { return _max_logical_processors; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t cache_line_size() const noexcept { return _cache_line_size; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t hw_thread_count() const noexcept { return _hw_thread_count; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* vendor() const noexcept { return _vendor.str; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_vendor(const char* s) const noexcept { return _vendor.equals(s); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* brand() const noexcept { return _brand.str; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuFeatures& features() noexcept { return _features; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CpuFeatures& features() const noexcept { return _features; }
  template<typename FeatureId>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_feature(const FeatureId& feature_id) const noexcept { return _features.has(feature_id); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void add_feature(Args&&... args) noexcept { return _features.add(std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG void remove_feature(Args&&... args) noexcept { return _features.remove(std::forward<Args>(args)...); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CpuHints hints() const noexcept { return _hints; }
  ASMJIT_INLINE void update_hints() noexcept { _hints = recalculate_hints(*this, _features); }
};
ASMJIT_END_NAMESPACE
#endif