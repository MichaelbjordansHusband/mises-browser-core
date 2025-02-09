/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mises/renderer/mises_content_renderer_client.h"

#include "base/feature_list.h"
#include "mises/components/brave_wallet/common/features.h"
#include "mises/renderer/brave_wallet/brave_wallet_render_frame_observer.h"
#include "mises/components/safe_builtins/renderer/safe_builtins.h"
#include "chrome/common/chrome_isolated_world_ids.h"
#include "chrome/renderer/chrome_render_thread_observer.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/platform/web_runtime_features.h"
#include "third_party/blink/public/web/modules/service_worker/web_service_worker_context_proxy.h"
#include "third_party/blink/public/web/web_script_controller.h"
#include "url/gurl.h"


MisesContentRendererClient::MisesContentRendererClient() = default;

void MisesContentRendererClient::
    SetRuntimeFeaturesDefaultsBeforeBlinkInitialization() {
  ChromeContentRendererClient::
      SetRuntimeFeaturesDefaultsBeforeBlinkInitialization();

  // blink::WebRuntimeFeatures::EnableWebNFC(false);

  // // These features don't have dedicated WebRuntimeFeatures wrappers.
  // blink::WebRuntimeFeatures::EnableFeatureFromString("DigitalGoods", false);
  // if (!base::FeatureList::IsEnabled(blink::features::kFileSystemAccessAPI)) {
  //   blink::WebRuntimeFeatures::EnableFeatureFromString("FileSystemAccessLocal",
  //                                                      false);
  //   blink::WebRuntimeFeatures::EnableFeatureFromString(
  //       "FileSystemAccessAPIExperimental", false);
  // }
  // blink::WebRuntimeFeatures::EnableFeatureFromString("Serial", false);
  // blink::WebRuntimeFeatures::EnableFeatureFromString(
  //     "SpeculationRulesPrefetchProxy", false);
  // blink::WebRuntimeFeatures::EnableFeatureFromString("AdTagging", false);
}

MisesContentRendererClient::~MisesContentRendererClient() = default;

void MisesContentRendererClient::RenderThreadStarted() {
  ChromeContentRendererClient::RenderThreadStarted();
  blink::WebScriptController::RegisterExtension(
    mises::SafeBuiltins::CreateV8Extension());
}

void MisesContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  ChromeContentRendererClient::RenderFrameCreated(render_frame);


  if (base::FeatureList::IsEnabled(
          brave_wallet::features::kNativeBraveWalletFeature)) {
    new brave_wallet::BraveWalletRenderFrameObserver(
        render_frame);
  }

}

void MisesContentRendererClient::RunScriptsAtDocumentStart(
    content::RenderFrame* render_frame) {

  ChromeContentRendererClient::RunScriptsAtDocumentStart(render_frame);
}

void MisesContentRendererClient::WillEvaluateServiceWorkerOnWorkerThread(
    blink::WebServiceWorkerContextProxy* context_proxy,
    v8::Local<v8::Context> v8_context,
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url) {
  ChromeContentRendererClient::WillEvaluateServiceWorkerOnWorkerThread(
      context_proxy, v8_context, service_worker_version_id,
      service_worker_scope, script_url);
}

void MisesContentRendererClient::WillDestroyServiceWorkerContextOnWorkerThread(
    v8::Local<v8::Context> v8_context,
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url) {
  ChromeContentRendererClient::WillDestroyServiceWorkerContextOnWorkerThread(
      v8_context, service_worker_version_id, service_worker_scope, script_url);
}
