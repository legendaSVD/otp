#ifndef ASMJIT_CORE_ENVIRONMENT_H_INCLUDED
#define ASMJIT_CORE_ENVIRONMENT_H_INCLUDED
#include <asmjit/core/archtraits.h>
#if defined(__APPLE__)
  #include <TargetConditionals.h>
#endif
ASMJIT_BEGIN_NAMESPACE
enum class Vendor : uint8_t {
  kUnknown = 0,
  kMaxValue = kUnknown,
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#else
    kUnknown
#endif
};
enum class Platform : uint8_t {
  kUnknown = 0,
  kWindows,
  kOther,
  kLinux,
  kHurd,
  kFreeBSD,
  kOpenBSD,
  kNetBSD,
  kDragonFlyBSD,
  kHaiku,
  kOSX,
  kIOS,
  kTVOS,
  kWatchOS,
  kEmscripten,
  kMaxValue = kEmscripten,
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#elif defined(__EMSCRIPTEN__)
    kEmscripten
#elif defined(_WIN32)
    kWindows
#elif defined(__linux__)
    kLinux
#elif defined(__gnu_hurd__)
    kHurd
#elif defined(__FreeBSD__)
    kFreeBSD
#elif defined(__OpenBSD__)
    kOpenBSD
#elif defined(__NetBSD__)
    kNetBSD
#elif defined(__DragonFly__)
    kDragonFlyBSD
#elif defined(__HAIKU__)
    kHaiku
#elif defined(__APPLE__) && TARGET_OS_OSX
    kOSX
#elif defined(__APPLE__) && TARGET_OS_TV
    kTVOS
#elif defined(__APPLE__) && TARGET_OS_WATCH
    kWatchOS
#elif defined(__APPLE__) && TARGET_OS_IPHONE
    kIOS
#else
    kOther
#endif
};
enum class PlatformABI : uint8_t {
  kUnknown = 0,
  kMSVC,
  kGNU,
  kAndroid,
  kCygwin,
  kDarwin,
  kMaxValue,
  kHost =
#if defined(_DOXYGEN)
    DETECTED_AT_COMPILE_TIME
#elif defined(_MSC_VER)
    kMSVC
#elif defined(__CYGWIN__)
    kCygwin
#elif defined(__MINGW32__) || defined(__GLIBC__)
    kGNU
#elif defined(__ANDROID__)
    kAndroid
#elif defined(__APPLE__)
    kDarwin
#else
    kUnknown
#endif
};
enum class FloatABI : uint8_t {
  kHardFloat = 0,
  kSoftFloat,
  kHost =
#if ASMJIT_ARCH_ARM == 32 && defined(__SOFTFP__)
  kSoftFloat
#else
  kHardFloat
#endif
};
enum class ObjectFormat : uint8_t {
  kUnknown = 0,
  kJIT,
  kELF,
  kCOFF,
  kXCOFF,
  kMachO,
  kMaxValue
};
class Environment {
public:
  Arch _arch = Arch::kUnknown;
  SubArch _sub_arch = SubArch::kUnknown;
  Vendor _vendor = Vendor::kUnknown;
  Platform _platform = Platform::kUnknown;
  PlatformABI _platform_abi = PlatformABI::kUnknown;
  ObjectFormat _object_format = ObjectFormat::kUnknown;
  FloatABI _float_abi = FloatABI::kHardFloat;
  uint8_t _reserved = 0;
  ASMJIT_INLINE_CONSTEXPR Environment() noexcept = default;
  ASMJIT_INLINE_CONSTEXPR Environment(const Environment& other) noexcept = default;
  ASMJIT_INLINE_CONSTEXPR explicit Environment(
    Arch arch,
    SubArch sub_arch = SubArch::kUnknown,
    Vendor vendor = Vendor::kUnknown,
    Platform platform = Platform::kUnknown,
    PlatformABI platform_abi = PlatformABI::kUnknown,
    ObjectFormat object_format = ObjectFormat::kUnknown,
    FloatABI float_abi = FloatABI::kHardFloat) noexcept
    : _arch(arch),
      _sub_arch(sub_arch),
      _vendor(vendor),
      _platform(platform),
      _platform_abi(platform_abi),
      _object_format(object_format),
      _float_abi(float_abi) {}
  static ASMJIT_INLINE_CONSTEXPR Environment host() noexcept {
    return Environment(Arch::kHost, SubArch::kHost, Vendor::kHost, Platform::kHost, PlatformABI::kHost, ObjectFormat::kUnknown, FloatABI::kHost);
  }
  ASMJIT_INLINE_NODEBUG Environment& operator=(const Environment& other) noexcept = default;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const Environment& other) const noexcept { return equals(other); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const Environment& other) const noexcept { return !equals(other); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept {
    return _packed() == 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept {
    return _arch != Arch::kUnknown;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t _packed() const noexcept {
    uint64_t x;
    memcpy(&x, this, 8);
    return x;
  }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Environment{}; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool equals(const Environment& other) const noexcept { return _packed() == other._packed(); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return _sub_arch; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Vendor vendor() const noexcept { return _vendor; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Platform platform() const noexcept { return _platform; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG PlatformABI platform_abi() const noexcept { return _platform_abi; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ObjectFormat object_format() const noexcept { return _object_format; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FloatABI float_abi() const noexcept { return _float_abi; }
  inline void init(
    Arch arch,
    SubArch sub_arch = SubArch::kUnknown,
    Vendor vendor = Vendor::kUnknown,
    Platform platform = Platform::kUnknown,
    PlatformABI platform_abi = PlatformABI::kUnknown,
    ObjectFormat object_format = ObjectFormat::kUnknown,
    FloatABI float_abi = FloatABI::kHardFloat) noexcept {
    _arch = arch;
    _sub_arch = sub_arch;
    _vendor = vendor;
    _platform = platform;
    _platform_abi = platform_abi;
    _object_format = object_format;
    _float_abi = float_abi;
    _reserved = 0;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_x86() const noexcept { return _arch == Arch::kX86; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_x64() const noexcept { return _arch == Arch::kX64; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_arm() const noexcept { return is_arch_arm(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_thumb() const noexcept { return is_arch_thumb(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_aarch64() const noexcept { return is_arch_aarch64(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_mips32() const noexcept { return is_arch_mips32(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_mips64() const noexcept { return is_arch_mips64(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_riscv32() const noexcept { return _arch == Arch::kRISCV32; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_arch_riscv64() const noexcept { return _arch == Arch::kRISCV64; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_32bit() const noexcept { return is_32bit(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_64bit() const noexcept { return is_64bit(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_little_endian() const noexcept { return is_little_endian(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_big_endian() const noexcept { return is_big_endian(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_x86() const noexcept { return is_family_x86(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_arm() const noexcept { return is_family_arm(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_aarch32() const noexcept { return is_family_aarch32(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_aarch64() const noexcept { return is_family_aarch64(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_mips() const noexcept { return is_family_mips(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_family_riscv() const noexcept { return is_family_riscv(_arch); }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_windows() const noexcept { return _platform == Platform::kWindows; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_linux() const noexcept { return _platform == Platform::kLinux; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_hurd() const noexcept { return _platform == Platform::kHurd; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_haiku() const noexcept { return _platform == Platform::kHaiku; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_bsd() const noexcept {
    return _platform == Platform::kFreeBSD ||
           _platform == Platform::kOpenBSD ||
           _platform == Platform::kNetBSD ||
           _platform == Platform::kDragonFlyBSD;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_platform_apple() const noexcept {
    return _platform == Platform::kOSX ||
           _platform == Platform::kIOS ||
           _platform == Platform::kTVOS ||
           _platform == Platform::kWatchOS;
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_msvc_abi() const noexcept { return _platform_abi == PlatformABI::kMSVC; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_gnu_abi() const noexcept { return _platform_abi == PlatformABI::kGNU; }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_darwin_abi() const noexcept { return _platform_abi == PlatformABI::kDarwin; }
  [[nodiscard]]
  ASMJIT_API uint32_t stack_alignment() const noexcept;
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t register_size() const noexcept { return reg_size_of_arch(_arch); }
  ASMJIT_INLINE_NODEBUG void set_arch(Arch arch) noexcept { _arch = arch; }
  ASMJIT_INLINE_NODEBUG void set_sub_arch(SubArch sub_arch) noexcept { _sub_arch = sub_arch; }
  ASMJIT_INLINE_NODEBUG void set_vendor(Vendor vendor) noexcept { _vendor = vendor; }
  ASMJIT_INLINE_NODEBUG void set_platform(Platform platform) noexcept { _platform = platform; }
  ASMJIT_INLINE_NODEBUG void set_platform_abi(PlatformABI platform_abi) noexcept { _platform_abi = platform_abi; }
  ASMJIT_INLINE_NODEBUG void set_object_format(ObjectFormat object_format) noexcept { _object_format = object_format; }
  ASMJIT_INLINE_NODEBUG void set_float_abi(FloatABI float_abi) noexcept { _float_abi = float_abi; }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_defined_arch(Arch arch) noexcept {
    return uint32_t(arch) <= uint32_t(Arch::kMaxValue);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_valid_arch(Arch arch) noexcept {
    return arch != Arch::kUnknown && uint32_t(arch) <= uint32_t(Arch::kMaxValue);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_32bit(Arch arch) noexcept {
    return (uint32_t(arch) & uint32_t(Arch::k32BitMask)) == uint32_t(Arch::k32BitMask);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_64bit(Arch arch) noexcept {
    return (uint32_t(arch) & uint32_t(Arch::k32BitMask)) == 0;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_little_endian(Arch arch) noexcept {
    return uint32_t(arch) < uint32_t(Arch::kBigEndian);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_big_endian(Arch arch) noexcept {
    return uint32_t(arch) >= uint32_t(Arch::kBigEndian);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_thumb(Arch arch) noexcept {
    return arch == Arch::kThumb || arch == Arch::kThumb_BE;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_arm(Arch arch) noexcept {
    return arch == Arch::kARM || arch == Arch::kARM_BE;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_aarch64(Arch arch) noexcept {
    return arch == Arch::kAArch64 || arch == Arch::kAArch64_BE;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_mips32(Arch arch) noexcept {
    return arch == Arch::kMIPS32_LE || arch == Arch::kMIPS32_BE;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_arch_mips64(Arch arch) noexcept {
    return arch == Arch::kMIPS64_LE || arch == Arch::kMIPS64_BE;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_x86(Arch arch) noexcept {
    return arch == Arch::kX86 || arch == Arch::kX64;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_aarch32(Arch arch) noexcept {
    return is_arch_arm(arch) || is_arch_thumb(arch);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_aarch64(Arch arch) noexcept {
    return is_arch_aarch64(arch);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_arm(Arch arch) noexcept {
    return is_family_aarch32(arch) || is_family_aarch64(arch);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_mips(Arch arch) noexcept {
    return is_arch_mips32(arch) || is_arch_mips64(arch);
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_family_riscv(Arch arch) noexcept {
    return arch == Arch::kRISCV32 || arch == Arch::kRISCV64;
  }
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG uint32_t reg_size_of_arch(Arch arch) noexcept {
    return is_32bit(arch) ? 4u : 8u;
  }
};
static_assert(sizeof(Environment) == 8,
              "Environment must occupy exactly 8 bytes.");
ASMJIT_END_NAMESPACE
#endif