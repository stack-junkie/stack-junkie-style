# App repo delivery

Defaults for an app repository's README, versions and releases. They apply when the task touches those; a planning-only request authorizes none of this work. The repo's own conventions and any reference project the user names come first.

## README

- A short purpose statement, then two sections: Setup and How to use. Setup takes someone with a clean clone through prerequisites, build and run. Separate installing a release from building source when both exist.
- A command code block holds executable commands only. Explanations, prompts and expected output go in the prose around it, which names the shell and working directory. Run the documented commands from a clean checkout before calling setup reproducible.
- How to use shows fresh screenshots of the real app with short instructions and alt text. Refresh them when the workflow changes. Mockups and stale images never stand in for the released app.
- Link troubleshooting, contributor and release detail instead of growing the README into a manual. No emojis.

## Versions and releases

- One version file is the only source. App, About and package metadata read or sync from it; the version is never hardcoded.
- For a new app without its own rules: semantic versions, matching `vX.Y.Z` tags for approved releases, and a dated Keep a Changelog history of user-visible changes. Record only checks actually performed.
- Bump the version for a delivered release, not for each prompt or planning edit. Ask before choosing a version that was not specified.
- Keep build and version commands small and CI proportionate. Hooks, policy engines and release machinery come only when the user asks.
- Keep user data, credentials, local settings and build output out of the repo. Get the user's approval before any public publication.
