# Contributing to OBS-WebRTC-Link

Thank you for considering contributing to OBS-WebRTC-Link!

## Code Style and Formatting

This project uses automated code formatting to maintain consistency.

### C++ Code Formatting

We use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) for C++ code formatting.

#### Installation

**Ubuntu/Debian:**
```bash
sudo apt-get install clang-format
```

**macOS:**
```bash
brew install clang-format
```

**Windows:**
Download from the [LLVM website](https://releases.llvm.org/).

#### Usage

**Format all C++ files:**
```bash
./scripts/format-code.sh
```

**Check formatting without modifying files:**
```bash
./scripts/check-format.sh
```

**Show diffs for files that need formatting:**
```bash
SHOW_DIFF=1 ./scripts/check-format.sh
```

### CMake Formatting

We use [cmake-format](https://cmake-format.readthedocs.io/) for CMakeLists.txt files.

#### Installation

```bash
pip install cmake-format
```

#### Usage

```bash
cmake-format -i CMakeLists.txt
cmake-format -i tests/unit/CMakeLists.txt
```

### Editor Integration

#### VS Code

Install the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) and add to `.vscode/settings.json`:

```json
{
  "C_Cpp.clang_format_style": "file",
  "editor.formatOnSave": true
}
```

#### CLion / IntelliJ IDEA

1. Go to **Settings** → **Editor** → **Code Style** → **C/C++**
2. Select **Set from...** → **Predefined Style** → **Custom**
3. Check **Enable ClangFormat**
4. Select **Use .clang-format file**

#### Vim / Neovim

Add to your `.vimrc` or `init.vim`:

```vim
" Format on save
autocmd BufWritePre *.cpp,*.hpp,*.h,*.c :silent! !clang-format -i %
```

Or use a plugin like [vim-clang-format](https://github.com/rhysd/vim-clang-format).

#### Emacs

Add to your `.emacs` or `init.el`:

```elisp
(require 'clang-format)
(global-set-key (kbd "C-c C-f") 'clang-format-region)
(add-hook 'c++-mode-hook
          (lambda () (add-hook 'before-save-hook 'clang-format-buffer nil 'local)))
```

### EditorConfig

This project includes an [`.editorconfig`](.editorconfig) file that defines basic formatting rules (indentation, line endings, etc.) for various file types.

Most modern editors support EditorConfig automatically or via plugins:
- VS Code: Install [EditorConfig extension](https://marketplace.visualstudio.com/items?itemName=EditorConfig.EditorConfig)
- JetBrains IDEs: Built-in support
- Vim/Neovim: Install [editorconfig-vim](https://github.com/editorconfig/editorconfig-vim)

## Continuous Integration

All pull requests are automatically checked for:
- ✅ Code formatting (clang-format)
- ✅ Build success on Linux, Windows, and macOS
- ✅ Unit tests passing

If formatting checks fail, the CI will provide instructions on how to fix them.

## Pre-commit Hooks (Optional)

You can set up pre-commit hooks to automatically check formatting before committing:

```bash
# Create a pre-commit hook
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
./scripts/check-format.sh
EOF

chmod +x .git/hooks/pre-commit
```

## Development Workflow

1. **Fork and clone** the repository
2. **Create a feature branch**: `git checkout -b feature/my-feature`
3. **Make your changes** following the code style
4. **Format your code**: `./scripts/format-code.sh`
5. **Run tests**: Follow instructions in the README
6. **Commit your changes**: Use clear, descriptive commit messages
7. **Push and create a Pull Request**

## Documentation Contributions

### Adding Screenshots and Images

When contributing screenshots or images for documentation:

#### Image Guidelines

1. **Format**
   - **Preferred**: PNG for UI screenshots (lossless)
   - **Alternative**: WebP for smaller file sizes
   - **Avoid**: JPEG for UI screenshots (compression artifacts)

2. **Size and Optimization**
   - Maximum width: 1920px
   - Recommended width for UI screenshots: 800-1200px
   - Compress images to keep file sizes under 500KB
   - Use appropriate tools:
     - [pngquant](https://pngquant.org/): Lossy PNG compression
     - [optipng](http://optipng.sourceforge.net/): Lossless PNG optimization
     - [ImageOptim](https://imageoptim.com/) (macOS): All-in-one optimizer
     - [squoosh.app](https://squoosh.app/): Web-based image optimizer

3. **Naming Convention**
   - Use lowercase with hyphens: `my-screenshot.png`
   - Include context in filename: `obs-source-settings-whep.png`
   - Avoid spaces and special characters

4. **Privacy**
   - Remove or blur any sensitive information (tokens, API keys, personal URLs)
   - Use example/placeholder data where possible
   - Example tokens: `eyJhbGc...` (truncated)
   - Example URLs: `https://example.livekit.cloud`

5. **Directory Structure**
   ```
   docs/images/
   ├── screenshots/    # UI screenshots
   ├── diagrams/       # Architecture and workflow diagrams
   └── examples/       # Example use case images
   ```

#### Screenshot Checklist

Before submitting screenshots:
- [ ] Image is optimized and under 500KB
- [ ] Sensitive information is removed/blurred
- [ ] Filename follows naming convention
- [ ] Placed in correct subdirectory
- [ ] Referenced in documentation with descriptive alt text

#### Example Image Reference

```markdown
![WHEP Source Settings](docs/images/screenshots/obs-source-settings-whep.png)
```

For more details, see [docs/images/README.md](images/README.md).

## Pull Request Guidelines

- Ensure all CI checks pass (build, tests, formatting)
- Write clear PR descriptions
- Reference related issues (e.g., "Closes #123")
- Keep PRs focused on a single feature or fix
- Add tests for new features
- For documentation PRs: Ensure images are optimized and follow guidelines

## Questions?

If you have any questions, feel free to:
- Open an issue on GitHub
- Check the [README](../README.md) for general information
- Review existing issues and pull requests

Thank you for contributing!
