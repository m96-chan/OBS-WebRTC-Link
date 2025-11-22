# GitHub Actions Workflows

This directory contains CI/CD workflows for the OBS-WebRTC-Link project.

## Workflows

### build.yml - Main Build Workflow

Automatically builds and tests the project on multiple platforms.

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop` branches
- Manual workflow dispatch

**Platforms:**
- Ubuntu Latest (Linux)
- Windows Latest
- macOS Latest

**Build Matrix:**
| Platform | Compiler | CMake Generator |
|----------|----------|-----------------|
| Ubuntu   | GCC      | Ninja           |
| Windows  | MSVC     | Visual Studio   |
| macOS    | Clang    | Ninja           |

### Current Status

The workflow currently validates:
- ✅ Git submodule initialization
- ✅ Dependency compilation (libdatachannel, nlohmann-json)
- ✅ Cross-platform CMake configuration
- ⏳ Full OBS plugin build (requires OBS Studio SDK)

### Why Dependencies Only?

The full OBS plugin build requires the OBS Studio SDK, which is not included in this repository. The current CI/CD pipeline focuses on:

1. **Submodule Validation**: Ensures Git submodules are properly initialized
2. **Dependency Compilation**: Verifies libdatachannel and nlohmann-json build successfully
3. **CMake Configuration**: Tests CMake setup on all platforms
4. **Build Logs**: Captures detailed logs for troubleshooting

### Future Enhancements

When OBS Studio SDK integration is added:
- Full plugin compilation
- Unit test execution
- Integration test execution
- Code coverage reporting
- Static analysis (clang-tidy, cppcheck)
- Build artifact archiving

### Build Artifacts

Each workflow run uploads build logs as artifacts:
- `linux-build-logs` - CMake output and error logs (Ubuntu)
- `windows-build-logs` - CMake output and error logs (Windows)
- `macos-build-logs` - CMake output and error logs (macOS)

Artifacts are retained for 90 days and can be downloaded from the Actions tab.

### Viewing Build Status

Build status badges can be added to README.md:

```markdown
![Build Status](https://github.com/m96-chan/OBS-WebRTC-Link/workflows/Build/badge.svg)
```

### Running Workflows Manually

1. Go to the **Actions** tab on GitHub
2. Select the **Build** workflow
3. Click **Run workflow**
4. Select the branch and click **Run workflow**

### Troubleshooting

#### Build Failures

If the build fails, check the workflow logs:

1. Go to the **Actions** tab
2. Click on the failed workflow run
3. Expand the failed job and step
4. Download build log artifacts for detailed information

#### Common Issues

**Submodules not initialized:**
```yaml
- uses: actions/checkout@v4
  with:
    submodules: recursive  # ← Make sure this is present
```

**CMake configuration fails:**
- Check that all dependencies are installed
- Verify CMakeLists.txt syntax
- Review CMakeError.log in artifacts

**Dependency build fails:**
- Check libdatachannel compatibility
- Verify nlohmann-json version
- Review compiler error messages

### Adding New Jobs

To add a new job to the workflow:

```yaml
jobs:
  my-new-job:
    name: My New Job
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          submodules: recursive
      - name: My step
        run: echo "Hello!"
```

### Cache Configuration

To speed up builds, consider adding caching:

```yaml
- name: Cache CMake build
  uses: actions/cache@v4
  with:
    path: build
    key: ${{ runner.os }}-cmake-${{ hashFiles('**/CMakeLists.txt') }}
```

### Matrix Builds

The workflow uses implicit matrix builds across different OS platforms. To add explicit matrix configurations:

```yaml
jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        build_type: [Debug, Release]
    runs-on: ${{ matrix.os }}
```

### Security Considerations

- Workflows run in isolated environments
- Secrets are not exposed in logs
- Pull requests from forks have restricted permissions
- Use `continue-on-error: true` cautiously

### References

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Events that trigger workflows](https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows)
- [Context and expression syntax](https://docs.github.com/en/actions/learn-github-actions/contexts)
