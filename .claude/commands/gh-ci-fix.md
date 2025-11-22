---
description: GitHub CIをすべてPASSさせる
---

Keep retrying the GitHub Actions workflow until all CI checks pass.
Use `gh run list` to get the latest run status and loop until its conclusion is `success`.
If the `gh` command is not available, stop and report an error.