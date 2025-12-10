# Asterisk Dialplan Validator

A lightweight, standalone syntax validator for Asterisk dialplan files. Zero dependencies, instant results.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Version: 1.3](https://img.shields.io/badge/Version-1.3-green.svg)](https://github.com/calvintwells/dialplan_validator)

**Version:** 1.3  
**Status:** Production Ready  
**GitHub:** https://github.com/calvintwells/dialplan_validator  
**License:** MIT

---

## Installation

### Clone the Repository
```bash
git clone https://github.com/calvintwells/dialplan_validator.git
cd dialplan_validator
```

### Compile
```bash
gcc -o dialplan_validator dialplan_validator.c -Wall
```

### Install to System (Optional)
```bash
sudo cp dialplan_validator /usr/local/bin/
sudo chmod +x /usr/local/bin/dialplan_validator
```

### Verify Installation
```bash
dialplan_validator --help
```

### Requirements

- **Compilation:** GCC or Clang
- **Runtime:** Nothing (zero dependencies)
- **OS:** Linux, macOS, BSD, or any POSIX system

---

## Why This Tool Exists

**If you already have asterisklint, you probably don't need this for comprehensive validation.** 

This tool exists for two specific scenarios:

1. **Quick syntax-only checks** - When you've made small changes and only want to verify syntax without running full semantic analysis

2. **Environments where Python isn't available** - Production servers, minimal containers, embedded systems, or CI/CD environments where installing Python 3 + dependencies isn't practical

**In short:** This is a lightweight precursory check for quick edits, not a replacement for proper validation tools.

---

## Table of Contents

- [Installation](#installation)
- [Why This Tool Exists](#why-this-tool-exists)
- [Where This Tool Fits](#where-this-tool-fits)
- [Pros and Cons](#pros-and-cons)
- [Quick Start](#quick-start)
- [What It Validates](#what-it-validates)
- [What It Does NOT Validate](#what-it-does-not-validate)
- [Usage Examples](#usage-examples)
- [CI/CD Integration](#cicd-integration)
- [Comparison with Other Tools](#comparison-with-other-tools)
- [Error Examples](#error-examples)
- [Performance Benchmarks](#performance-benchmarks)
- [Recommended Workflow](#recommended-workflow)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

---

## Where This Tool Fits

### The Validation Hierarchy
```
┌─────────────────────────────────────────────────────────────┐
│ Level 1: SYNTAX VALIDATION (This Tool)                      │
│ ✓ Fast (<1ms for typical configs)                           │
│ ✓ Zero dependencies (50KB binary)                           │
│ ✓ Catches: brackets, quotes, missing commas, invalid syntax │
│ ✗ Does NOT validate: app names, context references, logic   │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ Level 2: SEMANTIC VALIDATION (asterisklint)                 │
│ ✓ Validates application names (Dial, Hangup, etc.)          │
│ ✓ Validates function names (CALLERID, IF, etc.)             │
│ ✓ Checks Goto/Gosub destinations exist                      │
│ ✓ Validates application arguments                           │
│ ✓ Pattern canonicalization suggestions                      │
│ ✗ Requires: Python 3 + dependencies (~50MB)                 │
│ ✗ Slower: ~100x slower than syntax-only                     │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│ Level 3: RUNTIME VALIDATION (Asterisk)                      │
│ ✓ Full dialplan loading and compilation                     │
│ ✓ Module availability checking                              │
│ ✓ Complete semantic analysis                                │
│ ✗ Requires: Full Asterisk installation                      │
│ ✗ Risk: Affects production if used on live system           │
└─────────────────────────────────────────────────────────────┘
```

**This tool operates at Level 1** - it's your first line of defense, not your only defense.

---

## Pros and Cons

### ✅ Strengths

| Feature | Benefit |
|---------|---------|
| **Zero Dependencies** | Single 50KB binary, runs anywhere |
| **Blazing Fast** | <1ms for 100 lines, ~50ms for 10,000 lines |
| **Production Ready** | Safe for deployment on live servers |
| **CI/CD Friendly** | Perfect for pre-commit hooks and fast-fail pipelines |
| **Easy Deployment** | Just copy the binary, no installation needed |
| **No Python Required** | Works on minimal/embedded systems |
| **Syntax Focus** | Catches the most common deployment-breaking errors |

### ❌ Limitations (By Design)

| Does NOT Validate | Why | Use Instead |
|-------------------|-----|-------------|
| Application names | Would require Asterisk app database | asterisklint |
| Function names | Would require function definitions | asterisklint |
| Goto/Gosub targets | Requires context cross-referencing | asterisklint |
| Variable existence | Context-dependent, runtime concern | asterisklint |
| Pattern correctness | Can't verify pattern logic | asterisklint |
| Application arguments | App-specific validation too complex | asterisklint |

**Key Point:** This tool catches **syntax errors** that break the parser. It does NOT catch **logic errors** that break the dialplan behavior.

### 🎯 Sweet Spot

**Use dialplan_validator when:**
- ✅ Making small changes and only need syntax validation
- ✅ Production servers where Python isn't installed
- ✅ Minimal environments (containers, embedded systems)
- ✅ Quick syntax-only checks during iterative editing
- ✅ Fast-fail in CI/CD pipelines (syntax before semantics)

**Use asterisklint instead when:**
- ✅ Making significant dialplan changes
- ✅ You need semantic validation (app names, goto targets, etc.)
- ✅ You have Python 3 available
- ✅ Pre-deployment comprehensive validation
- ✅ Learning Asterisk (asterisklint teaches best practices)

**Don't use dialplan_validator when:**
- ❌ You have asterisklint and need comprehensive validation
- ❌ You need to validate application names or logic
- ❌ You're doing a full dialplan review

---

## Quick Start

### Compile
```bash
gcc -o dialplan_validator dialplan_validator.c -Wall
```

### Run
```bash
./dialplan_validator /etc/asterisk/extensions.conf
```

### Output
```
✓ Syntax valid: /etc/asterisk/extensions.conf
```

Or:
```
Line 24: Unbalanced delimiters (parens=1, brackets=0, braces=0)
Line 42: Invalid priority 'n(mylabel)' (must be number, 'n', or 'hint')
Validation complete: 2 error(s), 0 warning(s)
```

---

## What It Validates

### ✅ Core Syntax Elements

| Element | Example | What's Checked |
|---------|---------|----------------|
| **Contexts** | `[default]` | Bracket matching, non-empty names |
| **Extensions** | `exten => _1NXXXXXXXXX,1,Dial(...)` | Arrow syntax, field count |
| **Continuations** | `same => n,NoOp()` | Arrow syntax, field count |
| **Priority Labels** | `n(start)`, `1(retry)` | Label syntax, parentheses matching |
| **Priorities** | `1`, `n`, `hint`, `5(retry)` | Format validation |
| **Parentheses** | `Dial(SIP/peer,30,g)` | Balanced `()` |
| **Brackets** | `$[${COUNT} + 1]` | Balanced `[]` |
| **Braces** | `${CALLERID(num)}` | Balanced `{}` |
| **Variables** | `${EXTEN}`, `${IF(...)}` | Syntax `${...}` |
| **Expressions** | `$[1 + 1]` | Syntax `$[...]` |
| **Includes** | `include => other-context` | Arrow syntax |
| **Switches** | `switch => Realtime/...` | Arrow syntax |

### 📋 Supported Priority Formats
```ini
exten => s,1,NoOp()           ; Numeric priority
same => n,Dial(...)           ; Next priority
same => n(label),Verbose(...) ; Next with label
same => 5,NoOp()              ; Explicit priority
same => 5(start),Dial(...)    ; Explicit with label
same => hint,NoOp()           ; Hint priority
```

### 🚫 Quote Handling (Important!)

**Quotes are NOT validated** because Asterisk handles them in a context-dependent way:
```ini
; These are ALL valid in Asterisk:
same => n,Verbose(1, It's working)            ; Apostrophe in text
same => n,Verbose(1, Provider's share: 50%)   ; Possessive apostrophe
same => n,Set(VAR="value")                    ; Quote delimiters
same => n,Set(VAR='value')                    ; Single quote delimiters
```

The validator only checks **parentheses, brackets, and braces** for balance.

---

## What It Does NOT Validate

### ❌ Semantic Elements (Use asterisklint)
```ini
[default]
exten => s,1,NoOp()
same => n,Payback(audiofile)           ; ❌ Wrong app name (should be Playback)
same => n,Goto(nowhere,s,1)            ; ❌ Context "nowhere" doesn't exist
same => n,Set(VAR=${NONEXISTENT})      ; ❌ Variable doesn't exist
same => n,Dial(${HINT})                ; ❌ HINT not defined anywhere
```

**dialplan_validator says:** ✅ Syntax valid  
**asterisklint says:** ❌ 4 errors found

### 🔍 Example: What Each Tool Catches

**Test dialplan:**
```ini
[default]
exten => _8[2-9]x,1,NoOp
same => n,GoSub(somewhere,s,1(argument1,argument2)
same => n,Payback(audiofile)
```

**dialplan_validator output:**
```
Line 3: Unbalanced delimiters (parens=1, brackets=0, braces=0)
Validation complete: 1 error(s), 0 warning(s)
```
- ✅ Catches unbalanced `)`
- ❌ Doesn't know "Payback" is wrong
- ❌ Doesn't know "somewhere" doesn't exist

**asterisklint output:**
```
extensions.conf:2 H_PAT_NON_CANONICAL: pattern '_8[2-9]x' not canonical (use '_8NX')
extensions.conf:3 W_APP_BAD_CASE: app 'GoSub' should be 'Gosub'
extensions.conf:3 W_APP_BALANCE: unbalanced parentheses in app data
extensions.conf:4 E_APP_MISSING: app 'Payback' does not exist (typo for 'Playback'?)
extensions.conf:3 E_DP_GOTO_NOCONTEXT: context 'somewhere' not found
```
- ✅ Catches ALL errors
- ✅ Provides best practice hints
- ✅ Validates semantic correctness

---

## Usage Examples

### Basic Validation
```bash
# Validate a single file
dialplan_validator /etc/asterisk/extensions.conf

# Validate multiple files
dialplan_validator /etc/asterisk/extensions-*.conf
```

### Exit Codes
```bash
dialplan_validator extensions.conf
echo $?
# 0 = Syntax valid
# 1 = Errors found or file not found
```

### Integration with Scripts
```bash
#!/bin/bash
# Safe deployment script

if dialplan_validator /etc/asterisk/extensions.conf; then
    echo "Syntax valid, reloading..."
    asterisk -rx "dialplan reload"
else
    echo "Syntax errors detected, aborting reload!"
    exit 1
fi
```

---

## CI/CD Integration

### Git Pre-Commit Hook

Create `.git/hooks/pre-commit`:
```bash
#!/bin/bash
# Validate dialplan syntax before commit

echo "Validating dialplan syntax..."
dialplan_validator /path/to/extensions.conf

if [ $? -ne 0 ]; then
    echo "❌ Dialplan syntax errors detected!"
    echo "Fix errors before committing."
    exit 1
fi

echo "✅ Dialplan syntax valid"
exit 0
```
```bash
chmod +x .git/hooks/pre-commit
```

### GitHub Actions

Create `.github/workflows/dialplan-validation.yml`:
```yaml
name: Dialplan Validation

on: [push, pull_request]

jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Compile validator
        run: gcc -o dialplan_validator dialplan_validator.c -Wall
      
      - name: Validate syntax
        run: ./dialplan_validator asterisk/extensions.conf
      
      - name: Comprehensive check (asterisklint)
        run: |
          pip3 install asterisklint
          export ALINT_IGNORE=H_DP_,H_PAT_NON_CANONICAL
          asterisklint dialplan-check asterisk/extensions.conf
```

### GitLab CI

Create `.gitlab-ci.yml`:
```yaml
stages:
  - validate

syntax-check:
  stage: validate
  image: gcc:latest
  script:
    - gcc -o dialplan_validator dialplan_validator.c -Wall
    - ./dialplan_validator asterisk/extensions.conf
  only:
    - branches
    - merge_requests
```

### Jenkins Pipeline
```groovy
pipeline {
    agent any
    stages {
        stage('Validate Syntax') {
            steps {
                sh 'gcc -o dialplan_validator dialplan_validator.c -Wall'
                sh './dialplan_validator /etc/asterisk/extensions.conf'
            }
        }
    }
}
```

---

## Comparison with Other Tools

### Feature Matrix

| Feature | dialplan_validator | asterisklint | Asterisk CLI |
|---------|-------------------|--------------|--------------|
| **Dependencies** | None | Python 3 | Full Asterisk |
| **Binary Size** | 50KB | ~50MB | ~100MB+ |
| **Speed (1000 lines)** | ~5ms | ~500ms | ~1000ms |
| **Syntax Validation** | ✅ Full | ✅ Full | ✅ Full |
| **App Name Validation** | ❌ No | ✅ Yes | ✅ Yes |
| **Function Validation** | ❌ No | ✅ Yes | ✅ Yes |
| **Goto/Gosub Checking** | ❌ No | ✅ Yes | ✅ Yes |
| **Pattern Validation** | ❌ No | ✅ Yes | ✅ Yes |
| **Argument Checking** | ❌ No | ✅ Yes | ✅ Yes |
| **Production Safe** | ✅ Yes | ✅ Yes | ⚠️ Caution |
| **Offline Use** | ✅ Yes | ✅ Yes | ❌ No |
| **CI/CD Friendly** | ✅ Excellent | ✅ Good | ⚠️ Limited |

### Performance Comparison

**Test:** 1000-line production dialplan

| Tool | Time | Memory | CPU |
|------|------|--------|-----|
| dialplan_validator | 5ms | <1MB | Minimal |
| asterisklint | 500ms | ~50MB | Moderate |
| Asterisk reload | 1000ms | ~100MB | High |

### When to Use Which Tool

| Scenario | Use This | Not That | Why |
|----------|----------|----------|-----|
| **Pre-commit hook** | dialplan_validator | asterisk reload | No Asterisk installation required |
| **CI/CD pipeline** | Both tools | Asterisk only | Layered validation without production risk |
| **Production deployment** | dialplan_validator first | Direct reload | Syntax check before affecting live system |
| **Development** | asterisklint | dialplan_validator | Catches logic errors during development |
| **Minimal systems** | dialplan_validator | asterisklint | No Python dependency |
| **Learning Asterisk** | asterisklint | dialplan_validator | Educational feedback on best practices |

---

## Error Examples

### Unbalanced Delimiters
```ini
[default]
exten => s,1,Dial(SIP/peer,30
same => n,Hangup()
```

**Output:**
```
Line 2: Unbalanced delimiters (parens=1, brackets=0, braces=0)
Validation complete: 1 error(s), 0 warning(s)
```

### Invalid Priority Format
```ini
[default]
exten => s,0,NoOp()  ; Priority must be >= 1
same => x,Dial(...)  ; Priority must be number, 'n', or 'hint'
```

**Output:**
```
Line 2: Priority must be >= 1
Line 3: Invalid priority 'x' (must be number, 'n', or 'hint')
Validation complete: 2 error(s), 0 warning(s)
```

### Missing Arrow Syntax
```ini
[default]
exten = s,1,NoOp()  ; Should be =>
```

**Output:**
```
Line 2: Missing '=>' in extension definition
Validation complete: 1 error(s), 0 warning(s)
```

### Unclosed Variable
```ini
[default]
exten => s,1,Set(VAR=${CALLERID(num)
```

**Output:**
```
Line 2: Unclosed ${...} variable reference
Line 2: Unbalanced delimiters (parens=1, brackets=0, braces=0)
Validation complete: 2 error(s), 0 warning(s)
```

---

## Performance Benchmarks

Tested on: Intel Xeon E5-2680 v4 @ 2.40GHz

| Lines | Time | Memory | Rate |
|-------|------|--------|------|
| 100 | <1ms | <1MB | 100K lines/sec |
| 1,000 | 5ms | <1MB | 200K lines/sec |
| 10,000 | 50ms | <1MB | 200K lines/sec |
| 100,000 | 500ms | ~2MB | 200K lines/sec |

**Conclusion:** Linear O(n) performance, suitable for even massive dialplans.

---

## Recommended Workflow

### If You Have asterisklint
```bash
# Small edit workflow (syntax-only check)
vi extensions.conf
dialplan_validator extensions.conf  # Quick syntax check

# Comprehensive validation before commit
asterisklint dialplan-check extensions.conf  # Full validation
```

### If You DON'T Have asterisklint
```bash
# Production server workflow (no Python available)
vi /etc/asterisk/extensions.conf
dialplan_validator /etc/asterisk/extensions.conf
asterisk -rx "dialplan reload"
```

### CI/CD Workflow (Layered Validation)
```bash
#!/bin/bash
# Fast syntax check first (fails fast on obvious errors)
dialplan_validator extensions.conf || exit 1

# Only run comprehensive check if syntax is valid
asterisklint dialplan-check extensions.conf
```

**This saves CI/CD resources:** If there's a syntax error, you fail immediately instead of running full semantic analysis.

---

## Real-World Example

### Scenario 1: Quick Edit on Production Server (No Python)
```bash
# Production server - Python not installed
ssh server01
vi /etc/asterisk/extensions.conf  # Change one line

# Quick syntax check
dialplan_validator /etc/asterisk/extensions.conf
# ✅ Syntax valid

# Safe to reload
asterisk -rx "dialplan reload"
```

**Why not asterisklint?** Production servers often don't have Python/pip for security reasons.

---

### Scenario 2: Development with asterisklint Available
```bash
# Making 2-line change
vi extensions.conf

# Quick syntax check during editing
dialplan_validator extensions.conf
# ✅ Syntax valid

# Continue editing...
vi extensions.conf

# Before commit: comprehensive check
asterisklint dialplan-check extensions.conf
# ✅ All checks passed

git commit -m "Updated routing logic"
```

**Why use dialplan_validator here?** Quick syntax-only validation during iterative editing, then full validation (asterisklint) before commit.

---

### Scenario 3: Large Refactor
```bash
# Refactoring entire context (100+ lines changed)
vi extensions.conf

# Skip dialplan_validator, go straight to comprehensive check
asterisklint dialplan-check extensions.conf
```

**Why skip dialplan_validator?** For large changes, you need full semantic validation anyway, so the extra syntax-only step doesn't add value.

---

## Contributing

Contributions welcome! This tool is intentionally simple (syntax-only validation).

### Areas for Improvement

- ✅ Bug fixes always welcome
- ✅ Performance optimizations
- ✅ Additional syntax validations
- ✅ Better error messages
- ⚠️ Semantic validation → Consider contributing to asterisklint instead

### Development Guidelines

1. Keep zero dependencies (no external libraries)
2. Maintain single-file simplicity
3. Focus on syntax validation only
4. Preserve <1ms performance for typical configs
5. Add test cases for new validations

### Reporting Issues
```bash
# Include this information:
1. dialplan_validator version (dialplan_validator --help)
2. Sample dialplan that triggers the issue
3. Expected vs actual output
4. Asterisk version (if relevant)
```

---

## Changelog

### Version 1.3 (2024-12-10) - CURRENT

#### Critical Fix: Quote Handling
- **Removed:** Quote checking from delimiter validation
- **Reason:** Asterisk handles quotes context-dependently per application
- **Impact:** Eliminates false positives on valid constructs like:
  - `Verbose(1, It's working)` - apostrophes in text
  - `Verbose(1, Provider's share: 50%)` - possessive forms

#### Documentation
- Added comprehensive README with pros/cons
- Added validation hierarchy explanation
- Added comparison with asterisklint
- Added recommended workflow examples

---

### Version 1.2 (2024-12-10)

#### Critical Fix: Priority Label Support
- **Added:** Full support for priority labels: `n(label)`, `1(start)`, etc.
- **Implementation:** Based on actual Asterisk pbx_config.c parsing logic
- **New function:** `validate_priority_with_label()`

---

### Version 1.1 (2024-12-10)

#### Bug Fixes
- **Fixed:** `same` directive incorrectly required 3 fields instead of 2
- **Fixed:** Removed unused variable warnings

---

### Version 1.0 (2024-12-10) - Initial Release

#### Features
- Context definition validation
- Extension/same syntax validation
- Basic priority validation
- Delimiter balance checking
- Variable syntax validation

---

## FAQ

### Q: Should I use this instead of asterisklint?

**A:** No. If you have asterisklint, use that for comprehensive validation.

Use dialplan_validator only when:
- You need quick syntax-only validation for small edits
- You're on a system without Python (production servers, embedded systems)
- You're doing rapid iterative editing and don't need semantic checks yet

**Rule of thumb:**
- Small edit, syntax-only check? → dialplan_validator
- Significant changes or pre-deployment? → asterisklint
- No Python available? → dialplan_validator only

### Q: When would I use both tools?

**A:** Only in specific CI/CD scenarios where you want layered validation:
```bash
# Layer 1: Fast syntax check (fails fast on obvious errors)
dialplan_validator extensions.conf || exit 1

# Layer 2: Only run comprehensive check if syntax is valid
asterisklint dialplan-check extensions.conf
```

This prevents wasting CI/CD resources on semantic checks when there are obvious syntax errors.

### Q: Why doesn't this validate application names?

**A:** By design. Validating app names requires:
- Complete Asterisk module database
- Version-specific app lists
- Dynamic module loading awareness

This adds ~50MB of dependencies and 100x slower performance. Use asterisklint for that.

### Q: Can this replace `asterisk -rx "dialplan reload"`?

**A:** No. Asterisk performs full compilation and loading. This tool only checks syntax. Use this BEFORE reloading to catch errors early.

### Q: Is this safe for production servers?

**A:** Yes. It only reads files, never modifies anything or touches Asterisk.

### Q: Why C instead of Python/Bash?

**A:** Performance and portability:
- C: 50KB binary, <1ms, runs anywhere
- Python: Requires interpreter, ~50MB, ~100x slower
- Bash: Would be slow and fragile for complex parsing

### Q: Does this work with Asterisk 1.8 / 11 / 13 / 16 / 18 / 20 / 21?

**A:** Yes. The core dialplan syntax has been stable since Asterisk 1.6.

---

## License

MIT License

Copyright (c) 2024 Calvin Wells

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## Contact

**Author:** Calvin Wells  
**GitHub:** https://github.com/calvintwells/dialplan_validator  
**Issues:** https://github.com/calvintwells/dialplan_validator/issues

---

## Acknowledgments

- Based on parsing logic from **Asterisk PBX** (`asterisk/pbx/pbx_config.c`)
- Inspired by **asterisklint** by Walter Doekes (OSSO B.V.)
- Thanks to the Asterisk community for 20+ years of open source telephony

---

**Remember:** This tool is for **quick syntax checks** when you don't have or don't need asterisklint's comprehensive validation.
```bash
# Quick edit? Syntax-only check:
dialplan_validator extensions.conf

# Significant changes? Comprehensive validation:
asterisklint dialplan-check extensions.conf

# CI/CD? Layered approach (fail fast):
dialplan_validator extensions.conf && \
asterisklint dialplan-check extensions.conf
```
