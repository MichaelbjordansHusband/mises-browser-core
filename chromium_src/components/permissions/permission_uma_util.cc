#include "components/permissions/request_type.h"

#include "components/permissions/permission_uma_util.h"

#include "components/permissions/permissions_client.h"



// We do not record permissions UKM and this can save us from patching
// in RecordPermissionAction for unhandling switch cases for Brave's content
// settings type.
#define GetUkmSourceId             \
  GetSettingsMap(browser_context); \
  if (true)                        \
    return;                        \
  PermissionsClient::Get()->GetUkmSourceId
  
#if BUILDFLAG(IS_ANDROID)
#define MISES_GET_UMA_VALUE_FOR_REQUEST_TYPE      \
  case RequestType::kCameraPanTiltZoom:\
      return RequestTypeForUma::PERMISSION_CAMERA_PAN_TILT_ZOOM;\
  case RequestType::kLocalFonts: \
      return RequestTypeForUma::PERMISSION_LOCAL_FONTS; \
  case RequestType::kRegisterProtocolHandler: \
    return RequestTypeForUma::REGISTER_PROTOCOL_HANDLER; \
  case RequestType::kSecurityAttestation: \
    return RequestTypeForUma::PERMISSION_SECURITY_KEY_ATTESTATION; \
  case RequestType::kU2fApiRequest: \
    return RequestTypeForUma::PERMISSION_U2F_API_REQUEST; \
  case RequestType::kWindowManagement: \
    return RequestTypeForUma::PERMISSION_WINDOW_MANAGEMENT; \
  case RequestType::kWidevine:                    \
  case RequestType::kMisesEthereum:               \
  case RequestType::kMisesSolana:                 \
    return RequestTypeForUma::PERMISSION_VR;
  

#include "src/components/permissions/permission_uma_util.cc"


#else

// Since we don't do UMA just reuse an existing UMA type instead of adding one.
#define MISES_GET_UMA_VALUE_FOR_REQUEST_TYPE      \
  case RequestType::kWidevine:                    \
  case RequestType::kMisesEthereum:               \
  case RequestType::kMisesSolana:                 \
    return RequestTypeForUma::PERMISSION_VR;

#include "src/components/permissions/permission_uma_util.cc"

#endif

#undef MISES_GET_UMA_VALUE_FOR_REQUEST_TYPE
