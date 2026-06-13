/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const AI_CONTRACT = "@mozilla.org/aiengine/service;1";

export class AIEngineParent extends JSWindowActorParent {
  receiveMessage(msg) {
    switch (msg.name) {
      case "AI:AnalyzeTab": {
        try {
          const svc = Cc[AI_CONTRACT].getService(Ci.nsIAIEngineService);
          const { pageContent, prompt } = msg.data;
          const result = svc.analyzeTab(pageContent, prompt || "Summarize this page.");
          this.sendAsyncMessage("AI:TabResult", { result });
        } catch (e) {
          console.warn("AIEngineParent: analyzeTab failed", e);
          this.sendAsyncMessage("AI:TabResult", { error: e.message });
        }
        break;
      }
      case "AI:CrossPage": {
        try {
          const svc = Cc[AI_CONTRACT].getService(Ci.nsIAIEngineService);
          const { pages, prompt } = msg.data;
          const result = svc.crossPage(pages, prompt || "Compare these pages.");
          this.sendAsyncMessage("AI:CrossResult", { result });
        } catch (e) {
          this.sendAsyncMessage("AI:CrossResult", { error: e.message });
        }
        break;
      }
    }
  }
}
