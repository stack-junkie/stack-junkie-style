# Application delivery preferences

Apply these defaults when a task includes a maintained application repository or its documentation. They do not authorize implementation, publication, or release work during a planning-only request.

## README

Keep the README concise, with a short purpose statement and two clear sections: Setup and How to use. Setup should let someone with a clean clone identify prerequisites, build, and run the app without local knowledge. Separate installing a release from building source when both routes exist.

A command code block contains executable commands only. Keep explanations, headings, shell prompt markers, expected output, and commentary outside it. State the shell and working directory in the surrounding prose. Test the documented commands from a clean checkout before claiming setup is reproducible.

Use fresh screenshots of the actual app in How to use, paired with short instructions and useful alternative text. Refresh screenshots when the shown workflow changes. Mockups and stale images must not stand in for the released interface. Link longer troubleshooting, contributor, and release details instead of expanding the README into a manual.

## Versions and repository conventions

Inspect the current repository and the owner's named reference project before choosing conventions. For a new maintained utility without its own rules, prefer semantic versions from one authoritative source, synchronized application/About/package metadata, and a dated Keep a Changelog style history describing user-visible changes. Include relevant feature, fix, documentation, and packaging changes; record only checks actually performed.

Use matching vX.Y.Z tags for approved releases. Keep source, package metadata, changelog, and release assets consistent. Follow existing commit and review conventions. Do not create alpha entries or bump a product version for each prompt or planning edit.

Prefer small, transparent build/version commands and proportionate CI. Do not automatically add auto-staging hooks, policy engines, governance documents, or elaborate release machinery. Keep user data, credentials, local settings, and generated build output out of the repository. Preserve unrelated changes. Verify the installable app and obtain the owner's release approval before public publication when that is the project's rule.