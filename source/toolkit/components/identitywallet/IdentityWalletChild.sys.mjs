/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export class IdentityWalletChild extends JSWindowActorChild {
  handleEvent(event) {
    if (event.type === "DOMContentLoaded") {
      this._injectLoginButton();
    }
  }

  _injectLoginButton() {
    const doc = this.document;
    if (!doc || !doc.body) return;

    // Find password fields to inject login button near
    const passwordFields = doc.querySelectorAll('input[type="password"]');
    if (passwordFields.length === 0) return;

    // Check if we have an active login for this site
    const url = doc.location.href;
    if (!url.startsWith("http")) return;

    passwordFields.forEach(field => {
      // Only inject once per form
      const form = field.closest("form");
      if (!form || form.querySelector(".doda-login-btn")) return;

      const btn = doc.createElement("button");
      btn.className = "doda-login-btn";
      btn.textContent = "Passwordless Login";
      btn.style.cssText =
        "margin:4px 0;padding:6px 12px;background:#16c79a;color:#fff;" +
        "border:none;border-radius:4px;cursor:pointer;font-size:12px;font-weight:600;";

      btn.addEventListener("click", () => {
        // Ask parent for active login
        this.sendAsyncMessage("ID:GetActiveLogin", { site: doc.location.hostname });
      });

      form.insertBefore(btn, form.firstChild);
    });
  }
}
