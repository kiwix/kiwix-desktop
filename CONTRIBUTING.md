# Contributing to Kiwix Desktop

Thank you for your interest in contributing to Kiwix Desktop! Kiwix is an open-source project dedicated to making free knowledge available offline to millions of people around the world.

---

## 1. Getting Started

### Architecture & Codebase Guide
Before diving into code changes, please read the **[Kiwix Desktop Architecture Guide](docs/ARCHITECTURE.md)**. It provides:
* An overview of the Kiwix ecosystem (`libzim`, `libkiwix`, `kiwix-desktop`).
* Step-by-step application startup and tab lifecycle.
* How the `zim://` protocol handler bridges `libzim` with QtWebEngine.
* How to debug HTML/CSS/JS rendering using Chromium DevTools.
* A "Where to Start When..." lookup table mapping features to source files.

### Building from Source
For instructions on installing dependencies and compiling Kiwix Desktop on Linux or Windows, see the [Compilation section in README.md](README.md#compilation).

---

## 2. Ways to Contribute

* **Report Bugs:** Search [existing issues](https://github.com/kiwix/kiwix-desktop/issues) first. If your issue is new, open a detailed bug report including your operating system, Qt version, Kiwix Desktop version, and steps to reproduce.
* **Propose Features:** Open an issue describing the feature, its motivation, and potential user experience.
* **Fix Bugs & Implement Features:** Browse [good first issues](https://github.com/kiwix/kiwix-desktop/labels/good%20first%20issue) or [help wanted](https://github.com/kiwix/kiwix-desktop/labels/help%20wanted) tickets.
* **Translate:** Translation of Kiwix is managed via [translatewiki.net](https://translatewiki.net/wiki/Translating:Kiwix). Please contribute translations directly there rather than submitting PRs for translation files.

---

## 3. Pull Request Guidelines

1. **Fork and Branch:**
   * Fork the repository to your GitHub account.
   * Create a descriptive topic branch off `main`:
     ```bash
     git checkout -b fix-1234-description
     ```
2. **Coding Standards:**
   * Kiwix Desktop is written in modern C++ and Qt6.
   * Maintain consistent code style with existing code in `src/`.
   * Keep classes and functions modular and focused.
   * Ensure user-facing strings are wrapped with `gt(...)` or Qt translation macros for localization.
3. **Commit Messages:**
   * Write clear, concise commit messages explaining *why* the change was made.
   * Reference relevant issue numbers (e.g., `Fix #1234: prevent crash on tab close`).
4. **Submitting your PR:**
   * Push your branch to your fork and submit a Pull Request against `main`.
   * Describe the problem solved and include steps for testing.
   * Verify that the continuous integration (CI) tests pass.

---

## 4. Communication Channels

Feel free to join the conversation and ask questions:
* **Web Chat:** [chat.kiwix.org](https://chat.kiwix.org)
* **Slack:** `#android` / desktop channels ([Get an invite](https://slack-invite.kiwix.org/))
* **IRC:** `#kiwix` on `ircs://irc.libera.chat:6697`
* **Mailing List:** [kiwix-developer@lists.sourceforge.net](mailto:kiwix-developer@lists.sourceforge.net)
