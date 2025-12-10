# Asterisk Dialplan Validator

A lightweight, standalone syntax validator for Asterisk dialplan configuration files (`extensions.conf`). Validates dialplan syntax **without** requiring Asterisk to be installed or running.

Perfect for catching syntax errors **before** they break your production phone system.

## Features

✅ **Zero Dependencies** - Pure C, compiles with standard GCC  
✅ **Fast** - Validates large dialplans in milliseconds  
✅ **No Asterisk Required** - Runs independently on any Linux system  
✅ **Exit Codes** - Returns 0 for valid syntax, 1 for errors (script-friendly)  
✅ **Detailed Error Messages** - Shows line numbers and specific syntax issues  
✅ **CI/CD Ready** - Perfect for automated testing pipelines

## What It Validates

- **Context definitions**: `[context-name]`
- **Extension syntax**: `exten => pattern,priority,app(args)`
- **Same continuation lines**: `same => n,App(args)`
- **Balanced delimiters**: Parentheses `()`, brackets `[]`, braces `{}`
- **Quote matching**: Single `'` and double `"` quotes
- **Variable syntax**: `${VAR}` and `$[EXPR]` expressions
- **Priority values**: Must be `>= 1`, `n`, or `hint`
- **Include statements**: `include => context`
- **Switch statements**: `switch`, `eswitch`, `lswitch`

## Quick Start

### Download and Compile
```bash
# Clone the repository
git clone https://github.com/calvintwells/dialplan_validator.git
cd dialplan_validator

# Compile
gcc -o dialplan_validator dialplan_validator.c -Wall

# Test it
./dialplan_validator --help
```

### Install System-Wide (Optional)
```bash
sudo cp dialplan_validator /usr/local/bin/
sudo chmod +x /usr/local/bin/dialplan_validator
```

### Validate Your Dialplan
```bash
./dialplan_validator /etc/asterisk/extensions.conf
```

**Success:**
```
✓ Syntax valid: /etc/asterisk/extensions.conf
```

**Errors found:**
```
Line 42: Unbalanced delimiters (parens=1, brackets=0, braces=0)
Line 58: Unclosed ${...} variable reference
Line 103: Invalid priority 'x' (must be number, 'n', or 'hint')

Validation complete: 3 error(s), 0 warning(s)
```

## Why Use This?

### Problem: Asterisk Syntax Errors Break Production
```bash
# You edit a dialplan on your production server
vim /etc/asterisk/extensions.conf

# You reload the dialplan
asterisk -rx "dialplan reload"

# Oops! Syntax error! Calls start failing!
```

### Solution: Validate Before Deploying
```bash
# Validate FIRST (takes <10ms)
dialplan_validator /etc/asterisk/extensions.conf

# Only reload if validation passes
if [ $? -eq 0 ]; then
    asterisk -rx "dialplan reload"
fi
```

### Comparison with Other Tools

| Tool | Speed | Requires Asterisk | Dependencies | Best For |
|------|-------|-------------------|--------------|----------|
| **dialplan_validator** | ⚡ <10ms | ❌ No | None | Quick syntax checks, CI/CD |
| `asterisk -rx "dialplan reload"` | 🐢 2-5s | ✅ Yes | Full Asterisk | Final production verification |
| `asterisklint` | ⚡ Fast | ❌ No | Python + packages | Comprehensive code analysis |

## Use Cases

### 1. Pre-Deployment Validation

Check syntax before reloading Asterisk:
```bash
#!/bin/bash
# safe-reload.sh

CONF="/etc/asterisk/extensions.conf"

echo "Validating dialplan..."
dialplan_validator "$CONF"

if [ $? -ne 0 ]; then
    echo "❌ Syntax errors found. Not reloading."
    exit 1
fi

echo "✓ Syntax valid. Creating backup..."
cp "$CONF" "${CONF}.backup.$(date +%Y%m%d_%H%M%S)"

echo "Reloading dialplan..."
asterisk -rx "dialplan reload"
echo "✓ Complete"
```

### 2. Git Pre-Commit Hook

Prevent committing broken dialplans:
```bash
#!/bin/bash
# .git/hooks/pre-commit

if [ -f "extensions.conf" ]; then
    echo "Validating dialplan syntax..."
    dialplan_validator extensions.conf
    
    if [ $? -ne 0 ]; then
        echo ""
        echo "❌ Dialplan has syntax errors!"
        echo "Fix errors before committing."
        exit 1
    fi
    echo "✓ Dialplan syntax valid"
fi
```

Make it executable:
```bash
chmod +x .git/hooks/pre-commit
```

### 3. Automated Testing (CI/CD)

**What is CI/CD?** Continuous Integration / Continuous Deployment = Automatically test and deploy your code changes.

**GitHub Actions Example:**
```yaml
# .github/workflows/validate-dialplan.yml
name: Validate Asterisk Dialplan

on: [push, pull_request]

jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout code
        uses: actions/checkout@v3
      
      - name: Compile validator
        run: gcc -o dialplan_validator dialplan_validator.c -Wall
      
      - name: Validate dialplan
        run: ./dialplan_validator asterisk/extensions.conf
```

Every time you push code, GitHub automatically validates your dialplan. If validation fails, the push is blocked.

**Jenkins Example:**
```groovy
pipeline {
    agent any
    stages {
        stage('Validate Dialplan') {
            steps {
                sh './dialplan_validator asterisk/extensions.conf'
            }
        }
        stage('Deploy to Production') {
            when {
                branch 'main'
                expression { currentBuild.result == 'SUCCESS' }
            }
            steps {
                sh './deploy-to-servers.sh'
            }
        }
    }
}
```

