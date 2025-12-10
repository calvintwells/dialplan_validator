/* dialplan_validator.c
 * Lightweight standalone Asterisk dialplan syntax validator
 * Extracts core parsing logic from pbx_config.c without Asterisk dependencies
 * 
 * Compile: gcc -o dialplan_validator dialplan_validator.c
 * Usage: ./dialplan_validator /etc/asterisk/extensions.conf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 4096
#define MAX_VAR_NAME 80

typedef struct {
    int errors;
    int warnings;
    int line_num;
    char current_context[MAX_VAR_NAME];
} validator_state;

/* Trim leading/trailing whitespace */
static char *trim(char *str) {
    char *end;
    while (isspace(*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = 0;
    return str;
}

/* Check if line is a comment or blank */
static int is_comment_or_blank(const char *line) {
    while (isspace(*line)) line++;
    return (*line == ';' || *line == '\0' || *line == '#');
}

/* Check balanced delimiters */
static int check_balanced(const char *str, validator_state *state) {
    int parens = 0, brackets = 0, braces = 0;
    int in_quote = 0;
    char quote_char = 0;
    
    for (const char *p = str; *p; p++) {
        if (in_quote) {
            if (*p == quote_char && *(p-1) != '\\') {
                in_quote = 0;
            }
            continue;
        }
        
        switch (*p) {
            case '"':
            case '\'':
                quote_char = *p;
                in_quote = 1;
                break;
            case '(': parens++; break;
            case ')': parens--; break;
            case '[': brackets++; break;
            case ']': brackets--; break;
            case '{': braces++; break;
            case '}': braces--; break;
        }
        
        if (parens < 0 || brackets < 0 || braces < 0) {
            fprintf(stderr, "Line %d: Unbalanced delimiters (too many closing)\n", 
                    state->line_num);
            state->errors++;
            return 0;
        }
    }
    
    if (in_quote) {
        fprintf(stderr, "Line %d: Unclosed quote\n", state->line_num);
        state->errors++;
        return 0;
    }
    
    if (parens != 0 || brackets != 0 || braces != 0) {
        fprintf(stderr, "Line %d: Unbalanced delimiters (parens=%d, brackets=%d, braces=%d)\n",
                state->line_num, parens, brackets, braces);
        state->errors++;
        return 0;
    }
    
    return 1;
}

/* Validate variable syntax ${...} and $[...] */
static int check_variable_syntax(const char *str, validator_state *state) {
    const char *p = str;
    int valid = 1;
    
    while ((p = strchr(p, '$')) != NULL) {
        if (*(p+1) == '{') {
            // Found ${, look for closing }
            const char *start = p + 2;
            int brace_count = 1;
            p += 2;
            
            while (*p && brace_count > 0) {
                if (*p == '{') brace_count++;
                else if (*p == '}') brace_count--;
                p++;
            }
            
            if (brace_count != 0) {
                fprintf(stderr, "Line %d: Unclosed ${...} variable reference\n", 
                        state->line_num);
                state->errors++;
                valid = 0;
            }
        } else if (*(p+1) == '[') {
            // Found $[, look for closing ]
            const char *start = p + 2;
            int bracket_count = 1;
            p += 2;
            
            while (*p && bracket_count > 0) {
                if (*p == '[') bracket_count++;
                else if (*p == ']') bracket_count--;
                p++;
            }
            
            if (bracket_count != 0) {
                fprintf(stderr, "Line %d: Unclosed $[...] expression\n", 
                        state->line_num);
                state->errors++;
                valid = 0;
            }
        } else {
            p++;
        }
    }
    
    return valid;
}

/* Parse context line [context-name] */
static int parse_context(char *line, validator_state *state) {
    char *end = strchr(line, ']');
    if (!end) {
        fprintf(stderr, "Line %d: Malformed context (missing ']')\n", state->line_num);
        state->errors++;
        return 0;
    }
    
    *end = '\0';
    char *context = trim(line + 1);
    
    if (strlen(context) == 0) {
        fprintf(stderr, "Line %d: Empty context name\n", state->line_num);
        state->errors++;
        return 0;
    }
    
    strncpy(state->current_context, context, MAX_VAR_NAME - 1);
    return 1;
}

/* Parse extension line: exten => pattern,priority,app(args) */
static int parse_extension(char *line, validator_state *state) {
    char *arrow = strstr(line, "=>");
    if (!arrow) {
        fprintf(stderr, "Line %d: Missing '=>' in extension definition\n", state->line_num);
        state->errors++;
        return 0;
    }
    
    // Check if it's 'exten' or 'same'
    char *keyword = line;
    while (isspace(*keyword)) keyword++;
    
    int is_same = 0;
    if (strncasecmp(keyword, "exten", 5) == 0) {
        // Good
    } else if (strncasecmp(keyword, "same", 4) == 0) {
        is_same = 1;
    } else {
        fprintf(stderr, "Line %d: Unknown keyword (expected 'exten' or 'same')\n", 
                state->line_num);
        state->errors++;
        return 0;
    }
    
    // Parse pattern, priority, app
    char *data = arrow + 2;
    data = trim(data);
    
    // Split by commas (but respect balanced delimiters)
    char *pattern = data;
    char *comma1 = NULL, *comma2 = NULL;
    int parens = 0, brackets = 0;
    
    for (char *p = data; *p; p++) {
        if (*p == '(') parens++;
        else if (*p == ')') parens--;
        else if (*p == '[') brackets++;
        else if (*p == ']') brackets--;
        else if (*p == ',' && parens == 0 && brackets == 0) {
            if (!comma1) {
                comma1 = p;
            } else if (!comma2) {
                comma2 = p;
                break;
            }
        }
    }
    
    if (!comma1 || !comma2) {
        fprintf(stderr, "Line %d: Extension must have format: exten => pattern,priority,app(args)\n",
                state->line_num);
        state->errors++;
        return 0;
    }
    
    *comma1 = '\0';
    *comma2 = '\0';
    
    char *priority_str = trim(comma1 + 1);
    char *app = trim(comma2 + 1);
    
    pattern = trim(pattern);
    
    // Validate priority
    if (!is_same) {
        if (strcmp(priority_str, "hint") != 0) {
            // Check if priority is a number or 'n'
            if (strcmp(priority_str, "n") != 0) {
                char *endptr;
                long pri = strtol(priority_str, &endptr, 10);
                if (*endptr != '\0' && *endptr != '(') {
                    fprintf(stderr, "Line %d: Invalid priority '%s' (must be number, 'n', or 'hint')\n",
                            state->line_num, priority_str);
                    state->errors++;
                    return 0;
                }
                if (pri < 1) {
                    fprintf(stderr, "Line %d: Priority must be >= 1\n", state->line_num);
                    state->errors++;
                    return 0;
                }
            }
        }
    }
    
    // Check if app has parentheses for arguments
    if (strlen(app) > 0) {
        char *paren = strchr(app, '(');
        if (paren) {
            // Has arguments, check balanced
            if (!check_balanced(app, state)) {
                return 0;
            }
        }
    }
    
    // Check variable syntax in the entire line
    check_variable_syntax(data, state);
    
    return 1;
}

/* Parse include line */
static int parse_include(char *line, validator_state *state) {
    char *arrow = strstr(line, "=>");
    if (!arrow) {
        fprintf(stderr, "Line %d: Missing '=>' in include statement\n", state->line_num);
        state->errors++;
        return 0;
    }
    
    char *context = trim(arrow + 2);
    if (strlen(context) == 0) {
        fprintf(stderr, "Line %d: Empty context in include statement\n", state->line_num);
        state->errors++;
        return 0;
    }
    
    return 1;
}

/* Main validator */
static int validate_dialplan(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 1;
    }
    
    validator_state state = {0};
    char line[MAX_LINE];
    int in_context = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        state.line_num++;
        
        // Remove newline
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        // Skip comments and blank lines
        if (is_comment_or_blank(line)) {
            continue;
        }
        
        char *trimmed = trim(line);
        
        // Check for context
        if (trimmed[0] == '[') {
            parse_context(trimmed, &state);
            in_context = 1;
            continue;
        }
        
        if (!in_context) {
            // Must be in [general] or [globals] or similar
            if (strchr(trimmed, '=') && !strstr(trimmed, "=>")) {
                // Variable assignment in [general] or [globals]
                continue;
            }
        }
        
        // Check for exten/same
        if (strncasecmp(trimmed, "exten", 5) == 0 || strncasecmp(trimmed, "same", 4) == 0) {
            parse_extension(trimmed, &state);
            continue;
        }
        
        // Check for include
        if (strncasecmp(trimmed, "include", 7) == 0) {
            parse_include(trimmed, &state);
            continue;
        }
        
        // Check for switch
        if (strncasecmp(trimmed, "switch", 6) == 0 ||
            strncasecmp(trimmed, "eswitch", 7) == 0 ||
            strncasecmp(trimmed, "lswitch", 7) == 0) {
            // Switch statement, basic validation
            if (!strstr(trimmed, "=>")) {
                fprintf(stderr, "Line %d: Missing '=>' in switch statement\n", state.line_num);
                state.errors++;
            }
            continue;
        }
        
        // Unknown line type
        if (in_context && strlen(trimmed) > 0) {
            fprintf(stderr, "Line %d: Warning: Unknown directive '%s'\n", 
                    state.line_num, trimmed);
            state.warnings++;
        }
    }
    
    fclose(fp);
    
    // Summary
    printf("\n");
    if (state.errors == 0 && state.warnings == 0) {
        printf("✓ Syntax valid: %s\n", filename);
        return 0;
    } else {
        printf("Validation complete: %d error(s), %d warning(s)\n", 
               state.errors, state.warnings);
        return (state.errors > 0) ? 1 : 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <extensions.conf>\n", argv[0]);
        fprintf(stderr, "Example: %s /etc/asterisk/extensions-test.conf\n", argv[0]);
        return 1;
    }
    
    return validate_dialplan(argv[1]);
}
