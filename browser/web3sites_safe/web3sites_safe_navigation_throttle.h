#pragma once

#include <string>
#include <memory>

#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/navigation_throttle.h"
#include "mises/browser/web3sites_safe/web3sites_safe_blocking_page.h"
#include "mises/browser/web3sites_safe/web3sites_safe_util.h"


namespace content {
class NavigationHandle;
class BrowserContext;
}

namespace network {
class SharedURLLoaderFactory;
class SimpleURLLoader;
}

class Profile;


struct  DomainInfo;

class Web3sitesSafeNavigationThrottle : public content::NavigationThrottle {
  public:
   explicit Web3sitesSafeNavigationThrottle(content::NavigationHandle* handle);
   ~Web3sitesSafeNavigationThrottle() override;

   // content::NavigationThrottle:
  ThrottleCheckResult WillStartRequest() override;

  ThrottleCheckResult WillProcessResponse() override;

  ThrottleCheckResult WillFailRequest() override;

  const char* GetNameForLogging() override;

  void OnApiCheckUrlResult(const MisesURLCheckResult&);

  ThrottleCheckResult OnLocalCheckUrlResult(const MisesURLCheckResult&);

  static std::unique_ptr<Web3sitesSafeNavigationThrottle>
  MaybeCreateNavigationThrottle(content::NavigationHandle* handle);

  private:

  void PerformChecksDeferred  ();

  ThrottleCheckResult PerformChecks  ();

  MisesURLCheckResult LocalCheckUrl(const GURL& url);

  ThrottleCheckResult ShowInterstitial(const GURL& safe_domain,
                                       const GURL& lookalike_domain,
                                       ukm::SourceId source_id,
                                       Web3sitesResultType::Type result_type,
                                       bool triggered_by_initial_url);

  raw_ptr<Profile> profile_;

  base::WeakPtrFactory<Web3sitesSafeNavigationThrottle> weak_ptr_factory_{
      this};


};
