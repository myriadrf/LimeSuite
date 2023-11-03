#ifndef LMS7_VALIDATION
#define LMS7_VALIDATION

namespace lime {

// Checks if config parameters are within expected values and can produce working state
bool LMS7002M_Validate(const SDRDevice::SDRConfig& config, std::vector<std::string>& out_errors, const int lms7002_chCount = 2);

} // namespace lime

#endif // LMS7_VALIDATION
