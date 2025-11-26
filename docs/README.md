# Building the QuectelEC200U Documentation

This directory contains the Sphinx documentation for the QuectelEC200U Arduino library.

## Prerequisites

1. **Python** (3.8 or higher)
2. **pip** (Python package manager)

## Installation

### Step 1: Install Python Dependencies

```bash
# Navigate to docs directory
cd docs

# Install required packages
pip install -r requirements.txt
```

Or install packages individually:

```bash
pip install sphinx sphinx-rtd-theme breathe myst-parser
```

### Step 2: Verify Installation

```bash
sphinx-build --version
```

## Building HTML Documentation

### On Linux/macOS

```bash
# Navigate to docs directory
cd docs

# Build HTML documentation
make html

# The output will be in _build/html/
```

### On Windows

```cmd
# Navigate to docs directory
cd docs

# Build HTML documentation
make.bat html

# The output will be in _build\html\
```

### Alternative Method (All Platforms)

```bash
# Direct sphinx-build command
sphinx-build -b html . _build/html
```

## Viewing the Documentation

After building, open the documentation in your browser:

### Linux/macOS
```bash
open _build/html/index.html
```

### Windows
```cmd
start _build\html\index.html
```

### Or manually navigate to:
`docs/_build/html/index.html`

## Building Other Formats

### PDF (requires LaTeX)

```bash
# Install LaTeX first (platform-specific)
# Ubuntu/Debian:
sudo apt-get install texlive-full

# macOS:
brew install --cask mactex

# Then build PDF
make latexpdf
```

### EPUB

```bash
make epub
```

### Single HTML File

```bash
make singlehtml
```

## Live Development Server

For live reloading during documentation development:

```bash
# Install sphinx-autobuild
pip install sphinx-autobuild

# Run live server
sphinx-autobuild . _build/html

# Open http://127.0.0.1:8000 in your browser
```

## Cleaning Build Files

```bash
# Remove all built files
make clean

# On Windows
make.bat clean
```

## Documentation Structure

```
docs/
├── conf.py              # Sphinx configuration
├── index.rst            # Main documentation index
├── introduction.rst     # Introduction and overview
├── installation.rst     # Installation guide
├── getting_started.rst  # Getting started tutorial
├── api_reference.rst    # Complete API documentation
├── examples.rst         # Code examples
├── error_codes.rst      # Error code reference
├── troubleshooting.rst  # Troubleshooting guide
├── changelog.rst        # Version history
├── requirements.txt     # Python dependencies
├── Makefile            # Unix/Linux build script
├── make.bat            # Windows build script
└── _build/             # Build output directory (generated)
    └── html/           # HTML output
        └── index.html  # Main entry point

```

## Customization

### Changing Theme

Edit `conf.py`:

```python
# Change from:
html_theme = 'sphinx_rtd_theme'

# To another theme, e.g.:
html_theme = 'alabaster'
# or
html_theme = 'sphinx_book_theme'  # requires: pip install sphinx-book-theme
```

### Adding New Pages

1. Create new `.rst` file in `docs/` directory
2. Add to `toctree` in `index.rst`:

```rst
.. toctree::
   :maxdepth: 2

   introduction
   installation
   your_new_page  # Add here
```

## Troubleshooting

### "sphinx-build: command not found"

```bash
# Ensure sphinx is in PATH
export PATH=$PATH:~/.local/bin

# Or use python module directly
python -m sphinx.cmd.build -b html . _build/html
```

### "Module not found" errors

```bash
# Reinstall requirements
pip install --upgrade -r requirements.txt
```

### Build warnings

```bash
# Build with verbose output
make html SPHINXOPTS="-v"

# Treat warnings as errors (for CI)
make html SPHINXOPTS="-W"
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build Documentation

on: [push, pull_request]

jobs:
  docs:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - uses: actions/setup-python@v2
      with:
        python-version: '3.9'
    - name: Install dependencies
      run: |
        pip install -r docs/requirements.txt
    - name: Build HTML docs
      run: |
        cd docs
        make html
    - name: Upload artifacts
      uses: actions/upload-artifact@v2
      with:
        name: html-docs
        path: docs/_build/html/
```

## Publishing to GitHub Pages

```bash
# Install gh-pages
pip install ghp-import

# Build and push to gh-pages branch
cd docs
make html
ghp-import -n -p -f _build/html
```

## Publishing to Read the Docs

1. Create account at https://readthedocs.org
2. Import your GitHub repository
3. RTD will automatically build from `docs/` directory

## Additional Resources

- [Sphinx Documentation](https://www.sphinx-doc.org/)
- [reStructuredText Primer](https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html)
- [Read the Docs Theme](https://sphinx-rtd-theme.readthedocs.io/)
- [Breathe (C++ Documentation)](https://breathe.readthedocs.io/)

## License

Documentation is licensed under CC BY 4.0. See LICENSE file for details.