### 4. Multi-Server Deployment

Deploy validated configs to multiple servers:
```bash
#!/bin/bash
# deploy-to-all-servers.sh

SERVERS="sbc01 sbc02 sbc03 sbc04"
CONFIG="extensions.conf"

# Validate first
echo "Validating dialplan..."
dialplan_validator "$CONFIG"
if [ $? -ne 0 ]; then
    echo "❌ Validation failed. Aborting deployment."
    exit 1
fi

# Deploy to all servers
for server in $SERVERS; do
    echo "Deploying to $server..."
    scp "$CONFIG" "$server:/etc/asterisk/"
    ssh "$server" "asterisk -rx 'dialplan reload'"
done

echo "✓ Deployed to all servers"
```

## Example Dialplan
```ini
[general]
static=yes
writeprotect=no

[globals]
TRUNK=SIP/provider

[internal]
; Internal extensions
exten => _1XX,1,NoOp(Internal call to ${EXTEN})
    same => n,Dial(PJSIP/${EXTEN},20)
    same => n,VoiceMail(${EXTEN}@default,u)
    same => n,Hangup()

; Outbound calls
exten => _91NXXNXXXXXX,1,NoOp(Outbound call)
    same => n,Set(CALLERID(num)=${COMPANY_CID})
    same => n,Dial(${TRUNK}/${EXTEN:1},60)
    same => n,Hangup()

include => voicemail-main
```

**Test it:**
```bash
./dialplan_validator example-dialplan.conf
# ✓ Syntax valid: example-dialplan.conf
```

## Common Errors Detected

### Unbalanced Parentheses
```ini
[test]
exten => s,1,Set(VAR=${IF($["${STATUS}"="OK"]?yes:no)
#                                                     ^ Missing )
```

**Output:**
```
Line 2: Unbalanced delimiters (parens=1, brackets=0, braces=0)
```

### Unclosed Variable Reference
```ini
[test]
exten => s,1,NoOp(Caller: ${CALLERID(num)
#                                        ^ Missing }
```

**Output:**
```
Line 2: Unclosed ${...} variable reference
```

### Invalid Priority
```ini
[test]
exten => s,abc,Hangup()
#          ^^^ Should be number, 'n', or 'hint'
```

**Output:**
```
Line 2: Invalid priority 'abc' (must be number, 'n', or 'hint')
```

### Missing Context Bracket
```ini
[test
exten => s,1,Hangup()
```

**Output:**
```
Line 1: Malformed context (missing ']')
```

## Exit Codes

Use in scripts to check validation status:
```bash
dialplan_validator /etc/asterisk/extensions.conf
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo "✓ Validation passed"
else
    echo "❌ Validation failed"
fi
```

| Exit Code | Meaning |
|-----------|---------|
| 0 | Syntax valid |
| 1 | Syntax errors found OR file not found |

## Performance

Validation times on modern hardware:

| Dialplan Size | Validation Time |
|--------------|-----------------|
| 100 lines | < 1ms |
| 1,000 lines | ~5ms |
| 10,000 lines | ~50ms |
| 100,000 lines | ~500ms |

**Much faster than reloading Asterisk** (which takes 2-5 seconds).

## What It Does NOT Validate

This tool performs **syntax checking only**. It does **not** check:

- ❌ Application names (e.g., whether `Dial` exists)
- ❌ Function names (e.g., whether `CALLERID` exists)
- ❌ Variable definitions (e.g., whether `${MY_VAR}` is defined)
- ❌ Context references (e.g., whether included contexts exist)
- ❌ Call flow logic (semantic correctness)
- ❌ Channel driver syntax (e.g., `PJSIP/endpoint` format)

For comprehensive validation including these checks, consider:
- **asterisklint**: `pip3 install asterisklint`
- **Asterisk reload**: Final verification in test environment

## Contributing

Contributions welcome! 

### Development Setup
```bash
# Clone the repo
git clone https://github.com/calvintwells/dialplan_validator.git
cd dialplan_validator

# Compile with debug symbols
gcc -g -o dialplan_validator dialplan_validator.c -Wall

# Check for memory leaks
valgrind ./dialplan_validator test.conf
```

### Adding New Validation Rules

1. Fork the repository
2. Add validation logic to `dialplan_validator.c`
3. Add test cases to `test/` directory
4. Submit a pull request

## Real-World Use Case

**Problem:** Managing dialplan changes across 4 Asterisk servers (sbc01-04) handling 40+ CPS.

**Before dialplan_validator:**
- Manual edits on each server
- Typos broke production calls
- 30+ minutes to deploy changes
- No easy rollback

**After dialplan_validator:**
- Edit dialplan locally in Git
- Auto-validate on commit (Git hook)
- Auto-deploy to all servers (Ansible)
- One-click rollback via Git
- Deploy time: 2 minutes
- Zero syntax-related outages since adoption

## License

MIT License - See [LICENSE](LICENSE) file for details

## Author

Created by Calvin Wells for validating Asterisk dialplan configurations in production VoIP environments.

## Related Projects

- [Asterisk PBX](https://www.asterisk.org/) - Open source communications toolkit
- [AsteriskLint](https://github.com/ossobv/asterisklint) - Comprehensive Python-based validator
- [FreePBX](https://www.freepbx.org/) - Web-based Asterisk GUI

## Support

- **Issues**: [GitHub Issues](https://github.com/calvintwells/dialplan_validator/issues)
- **Discussions**: [GitHub Discussions](https://github.com/calvintwells/dialplan_validator/discussions)

---

**⚡ Fast. Simple. Zero dependencies. Catch dialplan errors before they break production.**
